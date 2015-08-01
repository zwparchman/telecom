#include <assert.h>
#include <bitset>
#include <future>
#include <algorithm>
#include <ctype.h>
#include "entry_pool.h"
#include <exception>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include "omp.h"
#include <parallel/algorithm>
#include <stdlib.h>
#include <string>
#include <thread>
#include "Timer.h"
#include <unordered_set>
#include "uvector.h"
#include <vector>

#if ZWP_USE_BOOST
  #include <boost/iostreams/device/mapped_file.hpp>
#else
  #include "MappedFile.h"
#endif


const uint64_t max_phone_number=9'999'999'999;

using namespace std;
using ao::uvector;


template <typename T>
static inline T my_strto_int(const char *p, char const ** out) {
  T ret=0;
  char const *cur;

  for( cur=p; isspace(*cur); cur++){}

#if 1
  for( ; isdigit(*cur); cur++){
#else
  for( ; *cur>='0' && *cur<='9';  cur++){
#endif
    ret = ret * 10 + ( *cur -'0' );
  }

  *out=cur;

  return ret;
}

char const * lineBountry( char const * const beg, char const * const end){
  char const * ret = end;
  for( ; *ret != '\n' && beg < ret ; ret -- );
  assert(*ret == '\n' || ret == beg );
  return ret;
}

uint8_t read_prefrences( char const * beg, char const ** out){
  uint8_t ret=0;
  while( isdigit(*beg) ){
    uint8_t digit = *beg - '0';
    ret |= 1<<(digit-1);

    if( *(beg+1) == '#' ){
      beg += 2;
    } else {
      beg ++;
      break;
    }
  }

  *out = beg;
  return ret;
}

entry_pool fun (char const * const beg, char const * const end ){
  entry_pool ret( (end-beg)/ 20 );
  char const * cur = beg;
  while( cur < end ){
    //"Service Area Code","Phone Numbers","Preferences","Opstype","Phone Type"
    assert(*cur == '\"');
    cur+=1;
    char const * temp = beg;
    uint8_t service = my_strto_int<uint8_t>( cur, &temp);
    assert(cur != temp);
    assert(temp[0] == '\"');
    assert(temp[1] == ',');
    assert(temp[2] == '\"');
    cur = temp+3;
    uint64_t phoneNumber = my_strto_int<uint64_t>( cur, &temp);
    assert(cur != temp);
    assert(temp[0] == '\"');
    assert(temp[1] == ',');
    assert(temp[2] == '\"');
    cur = temp+3;
    uint8_t preferences = read_prefrences(cur, &temp);
    assert(cur != temp);
    assert(temp[0] == '\"');
    assert(temp[1] == ',');
    assert(temp[2] == '\"');

    cur = temp+3;
    char opstype = *cur;
    assert(cur[0] == 'A' || cur[0] == 'D');
    assert(cur[1] == '\"');
    assert(cur[2] == ',');
    assert(cur[3] == '\"');

    cur += 4;
    uint8_t phonetype = my_strto_int<uint8_t>( cur, &temp);
    assert(cur != temp);
    assert(temp[0] == '\"');
    assert(temp[1] == '\n');
    cur = temp+2;
    ret.add(phoneNumber, service, preferences, opstype, phonetype);
  }
  ret.self_sort();
  return ret;
}

#if ! ZWP_USE_BOOST
#endif 

entry_pool read( const string &fname){
  uvector<uint64_t> ret;

  Timer total_t;
  total_t.start();


  list<future<entry_pool> > futs;

#if ZWP_USE_BOOST
  boost::iostreams::mapped_file file(fname);
#else
  MappedFile file (fname);
#endif
  char const * cur = file.data();
  char const * const end = cur + file.size();

  size_t chunckSize= 100'000'000;

  //skip to past first newline
  for( ; *cur != '\n'; cur++);
  cur++;

  list<entry_pool> pools;
  future<entry_pool> merger;

  size_t maxRunning = 8;

  int pairs=0;
  while( cur < end ){
    if( *cur == '\n' ){ cur++; continue;}
    char const * temp = cur + chunckSize;
    temp = min(temp, end);

    temp=lineBountry(cur, temp);
    assert(*temp == '\n');

    if(temp == cur){
      cerr<<"error temp==cur"<<endl;
      cur++;
    } else {
      futs.push_back( async( launch::async, fun, cur, temp));
      cur=temp;
    }
    if( futs.size() >= maxRunning ){
      entry_pool a = futs.front().get();
      futs.pop_front();
      entry_pool b = futs.front().get();
      futs.pop_front();
      cout<<"grabbed pair "<<pairs++<<endl;

      entry_pool::merge(a,b);
      pools.push_back(a);
    }
  }

  while( futs.size() > 0 ){
    cout<<"futs.size() "<<futs.size()<<endl;
    entry_pool a = futs.front().get();
    futs.pop_front();
    pools.emplace_back( move(a) );
  }
  total_t.stop();
  
  while( pools.size() >= 2 ){
    cout<<"pools.size() "<<pools.size()<<endl;
    entry_pool a = pools.front();
    pools.pop_front();
    entry_pool b = pools.front();
    pools.pop_front();

    entry_pool::merge(a,b);
    pools.emplace_back( move(a) );
  }
  cout<<"pools.size() "<<pools.size()<<endl;

  cout<<endl;
  cout<< setw(10) << "time taken reading file "<< setw(10) << total_t.getTime() << endl;


  return pools.front();
}

uvector<uint64_t> getTests( size_t const testSize){
  uvector<uint64_t> ret;
  std::mt19937_64 mer;
  unsigned seed=4;
  seed = std::chrono::system_clock::now().time_since_epoch().count();
  
  mer.seed(seed);
  ret.reserve(testSize);

  std::uniform_int_distribution<uint64_t>bot_distro(0,max_phone_number);
  for( size_t i=0; i<testSize; i++){
    ret.emplace_back(bot_distro(mer));
  }
  return move(ret);
}

struct testRet{
  double time;
  int found;
};

template<typename T>
testRet doTests( T search, uvector<uint64_t> tests){
  testRet ret={0.0,0};
  Timer t;
  t.start();
  for( auto val : tests ){
    ret.found += search(val);
  }
  t.stop();
  ret.time = t.getTime();

  return ret;
}

string entry_to_string( uint64_t phoneNumber, uint8_t service_area_code, string prefrences, uint8_t opstype, uint8_t phone_type){
  string  ret;
  ret.reserve(1024);
  ret = "\""+to_string(service_area_code)+"\","+"\""+to_string( phoneNumber)+"\","+"\""+ prefrences +"\","+"\""+(opstype=='A'?"A":"D")+"\","+"\""+to_string( phone_type)+"\"";

  return ret;
}


int main(int argc, char *argv[] ){

#ifndef NDEBUG
  cout<<"Assertions are enabled"<<endl;
#endif

  string fname="dump";

  if( argc > 1 ){
    fname = argv[1];
  }


  Timer read_time;
  read_time.start();
  entry_pool pool = read(fname);
  read_time.stop();

  cout<<"read_time: "<<read_time.getTime()<<endl;

  {
    const size_t testSize = 1'000'000;
    cout<<"generating test numbers"<<endl;
    auto testset = getTests( testSize );
    cout<<"running test"<<endl;

    Timer searchTimer;
    searchTimer.start();
    int found = 0;
    for( auto i: testset){
      size_t temp = pool.getNumber_index(i) ;
      if( temp != pool.size() ){
        found++;
      }
    }
    searchTimer.stop();

    cout<<"Searching for "<<testSize<<" numbers took "<<searchTimer.getTime()<<" seconds"<<endl;
    cout<<"Found "<<found<<" numbers from a random set"<<endl;
  }

  uint64_t number;
  while( cin.good() ){
    cout<<">>>";
    cin>>number;
    
    size_t index = pool.getNumber_index(number);
    if( index == pool.size()){
      cout<<"Phone number not found"<<endl;
    } else { //found the number so display information
//"Service Area Code","Phone Numbers","Preferences","Opstype","Phone Type"
      uint8_t area_code = pool.get_service_area_code(index);
      string prefrences = entry_pool::prefrences_to_string( pool.get_prefrences(index) );
      char opstype = pool.get_ops_type(index);
      uint8_t type = pool.get_phone_type(index);

      cout << entry_to_string(number, area_code, prefrences, opstype, type) <<endl;

    }

    cout<< "index : "<<pool.getNumber_index(number)<<endl;
  }

  return 0;
}



#include <future>
#include <algorithm>
#include <boost/iostreams/device/mapped_file.hpp>
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

char * lineBountry( char * beg, char * end){
  for( ; *end != '\n' && beg > end ; end -- );
  return end;
}

uint8_t read_prefrences( char const * beg, char const ** out){
  uint8_t ret=0;
  while( isdigit(*beg) ){
    uint8_t digit = *beg - '0';
    ret |= 1<<digit;

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
    cur+=1;
    char const * temp = beg;
    uint8_t service = my_strto_int<uint8_t>( cur, &temp);
    cur = temp+3;
    uint64_t phoneNumber = my_strto_int<uint64_t>( cur, &temp);
    cur = temp+3;
    uint8_t preferences = read_prefrences(cur, &temp);
    cur = temp+3;
    uint8_t opstype = *cur == 'A';
    cur += 3;
    uint8_t phonetype = my_strto_int<uint8_t>( cur, &temp);
    cur = temp+4;
    ret.add(phoneNumber, service, preferences, opstype, phonetype);
  }
  return ret;
}


entry_pool read( const string &fname){
  uvector<uint64_t> ret;

  Timer total_t;
  total_t.start();

  boost::iostreams::mapped_file file(fname);

  list<future<entry_pool> > futs;

  auto cur = file.data();
  auto end = cur + file.size();
  size_t chunckSize= 1'000'000'000;

  //skip to past first newline
  for( ; *cur != '\n'; cur++);
  cur++;

  list<entry_pool> pools;

  size_t maxRunning = 4;

  int pairs=0;
  while( cur < end ){
    if( *cur == '\n' ){ cur++; continue;}
    auto temp = cur + chunckSize;
    temp = min(temp, end);

    temp=lineBountry(cur, temp);

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
      entry_pool a = futs.front().get();
      futs.pop_front();
      pools.push_back(a);
  }

  while( pools.size() >= 2 ){
    entry_pool a = pools.front();
    pools.pop_front();
    entry_pool b = pools.front();
    pools.pop_front();

    entry_pool::merge(a,b);
    pools.push_back(a);
  }

  total_t.stop();

  cout<<endl;
  cout<< setw(10) << "total_t "<< setw(10) << total_t.getTime() << endl;

  return pools.front();
}

uvector<uint64_t> getTests( const uvector<uint64_t> &v, double p){
  uvector<uint64_t> ret;
  size_t testSize = 1'000'000;
  std::mt19937_64 mer;
  mer.seed(4);
  ret.reserve(testSize);

  std::uniform_int_distribution<uint64_t>bot_distro(0,max_phone_number);
  std::uniform_real_distribution<double>fdistro(0,1);
  std::uniform_real_distribution<double>select_distro(0,v.size());
  if( v.size() == 0 ){
    p=-8;
  }
  for( size_t i=0; i<testSize; i++){
    const double rd = fdistro(mer);
    if( rd < p ){
      ret.emplace_back(v[select_distro(mer)]);
    } else {
      ret.emplace_back(bot_distro(mer));
    }
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

int main(){
  Timer read_time;
  read_time.start();
  entry_pool pool = read("dump");
  read_time.stop();

  //uvector<uint64_t> tests = getTests( v, 0.05);
  
  cout<<"read_time: "<<read_time.getTime()<<endl;

  uint64_t number;
  while( cin.good() ){
    cout<<">>>";
    cin>>number;

    cout<< pool.getNumber_index(number)<<endl;
  }

  return 0;
}



#include <fstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include <memory>
#include <stdint.h>
#include <string>
#include <stdlib.h>
#include <unordered_set>
#include <parallel/algorithm>
#include <future>
#include <Timer.h>
#include <list>

using namespace std;
vector<uint32_t> read( const string &fname){
  vector<uint32_t> ret;
  ret.reserve(20'000'000);

  list<future<void>> lst;
  list<vector<uint32_t>> parts;

  ifstream file(fname);

  if( !file ){
    cerr<<"bad file\n";
    exit(4);
  }

  int count=0;
  while(true){
    count++;
    if( ret.size() % 20'000'000 == 0 ){
      cerr<<"count "<<count<<endl;
      parts.emplace_back( move(ret));
      auto vv = --(parts.end());
      auto f = [=](){ __gnu_parallel::sort(vv->begin(), vv->end() );};
      lst.emplace_back( async( launch::async, f));

      ret=vector<uint32_t>();
      ret.reserve(20'000'000);
    }
    uint32_t val;
    file>>val;
    if( ! file.good() ){
      break;
    } 
    ret.emplace_back(val);
  }

  count=0;
  for( auto &v : lst ){
    cout<<"waiting on "<<count<<endl;
    count++;
    v.wait();
  }

  count=0;
  while( parts.size() > 1 ){
    cout<<"merge "<<count<<endl;
    count++;
    auto v1 = parts.front();
    parts.pop_front();
    auto v2 = parts.front();
    parts.pop_front();

    vector<uint32_t> m;
    m.resize( v1.size() + v2.size() );

    __gnu_parallel::merge( v1.begin(), v1.end(), v2.begin(), v2.end(), m.begin());
    parts.push_back( move(m));
  }

  return move( parts.front() );
}

struct BinarySearch{
  const vector<uint32_t> &vec;
  vector<uint32_t>::const_iterator beg,end;
  BinarySearch( const vector<uint32_t> &v): vec(v)
  {
    beg = vec.begin();
    end = vec.begin();
  }

  bool operator()(uint32_t val){
    auto ret = std::binary_search( vec.begin(), vec.end(), val);
    return ret;
  }
};

struct BitVec{
  vector<bool> vec;
  BitVec( const vector<uint32_t> &v){
    uint32_t high = 3'000'000'000;
    vec.resize(high);
    for( auto val : v){
      vec[val]=1;
    }
  }

  bool operator()( uint32_t val ){
    return vec[val];
  }
};

struct Unordered{
  std::unordered_set<uint32_t> set;
  Unordered( const vector<uint32_t> &v ){
    set.reserve( v.size()*1.3 );
    for( auto val: v){
      set.insert(val);
    }
  }

  bool operator()(uint32_t val){
    return set.find(val) != set.end();
  }
};


vector<uint32_t> getTests( const vector<uint32_t> &v, double p){
  vector<uint32_t> ret;
  size_t testSize = 1'000'000;
  std::mt19937_64 mer;
  ret.reserve(testSize);

  std::uniform_int_distribution<uint32_t>bot_distro(0,2'999'999'999);
  std::uniform_real_distribution<double>fdistro(0,1);
  std::uniform_real_distribution<double>select_distro(0,v.size());
  for( size_t i=0; i<testSize; i++){
    const double rd = fdistro(mer);
    if( rd < p ){
      ret.emplace_back(v[select_distro(mer)]);
    } else {
      ret.emplace_back(bot_distro(mer));
    }
  }
  return ret;
}

struct testRet{
  double time;
  int found;
};

template<typename T>
testRet doTests( T search, vector<uint32_t> tests){
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
  vector<uint32_t> v = read("dump");
  read_time.stop();
  cout<<"read_time: "<<read_time.getTime()<<endl;


  vector<uint32_t> tests = getTests( v, 0.05);
  testRet t1,t2;
  {
    cerr<<"building binary"<<endl;
    BinarySearch bsearch( v );
    cerr<<"testing bsearch"<<endl;
    t1 = doTests(bsearch, tests);
  }
  {
    cerr<<"building bitvec"<<endl;
    BitVec bitvec( v );
    cerr<<"testing bitvec"<<endl;
    t2 = doTests(bitvec, tests);
  }
  /* uses too much memory to be reasonable
  {
    cerr<<"building set"<<endl;
    Unordered unordered(v);
    cerr<<"testing set"<<endl;
    t3=doTests(unordered, tests);
  }  */

  cout<<"read_time: "<<read_time.getTime()<<endl;
  cout<<"bsearch found: "<<t1.found<<" time "<<t1.time<<endl;
  cout<<"bitvec found: "<<t2.found<<" time "<<t2.time<<endl;
  //cout<<"set found: "<<t3.found<<" time "<<t3.time<<endl;

  return 0;
}


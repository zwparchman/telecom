#include <fstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include <string>
#include <unordered_set>
#include "Timer.h"
#include <list>
#include "uvector.h"
#include <mutex>
#include <parallel/algorithm>
#include <omp.h>

const uint64_t max_phone_number=9'999'999'999;

using namespace std;
using ao::uvector;

struct Merger{
  list<uvector<uint64_t>>vecs;
  list<uvector<uint64_t>>sorted;
  volatile bool finished;
  mutex vecs_mutex;
  mutex sorted_mutex;

  thread *worker;

  static int trymerge(Merger *self){
    uvector<uint64_t> a;
    uvector<uint64_t> b;
    int ret=0;

    self->sorted_mutex.lock();
    if( self->sorted.size() >= 2 ){
      ret=1;
      a = move(self->sorted.front());
      self->sorted.pop_front();
      b = move(self->sorted.front());
      self->sorted.pop_front();
      self->sorted_mutex.unlock();

      uvector<uint64_t> m;
      m.resize( a.size() + b.size() );
      __gnu_parallel::merge(a.begin(),a.end(), b.begin(),b.end(), m.begin());

      self->sorted_mutex.lock();
      self->sorted.emplace_back(move(m));
      self->sorted_mutex.unlock();
    } else {
      self->sorted_mutex.unlock();
    }
    return ret;
  }
  static int trysort(Merger *self){
    uvector<uint64_t> mine;
    int ret=0;

    self->vecs_mutex.lock();
    if( self->vecs.size() >= 1 ){
      ret=1;
      mine = move(self->vecs.front());
      self->vecs.pop_front();
      self->vecs_mutex.unlock();

      //sort then add to merge list
      __gnu_parallel::sort(mine.begin(), mine.end());
      self->sorted_mutex.lock();
      self->sorted.emplace_front(move(mine));
      self->sorted_mutex.unlock();
    } else {
      self->vecs_mutex.unlock();
    }
    return ret;
  }

  static void work(Merger *self){
    omp_set_num_threads(4);
    int progress=0;
    while( !self->finished){
      progress += trysort(self);
      progress += trymerge(self);
      if(self->finished && progress == 0){
        self->vecs_mutex.lock();
        self->sorted_mutex.lock();
        bool con1=false;
        bool con2=false;
        con1 = 0== self->vecs.size();
        con2 = 1!= self->sorted.size();
        self->sorted_mutex.unlock();
        self->vecs_mutex.unlock();
        if( con1 || con2 ){
          continue;
        }
        return;
      }
    }
  }

  Merger(){
    finished=false;
    worker = new thread(work,this);
  }

  void add( uvector<uint64_t> &&v){
    uvector<uint64_t> myv = v;
    if( myv.size() == 0 ){
      return;
    }
    vecs_mutex.lock();
    vecs.emplace_back(v);
    vecs_mutex.unlock();
  }
  void end(){
    finished=true;
    worker->join();
    delete worker;
  }

  uvector<uint64_t> get(){
    if( !finished){
      finished=true;
      worker->join();
      delete worker;
    }
    if( sorted.size() == 0 ){
      uvector<uint64_t>ret;
      ret.resize(0);
      return move(ret);
    }
    return move(sorted.front());
  }
};

uvector<uint64_t> read( const string &fname){
  uvector<uint64_t> ret;
  const int block_size=2'000'000;
  ret.reserve(block_size);

  ifstream file(fname);

  if( !file ){
    cerr<<"bad file\n";
    exit(4);
  }

  Merger mer;
  int count=0;
  while(true){
    if( ret.size() % block_size == 0 ){
      cerr<<"count "<<count++<<endl;
      mer.add(move(ret));

      ret=uvector<uint64_t>();
      ret.reserve(block_size);
    }
    uint64_t val;
    file>>val;
    if( ! file.good() ){
      break;
    } 
    ret.emplace_back(val);
  }

  mer.end();
  return mer.get();
}

struct BinarySearch{
  const uvector<uint64_t> &vec;
  uvector<uint64_t>::const_iterator beg,end;
  BinarySearch( const uvector<uint64_t> &v): vec(v)
  {
    beg = vec.begin();
    end = vec.begin();
  }

  bool operator()(uint64_t val){
    auto ret = std::binary_search( vec.begin(), vec.end(), val);
    return ret;
  }
};

struct BitVec{
  vector<bool> vec;
  BitVec( const uvector<uint64_t> &v){
    uint64_t high = max_phone_number;
    vec.resize(high);

    omp_set_num_threads(8);
#pragma omp parallel for 
    for( size_t i=0; i<v.size(); i++){
      vec[v[i]]=1;
    }
  }

  bool operator()( uint64_t val ){
    return vec[val];
  }
};

struct Unordered{
  std::unordered_set<uint64_t> set;
  Unordered( const uvector<uint64_t> &v ){
    set.reserve( v.size()*1.3 );
    for( auto val : v){
      set.insert(val);
    }
  }

  bool operator()(uint64_t val){
    return set.find(val) != set.end();
  }
};


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
  uvector<uint64_t> v = read("dump");
  read_time.stop();
  cout<<"read_time: "<<read_time.getTime()<<endl;


  uvector<uint64_t> tests = getTests( v, 0.05);
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



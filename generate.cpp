//generate the 2.4 gb of numbers
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <stdint.h>
#include <chrono>
#include <omp.h>
#include <array>

using namespace std;

// number example 9818734126


//output will be in the range of 0 billion to (3 billion-1)
//corespoiding to starting with a 7,8,or 9 respectivly
template<typename T>
uint32_t generateNumber( T &r ){
  std::uniform_int_distribution<uint32_t>bot_distro(0,2999999999);
  const uint32_t ret = bot_distro(r);
  return ret;
}
int main(){


  // obtain a seed from the system clock:
  //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::array<std::mt19937_64,8> mer;

  for( int i=0; i<8;i++){
    mer[i].seed(i);
  }

  vector<uint32_t> v;

  size_t size = 600'000'000;
  v.resize(size);

#pragma openmp parallel for numthreads(8)
  for( size_t i=0; i<size; i++){
    int n = omp_get_thread_num();
    v[i] = generateNumber(mer[n]);
  }

  ofstream file("dump");

  int count = 0;
  for( auto i: v){
    if( count % 1'000'000 == 0 ){
      cerr<<"count: "<<count<<endl;
    }
    count++;
    file<<i<<"\n";
  }
}
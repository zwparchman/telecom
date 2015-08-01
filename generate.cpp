#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <stdint.h>
#include <chrono>
#include <omp.h>
#include <array>

#include <future>
#include <assert.h>

using namespace std;


const uint64_t max_phone_number=9'999'999'999;

struct entry{
  uint64_t phoneNumber;
  uint8_t service_area_code;
  uint8_t prefrences;
  uint8_t phone_type;
  uint8_t opstype;

  entry( std::mt19937_64 &mt){
    std::uniform_int_distribution<uint64_t>phone_distro(0,max_phone_number);
    std::uniform_int_distribution<uint8_t> service_distro(1,23);
    std::uniform_int_distribution<uint8_t> prefrences_distro(0,127);
    std::uniform_int_distribution<uint8_t> phone_type_distro(1,3);
    std::uniform_int_distribution<uint8_t> opstype_distro(0,1);

    phoneNumber = phone_distro(mt);
    service_area_code = service_distro(mt);
    prefrences = prefrences_distro(mt);
    phone_type = phone_type_distro(mt);
    opstype = opstype_distro(mt);
  }

  string prefrences_string()const{
    if( prefrences==0 ){
      return "0";
    }

    string ret="";
    for( int i=0; i<7; i++){
      if( ( prefrences >> i ) & 1 ){
        ret+=to_string(1+i)+"#";
      }
    }
    ret.erase( ret.end()-1, ret.end());

    return ret;
  }
};

string to_string( const entry & e){
  string  ret;
  ret.reserve(1024);
  ret = "\""+to_string(e.service_area_code)+"\","+"\""+to_string( e.phoneNumber)+"\","+"\""+ e.prefrences_string() +"\","+"\""+(e.opstype?"A":"D")+"\","+"\""+to_string( e.phone_type)+"\"";

  return ret;
}

vector<string> fun(mt19937_64 mt, size_t todo ){
  vector<string> v;
  for( size_t ii=0; ii<todo; ii++){
    string s;
    s.reserve(1024);
    s= to_string( entry(mt) ) + "\n";
    v.emplace_back( move(s) );
  }
  return v;
}

int main(int argc, char* argv[] ){
  size_t size=600'000'000;
  if(argc == 2 ){
    size=atoi(argv[1]);
  }

  // obtain a seed from the system clock:
  //unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  
  const int threads=100;
  assert( size % threads == 0 );

  ofstream file("dump");

  int count=0;
  list< future<vector<string>>> futs;
  for( int i=0; i<8; i++ ){
    mt19937_64 mer;
    mer.seed(count++);
    futs.push_back( async( launch::async, fun, mer, size/threads));
  }

  int ccount =0;

  file<<"\"Service Area Code\",\"Phone Numbers\",\"Preferences\",\"Opstype\",\"Phone Type\""<<endl;
  while( futs.size() > 0 ){
    auto v = futs.front().get();
    futs.pop_front();
    cerr<<"got future "<<ccount++<<endl;

    if( count < threads ){
      mt19937_64 mer;
      mer.seed(count++);
      futs.push_back( async( launch::async, fun, mer, size/threads));
    }
    for( auto &s : v ){
      file<<s;
    }
  }


  return 0;
}

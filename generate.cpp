#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <stdint.h>
#include <chrono>
#include <omp.h>
#include <array>
#include <algorithm>
#include <future>
#include <assert.h>
#include "Channel.hpp"

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

void fun(mt19937_64 mt, Channel<size_t> *in, Channel<vector<string>> *out){
  size_t todo ;
  while(true){
    if( ! in->get( todo ) ){
      return;
    }
    vector<string> v;
    v.reserve(todo);
    for( size_t ii=0; ii<todo; ii++){
      string s;
      s.reserve(1024);
      s= to_string( entry(mt) ) + "\n";
      v.emplace_back( move(s) );
    }

    out->emplace(move(v));
  }
}

int main(int argc, char* argv[] ){
  size_t size=600'000'000;
  if(argc == 2 ){
    size=atoi(argv[1]);
  }

  ofstream file("dump");

  const int number_of_workers=8;
  array<thread,number_of_workers> workers;
  Channel<uint64_t> in;
  Channel<vector<string>> out;

  for( int i=0; i<number_of_workers; i++){
    mt19937_64 mer;
    mer.seed(i);
    workers[i] = thread( fun, mer, &in, &out);
    workers[i].detach();
  }

  file<<"\"Service Area Code\",\"Phone Numbers\",\"Preferences\",\"Opstype\",\"Phone Type\""<<endl;

  const int step=500000;
  int cout_in=0;
  while(size > 0){
    int toget = step;
    if( step > size ) toget = size;
    in.put(toget);
    {
      size_t oldsize = size;
      size -= step;
      if( oldsize < size ) size=0; //prevent underflow
    }
    cout_in++;
  }
  in.close();

  for( int i=0; i<cout_in; i++){
    cout<<"count_in: "<<cout_in--<<endl;
    vector<string> v;
    out.get(v);

    for( const auto &s : v){
      file<<s;
    }
  }
  return 0;
}

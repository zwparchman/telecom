#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <list>
#include <future>
#include "uvector.h"
#include <boost/iostreams/device/mapped_file.hpp>
#include "Timer.h"
#include <ctype.h>
#include <exception>
using namespace std;
using ao::uvector;


uint64_t fun (char const * const beg, char const * const end ){
  uint64_t sum = 0;
  char const * cur = beg;
  while( cur < end ){
    char * next;
    uint64_t val = strtol(cur, &next, 10);
    sum += val;
    if( cur == end -2){ break;}
    if( cur == next ){
      cerr<<(void*)cur<<" "<<(void*)end<<" err in strtol"<<cur<<endl;
      cur++;
    } else {
      cur = next;
    }
  }
  return sum;
}

int main(){
  Timer total_t;
  total_t.start();

  boost::iostreams::mapped_file file("dump");

  list<future<uint64_t> > lst;

  auto cur = file.data();
  auto end = cur + file.size();
  size_t chunckSize= ( end - cur ) / 8;

  while( cur < end ){
    auto chunckEnd = cur + chunckSize;
    if( chunckEnd > end ){
      chunckEnd = end;
    }
    for( ; *chunckEnd != '\n'; chunckEnd -- ){};
    bool good=false;
    for( size_t i=0; i < (size_t)(end - chunckEnd); i++){
      if(! isspace(chunckEnd[i])){
        good=true;
        break;
      }
    }
    if( good ){
      try{
          lst.emplace_back( async( launch::async , fun, cur, chunckEnd));
      } catch( std::exception &e ){
        cerr<<e.what()<<" -- at byte "<<cur - file.data()<<endl;
        string line;
      }
    } else { 
      break;
    }
    cur = chunckEnd;
  }

  uint64_t sum=0;
  for( auto &f: lst){
    sum += f.get();
  }
  total_t.stop();

  cout<<endl;
  cout<< setw(10) << "total_t "<< setw(10) << total_t.getMicroTime() << endl;
  cout<<sum<<endl;
  return 0;
}

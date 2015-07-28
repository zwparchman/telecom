#include <fstream>
#include <iostream>
#include "Timer.h"
#include <parallel/algorithm>
#include <algorithm>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <list>
#include "uvector.h"
#include <iterator>
#include <stdio.h>
#include <stdint.h>
#include <array>
#include <boost/lockfree/queue.hpp>



using namespace std;
using ao::uvector;

boost::lockfree::queue<string*, boost::lockfree::fixed_sized<true> > stringqueue;


void readStrings(string fname){
  ifstream file(fname);

  while(true){
    string *line = new string;
    getline(file,*line);
    if( file.eof()){
      return;
    } else if( !file.good()){
      cerr<<"file read failed"<<endl;
      return;
    }
    stringqueue.push(line);
  }
}

int main(){
  thread writer(readStrings,"dump");

  return 0;
}

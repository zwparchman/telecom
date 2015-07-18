#include "Timer.h"


using namespace std::chrono ;

void Timer::start(){
  running  = true;
  start_time = steady_clock::now() ;
}

void Timer::stop(){
  end_time = steady_clock::now() ;
}

void Timer::reset(){
  start_time = steady_clock::now() ;
  end_time = start_time;
  running = false;
}

long int Timer::getMicroTime(){
  //duration<std::chrono::microseconds> time_span ;
  long int time_span;
  if( running ){
    time_span = duration_cast<std::chrono::microseconds>(end_time-start_time).count();
  } else {
    time_span = duration_cast<std::chrono::microseconds>(end_time-start_time).count();
  }
  return time_span;
}

double Timer::getTime(){
  duration<double> time_span ;
  if( running ){
    time_span = duration_cast<duration<double>>( steady_clock::now() - start_time );
  } else {
    time_span = duration_cast<duration<double>>(end_time - start_time);
  }

  return time_span.count() ;
}


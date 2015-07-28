
#ifndef  Timer_INC
#define  Timer_INC

#include <chrono>

struct Timer{
  bool running;

  std::chrono::high_resolution_clock::time_point start_time;
  std::chrono::high_resolution_clock::time_point end_time;

  void start();
  void stop();
  void reset();

  double getTime();
  long int getMicroTime();
};


#endif   /* ----- #ifndef Timer_INC  ----- */

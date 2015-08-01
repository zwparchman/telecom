#ifndef  Channel_INC
#define  Channel_INC

#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

template<class T>
struct Channel {


  std::list<T> lst;
  mutable std::mutex m;
  std::condition_variable cv;
  bool closed=false;

  //deleted due to condition_variable having it deleted
  Channel operator=(Channel &) = delete;

  void put(const T &i) {
    std::unique_lock<std::mutex> lock(m);
    if(closed){
      throw std::logic_error("Channel closed. Can not put");
    }

    lst.push_back(i);
    cv.notify_one();
  }

  void emplace( const T &&i){
    std::unique_lock<std::mutex> lock(m);
    if(closed){
      throw std::logic_error("Channel closed. Can not emplace");
    }
    lst.emplace_back(i);
    cv.notify_one();
  }

  
  bool get(T &out, bool wait = true) {
    std::unique_lock<std::mutex> lock(m);

    if(wait){
      cv.wait(lock, [&](){ return closed || !lst.empty(); });
    }
    if(lst.empty()) return false; 

    out = std::move(lst.front());

    lst.pop_front();
    return true;
  }

  size_t size() const{
    std::unique_lock<std::mutex> lock(m);
    return lst.size();
  }

  void close() {
    std::unique_lock<std::mutex> lock(m);
    closed = true;
    cv.notify_all();
  }

  bool is_closed() const {
    std::unique_lock<std::mutex> lock(m);
    return closed;
  }

};
#endif   /* ----- #ifndef Channel_INC  ----- */

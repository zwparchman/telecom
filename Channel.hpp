
#ifndef  Channel_INC
#define  Channel_INC


#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>



template<class item>
class channel {
private:
  std::list<item> queue;
  std::mutex m;
  std::condition_variable cv;
  bool closed;
public:
  channel() : closed(false) { }

  channel(channel &) = delete;
  channel operator=(channel &) = delete;



  void close() {
    std::unique_lock<std::mutex> lock(m);
    closed = true;
    cv.notify_all();
  }
  bool is_closed() {
    std::unique_lock<std::mutex> lock(m);
    return closed;
  }
  void put(const item &i) {
    std::unique_lock<std::mutex> lock(m);
    if(closed)
      throw std::logic_error("put to closed channel");
    queue.push_back(i);
    cv.notify_one();
  }

  void emplace( const item &&i){
    std::unique_lock<std::mutex> lock(m);
    if(closed){
      throw std::logic_error("emplace to a closed channel");
    }
    queue.emplace_back(i);
    cv.notify_one();
  }

  size_t size(){
    std::unique_lock<std::mutex> lock(m);
    return queue.size();
  }

  bool get(item &out, bool wait = true) {
    std::unique_lock<std::mutex> lock(m);
    if(wait)
      cv.wait(lock, [&](){ return closed || !queue.empty(); });
    if(queue.empty())
      return false;
    out = queue.front();
    queue.pop_front();
    return true;
  }
};



#endif   /* ----- #ifndef Channel_INC  ----- */

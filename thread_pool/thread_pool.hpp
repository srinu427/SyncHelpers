#include <thread>
#include <future>
#include <vector>
#include <functional>

#include "circular_buffer.hpp"

namespace s427{
  class ThreadPool{
  public:
    ThreadPool(uint32_t thread_count);
    ~ThreadPool();
    template< class Function, class... Args >
    auto addAsyncWork(Function&& f, Args&&... args) const -> std::future<decltype(f(args...))>;
  private:
    uint32_t _thread_count;
    std::condition_variable _spin_signal_cv;
    std::mutex _sscv_mut;
    std::vector<std::thread> _worker_threads;
    std::vector<std::atomic<bool>> _worker_stop_flags;
    CircularBuffer<std::function<void()>> _work_queue;
    
    void worker_run(uint32_t pool_tid);
  };
};
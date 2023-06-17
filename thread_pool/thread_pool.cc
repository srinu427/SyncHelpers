#include "thread_pool.hpp"
#include <chrono>
#include <memory>

namespace s427{
  ThreadPool::ThreadPool(uint32_t thread_count){
    _thread_count = thread_count;
    _worker_threads.reserve(_thread_count);
    _worker_stop_flags.reserve(_thread_count);
    for (uint32_t i=0; i < thread_count; i++){
      _worker_threads.push_back(std::thread(ThreadPool::worker_run, this, i));
      _worker_stop_flags.push_back(false);
    }
    
  }

  ThreadPool::~ThreadPool(){
    for (size_t i=0; i < _thread_count; i++){
      _worker_stop_flags[i].store(true);
    }
    for (auto& th: _worker_threads) th.join();
  }

  template< class Function, class... Args >
  auto ThreadPool::addAsyncWork(Function&& f, Args&&... args) const -> std::future<decltype(f(args...))>{
    // https://www.sobyte.net/post/2022-05/design-a-thread-pool/
    using return_t = decltype(f(args...));
    using future_t = std::future<return_t>;
    using task_t = std::packaged_task<return_t()>;

    auto bind_func = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
    std::shared_ptr<task_t> task = std::make_shared<task_t>(std::move(bind_func));
    future_t fut = task->get_future();
    _work_queue.emplace([task]() -> void { (*task)(); });
    _spin_signal_cv.notify_one();
    return fut;
  }

  void ThreadPool::worker_run(uint32_t pool_tid){
    std::unique_lock<std::mutex> lk(_sscv_mut);
    uint8_t empty_work_cycles = 0;
    while(!_worker_stop_flags[pool_tid].load()){
      if (empty_work_cycles & 4){
        empty_work_cycles = 0;
        _spin_signal_cv.wait_for(lk, std::chrono::milliseconds(1));
      }
      auto work = _work_queue.popElems();
      if (work.size() == 0) empty_work_cycles++;
      else{
        work[0]();
      }
    }
  }
}

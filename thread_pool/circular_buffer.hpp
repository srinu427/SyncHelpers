#include <vector>
#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace s427{
  template <class T>
  class CircularBuffer{
  public:
    CircularBuffer<T>(size_t max_buffer_size = 1000);
    ~CircularBuffer();
    std::vector<T> popElems(size_t count=1, bool from_back=false);
    bool pushElem(T& data, bool in_front=false);
    size_t size();
  private:
    T* _buffer;
    size_t _buffer_size;
    size_t _start = 0;
    size_t _end = 0;
    mutable std::shared_mutex _access_mut;
  };
};
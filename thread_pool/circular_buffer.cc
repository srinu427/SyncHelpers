#include "circular_buffer.hpp"

namespace s427{
  template <class T>
  CircularBuffer<T>::CircularBuffer(size_t buffer_size){
    _buffer = new T[buffer_size];
    _buffer_size = buffer_size;
  }

  template <class T>
  CircularBuffer<T>::~CircularBuffer(){
    delete[] _buffer;
  }

  template <class T>
  size_t CircularBuffer<T>::size(){
    std::shared_lock<std::shared_mutex> lck(_access_mut);
    return ((_end - _start) + _buffer_size) % _buffer_size;
  }

  template <class T>
  std::vector<T> CircularBuffer<T>::popElems(size_t count, bool from_back){
    std::vector<T> outdata;
    if (size() == 0) return outdata; 
    std::unique_lock<std::shared_mutex> lck(_access_mut);
    size_t size = ((_end - _start) + _buffer_size) % _buffer_size;
    size_t r_elem_count = std::min(count, size);
    outdata.reserve(r_elem_count);
    if (from_back){
      for (size_t i=0; i<size; i++){
        outdata[i] = _buffer[(_end - i - 1 + _buffer_size) % _buffer_size];
      }
      _end = (_end - r_elem_count + _buffer_size) % _buffer_size;
    }
    else{
      for (size_t i=0; i<size; i++){
        outdata[i] = _buffer[(_start + i) % _buffer_size];
      }
      _start = (_start + r_elem_count) % _buffer_size;
    }
    return outdata;
  }

  template <class T>
  bool CircularBuffer<T>::pushElem(T& data, bool in_front){
    std::unique_lock<std::shared_mutex> lck(_access_mut);
    size_t size = ((_end - _start) + _buffer_size) % _buffer_size;
    if (size == _buffer_size - 1){
      return false;
    }
    else{
      if (in_front){
        _buffer[(_start - 1 + _buffer_size) % _buffer_size] = data;
        _start = (_start + 1) & _buffer_size;
      }
      else{
        _buffer[_end] = data;
        _end = (_end + 1) & _buffer_size;
      }
      return true;
    }
  }
}
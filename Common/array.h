#pragma once

#if defined(CUDA)
#include "cuda_runtime.h"
#endif

#include "managed_allocation.h"
#include <memory>
#include "device.h"
#include <cstddef>
#include <stdexcept>

/**
   Simple array/vector like class to handle pool of managed memory
 **/
namespace sim {

template <typename T>
class Array: public ManagedAllocation {
  // @todo constexpr in c++14
  /*constexpr*/ T*  alloc_data() {
    #if defined(CUDA)
    T* data;
    auto err = cudaMallocManaged(&data, sizeof(T)*capacity_);
    if(err != cudaSuccess){
      throw std::runtime_error("error allocating managed memory");
    }
    return data;
    #else
    return new T[capacity_];
    #endif
  }

  /*constexpr C++14*/ void free_data() {
    #if defined(CUDA)
    cudaFree((void*)data_);
    #else
    delete[] data_;
    #endif
  }

public:
  Array(std::size_t capacity) : capacity_{capacity}, size_{0}, data_{alloc_data()}
  {}

  ~Array() {
    free_data();
  }

  /**
     Copying/moving the array is bad as it can free the data prematurely
     Copy the raw pointer instead using data() for tmp variables
   **/
  Array(const Array&)            =delete;
  Array& operator=(const Array&) =delete;
  Array(Array&&) noexcept        =delete;
  Array& operator=(Array&&)      =delete;

  std::size_t size() const {
    return size_;
  }

  std::size_t capacity() const {
    return capacity_;
  }

  std::size_t available() const {
    return capacity() - size();
  }

  void push_back(const T& value) {
    data_[size_++] = value;
  }

  void push_back(const T& value, const size_t push_count) {
    for(std::size_t i=0; i<push_count; i++)
      this->push_back(value);
  }

  void push_back(const T* values_ptr, const size_t push_count) {
    for(std::size_t i=0; i<push_count; i++)
      this->push_back(values_ptr[i]);
  }

  void pop_back() {
    size_--;
  }

  void pop_back(std::size_t pop_count) {
    size_ -= pop_count;
  }

  DEVICE_CALLABLE
  T* data() {
    return this->data_;
  }

  DEVICE_CALLABLE
  T* data() const {
    return this->data_;
  }

  DEVICE_CALLABLE
  T& operator[] (const std::size_t index) {
    return data_[index];
  }

  DEVICE_CALLABLE
  const T& operator[] (const std::size_t index) const {
    return data_[index];
  }

//private:
// DEVICE_CALLABLE cant use private member variables
public:
  std::size_t capacity_;
  std::size_t size_;
  T* data_;
};
}

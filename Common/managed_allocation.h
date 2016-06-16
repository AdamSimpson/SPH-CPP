#pragma once

#if defined(CUDA)
#include "cuda_runtime.h"
#endif

#include <iostream>
#include <new>

/**
  Class to overload new/delete operators to use cuda managed memory if CUDA is defined
**/
class ManagedAllocation {
public:
  ManagedAllocation()                                    = default; // Required for inheritance
  ~ManagedAllocation()                                   = default; // Required for inheritance
  ManagedAllocation(const ManagedAllocation&)            = delete;
  ManagedAllocation& operator=(const ManagedAllocation&) = delete;
  ManagedAllocation(ManagedAllocation&&) noexcept        = delete;
  ManagedAllocation& operator=(ManagedAllocation&&)      = delete;

  static void* operator new(std::size_t size) {
    #if defined(CUDA)
      void* data;
      auto err = cudaMallocManaged(&data, size);
      if(err != cudaSuccess) {
        throw std::runtime_error("error allocating managed memory");
      }
      return data;
    #else
      return ::operator new(size);
    #endif
  }

  static void* operator new[](std::size_t size) {
    return ManagedAllocation::operator new(size);
  }

  static void operator delete(void *block) {
    #if defined(CUDA)
      cudaFree((void*)block);
    #else
      ::operator delete(block);
    #endif
  }

  static void operator delete[](void *block) {
    ManagedAllocation::operator delete(block);
  }

};

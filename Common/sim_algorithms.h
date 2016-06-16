#pragma once

#include "device.h"
#include "vec.h"
#include "thrust/iterator/counting_iterator.h"
#include "thrust/execution_policy.h"
#include "thrust/for_each.h"

// Most calls to for_each will require a device synchronization
#ifdef CUDA
namespace sim {
  template<typename T>
  void for_each_index(IndexSpan span, T body) {
    thrust::counting_iterator<std::size_t> begin(span.begin);
    thrust::counting_iterator<std::size_t> end(span.end);

    thrust::for_each(thrust::system::cuda::par, begin, end, body);
    cudaDeviceSynchronize();
  }
} // end namespace sim
#endif

#ifdef OPENMP
namespace sim {
  template<typename T>
  void for_each_index(IndexSpan span, T body) {
    thrust::counting_iterator<std::size_t> begin(span.begin);
    thrust::counting_iterator<std::size_t> end(span.end);

    thrust::for_each(thrust::system::omp::par, begin, end, body);
  }
} // end namespace sim
#endif

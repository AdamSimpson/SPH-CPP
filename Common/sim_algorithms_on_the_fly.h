#pragma once

#include "parameters.h"
#include "dimension.h"
#include "execution_mode.h"
#include "device.h"
#include "vec.h"
#include "thrust/iterator/counting_iterator.h"
#include "thrust/sort.h"
#include "thrust/binary_search.h"
#include <thrust/system/omp/execution_policy.h>
#include <thrust/system/cuda/execution_policy.h>
#include "thrust/for_each.h"
#include <thrust/partition.h>

// @todo is this reasonable?
// This seems super janky with everything static
// Having all the algorithms derived from an object also seems janky
namespace sim {
  class algorithms {

  // @todo delete all generated functions

  public:
    static ExecutionMode execution_mode_;

    template<typename Real, Dimension Dim>
    static void process_parameters(Parameters<Real,Dim>& parameters) {
      execution_mode_ = parameters.execution_mode_;
    }

    // CUDA specific versions

    template<typename T>
    static void for_each_index_CUDA(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::cuda::par, begin, end, body);
      cudaDeviceSynchronize();
    }

    template<typename KeyIterator, typename ValueIterator>
    static void sort_by_key_CUDA(KeyIterator key_begin, KeyIterator key_end, ValueIterator value_begin) {
      thrust::sort_by_key(thrust::cuda::par, key_begin, key_end, value_begin);
      cudaDeviceSynchronize();
    }

    template<typename ForwardIterator, typename OutputIterator>
    static void lower_bound_CUDA(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::lower_bound(thrust::cuda::par, begin, end,
                          search_begin, search_end,
                          result);
      cudaDeviceSynchronize();
    }

    template<typename ForwardIterator, typename OutputIterator>
    static void upper_bound_CUDA(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::upper_bound(thrust::cuda::par, begin, end,
                          search_begin, search_end,
                          result);
      cudaDeviceSynchronize();
    }

    template<typename ForwardIterator, typename Predicate>
    static ForwardIterator partition_CUDA(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::cuda::par, begin, end, predicate);
      cudaDeviceSynchronize();
      return result;
    }

    // OMP specific versions

    template<typename T>
    static void for_each_index_OMP(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::omp::par, begin, end, body);
    }

    template<typename key_iterator, typename value_iterator>
    static void sort_by_key_OMP(key_iterator key_begin, key_iterator key_end, value_iterator value_begin) {
      thrust::sort_by_key(thrust::omp::par, key_begin, key_end, value_begin);
    }

    template<typename ForwardIterator, typename OutputIterator>
    static void lower_bound_OMP(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::lower_bound(thrust::omp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    template<typename ForwardIterator, typename OutputIterator>
    static void upper_bound_OMP(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::upper_bound(thrust::omp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    template<typename ForwardIterator, typename Predicate>
    static ForwardIterator partition_OMP(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::omp::par, begin, end, predicate);
      return result;
    }

    template<typename T>
    static void for_each_index(IndexSpan span, T body) {
        if(execution_mode_ == ExecutionMode::GPU)
          for_each_index_CUDA(span, body);
        else if(execution_mode_ == ExecutionMode::CPU)
          for_each_index_OMP(span, body);
    }

    template<typename KeyIterator, typename ValueIterator>
    static void sort_by_key(KeyIterator key_begin, KeyIterator key_end, ValueIterator value_begin) {
      if(execution_mode_ == ExecutionMode::GPU)
        sort_by_key_CUDA(key_begin, key_end, value_begin);
      else if(execution_mode_ == ExecutionMode::CPU)
        sort_by_key_OMP(key_begin, key_end, value_begin);
    }

    template<typename ForwardIterator, typename OutputIterator>
    static void lower_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      if(execution_mode_ == ExecutionMode::GPU)
        lower_bound_CUDA(begin, end, search_span, result);
      else if(execution_mode_ == ExecutionMode::CPU)
        lower_bound_OMP(begin, end, search_span, result);
    }

    template<typename ForwardIterator, typename OutputIterator>
    static void upper_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      if(execution_mode_ == ExecutionMode::GPU)
        upper_bound_CUDA(begin, end, search_span, result);
      else if(execution_mode_ == ExecutionMode::CPU)
        upper_bound_OMP(begin, end, search_span, result);
    }

    template<typename ForwardIterator, typename Predicate>
    static ForwardIterator partition(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      if(execution_mode_ == ExecutionMode::GPU)
        return partition_CUDA(begin, end, predicate);
      else
        return partition_OMP(begin, end, predicate);
    }

  };

  ExecutionMode algorithms::execution_mode_ = ExecutionMode::CPU;
} // end namespace sim

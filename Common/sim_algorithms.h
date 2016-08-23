#pragma once

#include "device.h"
#include "vec.h"
#include "thrust/iterator/counting_iterator.h"
#include "thrust/sort.h"
#include "thrust/binary_search.h"
#include "thrust/execution_policy.h"
#include "thrust/for_each.h"
#include <thrust/partition.h>

namespace sim {
  namespace algorithms {
#ifdef CUDA
    template<typename T>
    void for_each_index(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::system::cuda::par, begin, end, body);
      cudaDeviceSynchronize();
    }

    template<typename KeyIterator, typename ValueIterator>
    void sort_by_key(KeyIterator key_begin, KeyIterator key_end, ValueIterator value_begin) {
      thrust::sort_by_key(thrust::system::cuda::par, key_begin, key_end, value_begin);
      cudaDeviceSynchronize();
    }

    template<typename ForwardIterator, typename OutputIterator>
    void lower_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::lower_bound(thrust::system::cuda::par, begin, end,
                          search_begin, search_end,
                          result);
      cudaDeviceSynchronize();
    }

    template<typename ForwardIterator, typename OutputIterator>
    void upper_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::upper_bound(thrust::system::cuda::par, begin, end,
                          search_begin, search_end,
                          result);
      cudaDeviceSynchronize();
    }

    template<typename ForwardIterator, typename Predicate>
    ForwardIterator partition(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::system::cuda::par, begin, end, predicate);
      cudaDeviceSynchronize();
      return result;
    }

#endif

#ifdef OPENMP
    template<typename T>
    void for_each_index(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::system::omp::par, begin, end, body);
    }

    template<typename key_iterator, typename value_iterator>
    void sort_by_key(key_iterator key_begin, key_iterator key_end, value_iterator value_begin) {
      thrust::sort_by_key(thrust::system::omp::par, key_begin, key_end, value_begin);
    }

    template<typename ForwardIterator, typename OutputIterator>
    void lower_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::upper_bound(thrust::system::omp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    template<typename ForwardIterator, typename OutputIterator>
    void upper_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::lower_bound(thrust::system::omp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    template<typename ForwardIterator, typename Predicate>
    ForwardIterator partition(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::system::omp::par, begin, end, predicate);
      return result;
    }

#endif

#ifdef CPP_PAR
    template<typename T>
    void for_each_index(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::system::cpp::par, begin, end, body);
    }

    template<typename key_iterator, typename value_iterator>
    void sort_by_key(key_iterator key_begin, key_iterator key_end, value_iterator value_begin) {
      thrust::sort_by_key(thrust::system::cpp::par, key_begin, key_end, value_begin);
    }

    template<typename ForwardIterator, typename OutputIterator>
    void lower_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::upper_bound(thrust::system::cpp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    template<typename ForwardIterator, typename OutputIterator>
    void upper_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::lower_bound(thrust::system::cpp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    template<typename ForwardIterator, typename Predicate>
    ForwardIterator partition(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::system::cpp::par, begin, end, predicate);
      return result;
    }

#endif
  } // end namespace algorithm
} // end namespace sim

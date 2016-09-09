/*
The MIT License (MIT)

Copyright (c) 2016 Adam Simpson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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
    /*! Wrapper around thrust::for_each applied to counting_iterator using cuda device
     * @param span The span (] in which the body will be iterated over
     * @param body a Lambda function taking a std::size_t index argument ranging over the span
     */
    template<typename T>
    void for_each_index(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::system::cuda::par, begin, end, body);
      cudaDeviceSynchronize();
    }

    /*! Wrapper around Thrust::sort_by_key using cuda device
     * @param key_begin   Iterator to beginning of keys
     * @param key_end     Iterator to one past the end of keys
     * @param value_begin Iterator to beginning of values
     */
    template<typename KeyIterator, typename ValueIterator>
    void sort_by_key(KeyIterator key_begin, KeyIterator key_end, ValueIterator value_begin) {
      thrust::sort_by_key(thrust::system::cuda::par, key_begin, key_end, value_begin);
      cudaDeviceSynchronize();
    }

    /*! Wrapper around thrust::lower_bound applied to span using cuda device
     * @param begin       Iterator to beginning of ordered range
     * @param end         Iterator to end of ordered range
     * @param search_span Span over which the search will be preformed
     * @param result      Iterator to beginning of the output sequence
     */
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

    /*! Wrapper around thrust::upper_bound applied to span using cpp device
     * @param begin       Iterator to beginning of ordered range
     * @param end         Iterator to end of ordered range
     * @param search_span Span over which the search will be preformed
     * @param result      Iterator to beginning of the output sequence
     */
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

    /*! Wrapper around thrust::partition using cuda device
     * @param begin     Iterator to beginning of range to be partitioned
     * @param end       Iterator to end of range to be partitioned
     * @param predicate Predicate to partition on
     * @return          Iterator to first element of second partition
     */
    template<typename ForwardIterator, typename Predicate>
    ForwardIterator partition(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::system::cuda::par, begin, end, predicate);
      cudaDeviceSynchronize();
      return result;
    }

#endif

#ifdef OPENMP
    /*! Wrapper around thrust::for_each applied to counting_iterator using OpenMP device
     * @param span The span (] in which the body will be iterated over
     * @param body a Lambda function taking a std::size_t index argument ranging over the span
     */
    template<typename T>
    void for_each_index(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::system::omp::par, begin, end, body);
    }

    /*! Wrapper around Thrust::sort_by_key using OpenMP device
     * @param key_begin   Iterator to beginning of keys
     * @param key_end     Iterator to one past the end of keys
     * @param value_begin Iterator to beginning of values
     */
    template<typename key_iterator, typename value_iterator>
    void sort_by_key(key_iterator key_begin, key_iterator key_end, value_iterator value_begin) {
      thrust::sort_by_key(thrust::system::omp::par, key_begin, key_end, value_begin);
    }

    /*! Wrapper around thrust::lower_bound applied to span using OpenMP device
     * @param begin       Iterator to beginning of ordered range
     * @param end         Iterator to end of ordered range
     * @param search_span Span over which the search will be preformed
     * @param result      Iterator to beginning of the output sequence
     */
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

    /*! Wrapper around thrust::upper_bound applied to span using OpenMP device
     * @param begin       Iterator to beginning of ordered range
     * @param end         Iterator to end of ordered range
     * @param search_span Span over which the search will be preformed
     * @param result      Iterator to beginning of the output sequence
     */
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

    /*! Wrapper around thrust::partition using OpenMP device
     * @param begin     Iterator to beginning of range to be partitioned
     * @param end       Iterator to end of range to be partitioned
     * @param predicate Predicate to partition on
     * @return          Iterator to first element of second partition
     */
    template<typename ForwardIterator, typename Predicate>
    ForwardIterator partition(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::system::omp::par, begin, end, predicate);
      return result;
    }

#endif

#ifdef CPP_PAR
    /*! Wrapper around thrust::for_each applied to counting_iterator using cpp device
     * @param span The span (] in which the body will be iterated over
     * @param body a Lambda function taking a std::size_t index argument ranging over the span
     */
    template<typename T>
    void for_each_index(IndexSpan span, T body) {
      thrust::counting_iterator<std::size_t> begin(span.begin);
      thrust::counting_iterator<std::size_t> end(span.end);

      thrust::for_each(thrust::system::cpp::par, begin, end, body);
    }

    /*! Wrapper around Thrust::sort_by_key using cpp device
     * @param key_begin   Iterator to beginning of keys
     * @param key_end     Iterator to one past the end of keys
     * @param value_begin Iterator to beginning of values
     */
    template<typename key_iterator, typename value_iterator>
    void sort_by_key(key_iterator key_begin, key_iterator key_end, value_iterator value_begin) {
      thrust::sort_by_key(thrust::system::cpp::par, key_begin, key_end, value_begin);
    }

    /*! Wrapper around thrust::lower_bound applied to span using cpp device
     * @param begin       Iterator to begining of ordered range
     * @param end         Iterator to end of ordered range
     * @param search_span Span over which the search will be preformed
     * @param result      Iterator to beginning of the output sequence
     */
    template<typename ForwardIterator, typename OutputIterator>
    void lower_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::lower_bound(thrust::system::cpp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    /*! Wrapper around thrust::upper_bound applied to span using cpp device
     * @param begin       Iterator to begining of ordered range
     * @param end         Iterator to end of ordered range
     * @param search_span Span over which the search will be preformed
     * @param result      Iterator to beginning of the output sequence
     */
    template<typename ForwardIterator, typename OutputIterator>
    void upper_bound(ForwardIterator begin, ForwardIterator end,
                     IndexSpan search_span,
                     OutputIterator result) {
      thrust::counting_iterator<std::size_t> search_begin(search_span.begin);
      thrust::counting_iterator<std::size_t> search_end(search_span.end);

      thrust::upper_bound(thrust::system::cpp::par, begin, end,
                          search_begin, search_end,
                          result);
    }

    /*! Wrapper around thrust::partition using cpp device
     * @param begin     Iterator to beginning of range to be partitioned
     * @param end       Iterator to end of range to be partitioned
     * @param predicate Predicate to partition on
     * @return          Iterator to first element of second partition
     */
    template<typename ForwardIterator, typename Predicate>
    ForwardIterator partition(ForwardIterator begin, ForwardIterator end, Predicate predicate) {
      auto result = thrust::partition(thrust::system::cpp::par, begin, end, predicate);
      return result;
    }

#endif
  } // end namespace algorithm
} // end namespace sim

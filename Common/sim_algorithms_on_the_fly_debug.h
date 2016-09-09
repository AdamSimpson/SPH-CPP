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

#ifdef CUDA_DEBUG

#include <nvToolsExt.h>
#include <nvToolsExtCudaRt.h>

/**
  Wrapper functions to include NV debug markers
  Macros to pass calling function name to wrapper functions
**/

template<typename T>
static void for_each_index_debug(IndexSpan span, T body, const char *caller) {
  nvtxRangeId_t id1 = nvtxRangeStartA(caller);
  for_each_index(span, body);
  nvtxRangeEnd(id1);
}
#define for_each_index(...) for_each_index_debug (__VA_ARGS__, __func__)

template<typename KeyIterator, typename ValueIterator>
static void sort_by_key_debug(KeyIterator key_begin,
                              KeyIterator key_end,
                              ValueIterator value_begin,
                              const char *caller) {
  nvtxRangeId_t id1 = nvtxRangeStartA(caller);
  sort_by_key(key_begin, key_end, value_begin);
  nvtxRangeEnd(id1);
}
#define sory_by_key(...) sort_by_key_debug (__VA_ARGS__, __func__)

template<typename ForwardIterator, typename OutputIterator>
static void lower_bound_debug(ForwardIterator begin, ForwardIterator end,
                              IndexSpan search_span,
                              OutputIterator result,
                              const char *caller) {
  nvtxRangeId_t id1 = nvtxRangeStartA(caller);
  lower_bound(begin, end, search_span, result);
  nvtxRangeEnd(id1);
}
#define lower_bound(...) lower_bound_debug (__VA_ARGS__, __func__)

template<typename ForwardIterator, typename OutputIterator>
static void upper_bound_debug(ForwardIterator begin, ForwardIterator end,
                              IndexSpan search_span,
                              OutputIterator result,
                              const char *caller) {
  nvtxRangeId_t id1 = nvtxRangeStartA(caller);
  upper_bound(begin, end, search_span, result);
  nvtxRangeEnd(id1);
}
#define upper_bound(...) upper_bound_debug (__VA_ARGS__, __func__)

template<typename ForwardIterator, typename Predicate>
static ForwardIterator partition_debug(ForwardIterator begin,
                                       ForwardIterator end,
                                       Predicate predicate,
                                       const char *caller) {
  nvtxRangeId_t id1 = nvtxRangeStartA(caller);
  return partition(begin, end, predicate);
  nvtxRangeEnd(id1);
}
#define partition(...) partition_debug (__VA_ARGS__, __func__)

#endif

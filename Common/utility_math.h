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

#include <future>
#include "device.h"

namespace Utility {
  /*! Clamp value between lower and upper bound
   *
   * @param n     Value to clamp
   * @param lower Minimum allowed value
   * @param upper Maximum allowed value
   * @return      Clamped value
   */
  template<typename Util_T>
  DEVICE_CALLABLE
  Util_T clamp(const Util_T n, const Util_T &lower, const Util_T &upper) {
    return n < lower ? lower : n > upper ? upper : n;
  }

  /*! Clamp referenced value between lower and upper bound
   *
   * @param n     Value to clamp
   * @param lower Minimum allowed value
   * @param upper Maximum allowed value
   */
  template<typename Util_T>
  DEVICE_CALLABLE
  void clamp_in_place(Util_T &n, const Util_T &lower, const Util_T &upper) {
    if (n < lower)
      n = lower;
    else if (n > upper)
      n = upper;
  }

  /*! Test if std::future has completed
   *
   * @param f reference to future
   * @return true if future has completed
   */
  template<typename R>
  DEVICE_CALLABLE
  bool is_ready(std::future<R> const &f) {
    if (!f.valid()) // future hasn't been assigned yet
      return true;
    else
      return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
  }

}

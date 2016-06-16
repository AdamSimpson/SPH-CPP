#pragma once

#include <future>
#include "device.h"

namespace Utility {

template<typename Util_T>
DEVICE_CALLABLE
Util_T clamp(const Util_T n, const Util_T& lower, const Util_T& upper) {
  return n < lower ? lower : n > upper ? upper : n;
}

template<typename Util_T>
DEVICE_CALLABLE
void clamp_in_place(Util_T& n, const Util_T& lower, const Util_T& upper) {
  if(n < lower)
    n = lower;
  else if(n > upper)
    n = upper;
}

// Test if future has completed
template<typename R>
DEVICE_CALLABLE
bool is_ready(std::future<R> const& f) {
  if(!f.valid()) // future hasn't been assigned yet
    return true;
  else
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

}

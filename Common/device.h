#pragma once

// CUDA specific
#ifdef __CUDACC__
  #include "cuda_runtime.h"
  #define DEVICE_CALLABLE __host__ __device__
// NON-CUDA devices
#else
  #define DEVICE_CALLABLE
#endif

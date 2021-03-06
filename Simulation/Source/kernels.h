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

#include "dimension.h"
#include "vec.h"
#include "device.h"
#include <math.h>
#include <limits>

namespace sim {
/*!
 *  Generic Poly6 template class
 *  2D/3D specilizations provided
 */
template<typename Real, Dimension Dim> class Poly6;
/*!
 *  Generic gradient Poly6 template class
 *  2D/3D specilizations provided
 */
template<typename Real, Dimension Dim> class Del_Poly6;
/*!
 *  Generic gradient spikey template class
 *  2D/3D specilizations provided
 */
template<typename Real, Dimension Dim> class Del_Spikey;
/*!
 *  Generic C spline template class
 *  2D/3D specilizations provided
 */
template<typename Real, Dimension Dim> class C_Spline;

/*! 3D poly6 specilization
 **/
template <typename Real>
class Poly6<Real, three_dimensional> {
public:
  DEVICE_CALLABLE
  Poly6(const Real smoothing_radius): h_{smoothing_radius},
                                      norm_{static_cast<Real>(315.0/(64.0*M_PI*pow(h_, 9.0)))} {};

  DEVICE_CALLABLE
  Real operator()(const Real r_mag) const {
    if(r_mag > h_)
      return 0.0;

    return norm_ * (h_*h_ - r_mag*r_mag) * (h_*h_ - r_mag*r_mag) * (h_*h_ - r_mag*r_mag);
  };

private:
  Real h_;
  Real norm_;
};

/*! 2D poly6 specilization
**/
template <typename Real>
class Poly6<Real, two_dimensional> {
public:
  DEVICE_CALLABLE
  Poly6(const Real smoothing_radius): h_{smoothing_radius},
                                      norm_{static_cast<Real>(4.0/(M_PI*pow(h_, 8.0)))} {};

  DEVICE_CALLABLE
  Real operator()(const Real r_mag) const {
    if(r_mag > h_)
     return 0.0;

    return norm_ * (h_*h_ - r_mag*r_mag) * (h_*h_ - r_mag*r_mag) * (h_*h_ - r_mag*r_mag);
  };

private:
  Real h_;
  Real norm_;
};

/*! 3D gradient poly6 specilization
**/
template <typename Real>
class Del_Poly6<Real, three_dimensional> {
public:
  DEVICE_CALLABLE
  Del_Poly6(const Real smoothing_radius): h_{smoothing_radius},
                                          norm_{static_cast<Real>(-945.0/(32.0*M_PI*pow(h_, 9.0)))} {};

  DEVICE_CALLABLE
  Vec<Real,three_dimensional> operator()(const Vec<Real,three_dimensional>& vec_p,
                                         const Vec<Real,three_dimensional>& vec_q) const {
    const Vec<Real,three_dimensional> r = vec_p - vec_q;
    const Real r_mag_squared = magnitude_squared(r);
    const Real h_squared = h_*h_;
    if(r_mag_squared > h_squared)
      return Vec<Real,three_dimensional>(0.0);

    // r_mag / r_mag cancels out and is not included
    return norm_ * (h_squared - r_mag_squared) * (h_squared - r_mag_squared) *  r;
  };

private:
  Real h_;
  Real norm_;
};

/*! 2D gradient poly6 specilization
**/
template <typename Real>
class Del_Poly6<Real, two_dimensional> {
public:
  DEVICE_CALLABLE
  Del_Poly6(const Real smoothing_radius): h_{smoothing_radius},
                                          norm_{static_cast<Real>(-24.0/(M_PI*pow(h_, 8.0)))} {};

  DEVICE_CALLABLE
  Vec<Real,two_dimensional> operator()(const Vec<Real,two_dimensional>& vec_p,
                                         const Vec<Real,two_dimensional>& vec_q) const {
    const Vec<Real,two_dimensional> r = vec_p - vec_q;
    const Real r_mag_squared = magnitude_squared(r);
    const Real h_squared = h_*h_;

    if(r_mag_squared > h_squared)
      return Vec<Real,two_dimensional>(0.0);

    return norm_ * (h_squared - r_mag_squared) * (h_squared - r_mag_squared) *  r;
  };

private:
  Real h_;
  Real norm_;
};

/*! 3D gradient spikey specilization
**/
template <typename Real>
class Del_Spikey<Real, three_dimensional> {
public:
  DEVICE_CALLABLE
  Del_Spikey(const Real smoothing_radius): h_{smoothing_radius},
                                           norm_{static_cast<Real>(-45.0/(M_PI*pow(h_, 6.0)))},
                                           r_epsilon_(std::numeric_limits<Real>::epsilon()) {};

  DEVICE_CALLABLE
  Vec<Real,three_dimensional> operator()(const Vec<Real,three_dimensional>& vec_p,
                                         const Vec<Real,three_dimensional>& vec_q) const {
    const Vec<Real,three_dimensional> r = vec_p - vec_q;
    Real r_mag = magnitude(r);
    if(r_mag > h_)
      return Vec<Real,three_dimensional>{0.0};

    return norm_ * (h_ - r_mag) * (h_ - r_mag)  * r / (r_mag + r_epsilon_);
  };

private:
  const Real h_;
  const Real norm_;
  const Real r_epsilon_;
};

/*! 2D gradient spikey specilization
**/
template <typename Real>
class Del_Spikey<Real, two_dimensional> {
public:
  DEVICE_CALLABLE
  Del_Spikey(const Real smoothing_radius): h_{smoothing_radius},
                                           norm_{static_cast<Real>(-30.0/(M_PI*pow(h_, 5.0)))},
                                           r_min_(std::numeric_limits<Real>::epsilon()) {};

  DEVICE_CALLABLE
  Vec<Real,two_dimensional> operator()(const Vec<Real,two_dimensional>& vec_p,
                                       const Vec<Real,two_dimensional>& vec_q) const {
    const Vec<Real,two_dimensional> r = vec_p - vec_q;
    Real r_mag = magnitude(r);
    if(r_mag > h_ || r_mag < r_min_)
      return Vec<Real, two_dimensional>{0.0};

//    if(r_mag < r_min_)
//      r_mag = r_min_;

    return norm_ * (h_ - r_mag) * (h_ - r_mag) *  r / r_mag;
  };

private:
  Real h_;
  Real norm_;
  Real r_min_;
};

/*! 3D C_Spline specilization
   norm_ is artificial
**/
template <typename Real>
class C_Spline<Real, three_dimensional> {
public:
  DEVICE_CALLABLE
  C_Spline(const Real smoothing_radius): h_{smoothing_radius},
                                      norm_{static_cast<Real>(32.0/(M_PI*pow(h_, 9.0)))} {};

  DEVICE_CALLABLE
  Real operator()(const Real r) const {
    if(r > h_)
      return 0.0;
    else if(r <= h_*static_cast<Real>(0.5))
      return norm_ * ((2.0*(h_-r)*(h_-r)*(h_-r)*r*r*r) - (h_*h_*h_*h_*h_*h_/static_cast<Real>(64.0)));
    else
      return norm_ * (h_-r)*(h_-r)*(h_-r)*r*r*r;
  };

private:
  Real h_;
  Real norm_;
};

/*! 2D C_Spline specilization
   norm_ is artificial
**/
template <typename Real>
class C_Spline<Real, two_dimensional> {
public:
  DEVICE_CALLABLE
  C_Spline(const Real smoothing_radius): h_{smoothing_radius},
                                         norm_{static_cast<Real>(32.0/(M_PI*pow(h_, 8.0)))} {};

  DEVICE_CALLABLE
  Real operator()(const Real r) const {
    if(r > h_)
      return 0.0;
    else if(r <= h_*static_cast<Real>(0.5))
      return norm_ * ((2.0*(h_-r)*(h_-r)*(h_-r)*r*r*r) - (h_*h_*h_*h_*h_*h_/static_cast<Real>(64.0)));
    else
      return norm_ * (h_-r)*(h_-r)*(h_-r)*r*r*r;
  };

private:
  Real h_;
  Real norm_;
};
}

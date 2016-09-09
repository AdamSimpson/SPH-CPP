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
#include "dimension.h"
#include "vec.h"

/*! Generic AABB template class
 * 2D and 3D specilizations are provided
 */
template<typename T, Dimension dim> class AABB;

/*! 2D AABB specilization
 * 2 dimensional axis aligned bounding box class
 */
template<typename T>
class AABB<T, two_dimensional> {
public:

  Vec<T, two_dimensional> min; /**< minimum x,y coordinate **/
  Vec<T, two_dimensional> max; /**< maximum x,y coordinate **/

  /*! Default constructor
   * min and max undefined
   */
  AABB()                       = default;
  /*! Default destructor
   */
  ~AABB()                      = default;
  /*! Default copy constructor
   */
  AABB(const AABB&)            = default;
  /*! Default assignment operator
   */
  AABB& operator=(const AABB&) = default;
  /*! Default move constructor
   */
  AABB(AABB&&) noexcept        = default;
  /*! Default move assignment operator
   */
  AABB& operator=(AABB&&)      = default;

  /*! Return horizontal extent(length) of AABB
   */
  DEVICE_CALLABLE
  T length() const {
    return max.x - min.x;
  }

  /*! Return vertical extent(height) of AABB
   */
  DEVICE_CALLABLE
  T height() const {
    return max.y - min.y;
  }

  /*! Return area of AABB
   */
  DEVICE_CALLABLE
  T area() const {
    return this->length() * this->height();
  }

  /*! Return area of AABB
   * Volume is provided to simplify generic 2D/3D functions
   */
  DEVICE_CALLABLE
  T volume() const {
    return this->area();
  }

  /*! Return extent of AABB
   * @return a 2D Vec specifying the horizontal and vertical extents(length, height)
   */
  DEVICE_CALLABLE
  Vec<T,two_dimensional> extent() const {
    return max - min;
  }

  /*! Return center point of AABB
   * @return a 2D Vec specifying the geometric center point of the AABB
   */
  DEVICE_CALLABLE
  Vec<T,two_dimensional> center() const {
    return this->min + this->extent()/static_cast<T>(2);
  }

};

/*! 3D AABB specilization
 * 3 dimensional axis aligned bounding box class
 */
template<typename T>
class AABB<T, three_dimensional> {
public:

  Vec<T, three_dimensional> min; /**< minimum x,y,z coordinate **/
  Vec<T, three_dimensional> max; /**< maximum x,y,z coordinate **/

  /*! Default constructor
   * min and max undefined
   */
  AABB()                       = default;
  /*! Default destructor
   */
  ~AABB()                      = default;
  /*! Default copy constructor
   */
  AABB(const AABB&)            = default;
  /*! Default assignment operator
   */
  AABB& operator=(const AABB&) = default;
  /*! Default move constructor
   */
  AABB(AABB&&) noexcept        = default;
  /*! Default move assignment operator
   */
  AABB& operator=(AABB&&)      = default;

  /*! Return horizontal extent(length) of AABB
   */
  DEVICE_CALLABLE
  T length() const {
    return max.x - min.x;
  }

  /*! Return vertical extent(height) of AABB
   */
  DEVICE_CALLABLE
  T height() const {
    return max.y - min.y;
  }

  /*! Return z extent(depth) of AABB
   */
  DEVICE_CALLABLE
  T depth() const {
    return max.z - min.z;
  }

  /*! Return area of AABB
   */
  DEVICE_CALLABLE
  T area() const {
    return this->length() * this->height();
  }

  /*! Return area of AABB
   * Volume is provided to simplify generic 2D/3D functions
   */

  DEVICE_CALLABLE
  T volume() const {
    return this->length() * this->height() * this->depth();
  }

  /*! Return extent of AABB
   * @return a 3D Vec specifying the horizontal and vertical extents(length, height)
  **/
  DEVICE_CALLABLE
  Vec<T,three_dimensional> extent() const {
    return max - min;
  }

  /*! Return center point of AABB
   * @return a 3D Vec specifying the geometric center point of the AABB
   */
  DEVICE_CALLABLE
  Vec<T,three_dimensional> center() const {
    return this->min + this->extent()/static_cast<T>(2);
  }

  /*!
   * Cast operator: static_cast() components of aabb
   */
  template<typename T_out>
  operator AABB<T_out, three_dimensional>() {

    const auto min = static_cast<T_out>(this->min);
    const auto max = static_cast<T_out>(this->max);

    return AABB<T_out, three_dimensional>{min, max};
  }

};

/*! Return a Vec containing the number of bins
 *  that the AABB can be divided into given a bin size
 */
template <typename T, Dimension Dim>
DEVICE_CALLABLE
Vec<std::size_t, Dim> bin_count_in_volume(const AABB<T, Dim>& aabb, T spacing) {
  const Vec<T,Dim> space_counts{floor((aabb.max - aabb.min) / spacing)};
  Vec<std::size_t,Dim> int_counts(static_cast< Vec<std::size_t,Dim> >(space_counts));
    return int_counts;
  }

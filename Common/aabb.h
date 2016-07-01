#pragma once

#include "device.h"
#include "dimension.h"
#include "vec.h"

/////////////////////////////////////////////////
// A basic 2D and 3D Axis aligned boundary box
/////////////////////////////////////////////////

/**
  Empty generic template class
**/
template<typename Real, Dimension dim> class AABB;

/**
  2D AABB specilization
**/
template<typename Real>
class AABB<Real, two_dimensional> {
public:

  Vec<Real, two_dimensional> min; /**< minimum x,y coordinate **/
  Vec<Real, two_dimensional> max; /**< maximum x,y coordinate **/

  AABB()                       = default;
  ~AABB()                      = default;
  AABB(const AABB&)            = default;
  AABB& operator=(const AABB&) = default;
  AABB(AABB&&) noexcept        = default;
  AABB& operator=(AABB&&)      = default;


  /**
    @brief Return x extent of AABB
  **/
  DEVICE_CALLABLE
  Real length() const {
    return max.x - min.x;
  }

  /**
    @brief Return y extent of AABB
  **/
  DEVICE_CALLABLE
  Real height() const {
    return max.y - min.y;
  }

  /**
    @brief return area of 2D AABB
  **/
  DEVICE_CALLABLE
  Real area() const {
    return this->length() * this->height();
  }

  /**
    @brief return area of 2D AABB
  **/
  DEVICE_CALLABLE
  Real volume() const {
    return this->area();
  }

  /**
    @brief return length, width vector
  **/
  DEVICE_CALLABLE
  Vec<Real,two_dimensional> extent() const {
    return max - min;
  }

  /**
    @brief return center point of AABB
  **/
  DEVICE_CALLABLE
  Vec<Real,two_dimensional> center() const {
    return this->min + (Real)0.5 * this->extent();
  }

};

/**
  3D AABB specilization
**/
template<typename Real>
class AABB<Real, three_dimensional> {
public:

  Vec<Real, three_dimensional> min; /**< minimum x,y,z coordinate **/
  Vec<Real, three_dimensional> max; /**< maximum x,y,z coordinate **/

  AABB()                       = default;
  ~AABB()                      = default;
  AABB(const AABB&)            = default;
  AABB& operator=(const AABB&) = default;
  AABB(AABB&&) noexcept        = default;
  AABB& operator=(AABB&&)      = default;

  /**
    @brief Return x extent of AABB
  **/
  DEVICE_CALLABLE
  Real length() const {
    return max.x - min.x;
  }

  /**
    @brief Return y extent of AABB
  **/
  DEVICE_CALLABLE
  Real height() const {
    return max.y - min.y;
  }

  /**
    @brief Return z extent of AABB
  **/
  DEVICE_CALLABLE
  Real depth() const {
    return max.z - min.z;
  }

  /**
    @brief return volume of 3D AABB
  **/
  DEVICE_CALLABLE
  Real volume() const {
    return this->length() * this->height() * this->depth();
  }

  /**
    @brief return length, width, depth vector
  **/
  DEVICE_CALLABLE
  Vec<Real,three_dimensional> extent() const {
    return max - min;
  }

  /**
    @brief return center point of AABB
  **/
  DEVICE_CALLABLE
  Vec<Real,three_dimensional> center() const {
    return this->min + (Real)0.5 * this->extent();
  }

  /**
   @brief Cast operator: static_cast() components of aabb
  **/
  template<typename T_out>
  operator AABB<T_out, three_dimensional>() {

    const auto min = static_cast<T_out>(this->min);
    const auto max = static_cast<T_out>(this->max);

    return AABB<T_out, three_dimensional>{min, max};
  }
};

/**
   @brief return a vec containing the number of bins
   that the aabb can be divided into given a bin size
**/
template <typename Real, Dimension Dim>
DEVICE_CALLABLE
Vec<std::size_t, Dim> bin_count_in_volume(const AABB<Real, Dim>& aabb, Real spacing) {
  const Vec<Real,Dim> space_counts{floor((aabb.max - aabb.min) / spacing)};
  Vec<std::size_t,Dim> int_counts(static_cast< Vec<std::size_t,Dim> >(space_counts));
    return int_counts;
  }

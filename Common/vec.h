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
#include <iostream>
#include <cmath>
#include "utility_math.h"
#include "dimension.h"

/////////////////////////////////////////////////
// A basic vector class to handle arbitrary type
// and dimensions
/////////////////////////////////////////////////

/*! Determine the alignment of struct - requires C++14 to work correctly
 */
constexpr int vec_alignment(const size_t struct_bytes) {
/*
  // Available CUDA alignments
  int alignments[5] = {1, 2, 3, 8, 16};

  // Get first available alignment >= to struct size
  int first_larger = -1;
  for(int alignment : alignments) {
    first_larger = alignment;
    if(first_larger >= struct_bytes)
      break;
  }
  return first_larger;
*/
  return 8;
}

/**
  @struct Vec
  @brief Generic n-dim Vec implementation
  @tparam T component type
  @tparam n dimension
 */
template <typename T, int N>
struct Vec {

  T data_[N]; /**< Component storage **/

  /*! Default constructor: components uninitialized
   */
  DEVICE_CALLABLE
  Vec() = default;

  /*! Pointer constructor: components initilized to pointed data
     @param p_data pointer to data
   */
  DEVICE_CALLABLE
  explicit Vec(const T *const p_data){ for(int i=0; i<N; ++i){ data_[i] = p_data[i]; } }

  /*! Scalar constructor: components initilized to scalar value
     @param value scalar component value
   */
  DEVICE_CALLABLE
  explicit Vec(const T value){ for(int i=0; i<N; ++i){ data_[i] = value;} }

  /*! Cast operator: static_cast() components of n-dim Vec
   */
  template<typename T_out>
  DEVICE_CALLABLE
  operator Vec<T_out,N>() {
    T_out data_out[N];
    for(int i=0; i<N; ++i) {
      data_out[i] = static_cast<T_out>(data_[i]);
    }
    return Vec<T_out,N>(data_out);
  }

  /*! Subscript operator: access vector components with bracket notation
     @param index of element
     @return index'th element of Vec
   */
  DEVICE_CALLABLE
  T& operator[] (const size_t index) { return data_[index]; }

  /*! Const subscript operator: access vector components with bracket notation
     @param index of element
     @return index'th element of Vec
   */
  DEVICE_CALLABLE
  const T& operator[] (const size_t index) const { return data_[index]; }

};

/*! Template specialization for 2-dim Vec
 */
template <typename T>
struct Vec<T, 2> {

  union {
    T data_[2];          /**< component array */
    struct { T x, y; }; /**<  x, y access */
    struct { T begin, end; }; /**< begin, end access */
  }; /**< union component storage */

  /*! Default constructor: components uninitialized
   */
  DEVICE_CALLABLE
  Vec() = default;

  /*! Constructor: components initilized
     @param x_ x component
     @param y_ y component
   */
  DEVICE_CALLABLE
  explicit Vec(const T x_, const T y_) : x(x_), y(y_) {}

  /*! Constructor: components initilized to value
     @param value x, y component
   */
  DEVICE_CALLABLE
  constexpr explicit Vec(const T value) : x(value), y(value) {}

  /*! Pointer constructor: components initilized to pointed data
     @param p_data pointer to data
   */
  DEVICE_CALLABLE
  constexpr explicit Vec(T *data_in) : x(data_in[0]), y(data_in[1]) {}

  /*! Construct Vec2 from Vec3 by truncating z
     @param vec input Vec3
   */
  DEVICE_CALLABLE
  constexpr explicit Vec(const Vec<T,3>& vec): x{vec.x}, y{vec.y} {}

  /*! Cast operator: static_cast() data of Vec2
   */
  template<typename T_out>
  DEVICE_CALLABLE
  operator Vec<T_out,2>() const {
    return Vec<T_out,2>(static_cast<T_out>(x), static_cast<T_out>(y)); }

  /*! Subscript operator: access vector components with bracket notation
     @param index of element
     @return index'th element of Vec
   */
  DEVICE_CALLABLE
  T& operator[] (const size_t index) { return data_[index]; }
  DEVICE_CALLABLE
  T& operator[] (const int index) { return data_[index]; }

  /*! Const subscript operator: access vector components with bracket notation
     @param[in] index of element
     @return index'th element of Vec
   */
  DEVICE_CALLABLE
  const T& operator[] (const size_t index) const { return data_[index]; }
  DEVICE_CALLABLE
  const T& operator[] (const int index) const { return data_[index]; }
};

/**
  @struct Vec
  @brief Specialization for 3-dim Vec
 */
template <typename T>
struct Vec<T, 3> {

  union {
    T data_[3];              /**< component array */
    struct { T x, y, z; };  /**< x, y, z access  */
    struct { T r, g, b; };  /**< r, g, b access  */
    struct { T h, s, v; };  /**< h, s, v access */
  };

  /*! Default constructor: components uninitialized
  **/
  DEVICE_CALLABLE
  Vec() = default;

  /*! Constructor: components initilized
     @param x_ x component
     @param y_ y component
     @param z_ z component
  **/
  DEVICE_CALLABLE
  explicit Vec(const T x_, const T y_, const T z_) : x(x_), y(y_), z(z_) {}

  /*! Constructor: components initilized to value
     @param value x, y, z  component
  **/
  DEVICE_CALLABLE
  constexpr explicit Vec(const T value) : x(value), y(value), z(value) {}

  /*! Pointer constructor: components initilized to pointed data
     @param p_data pointer to data
  **/
  DEVICE_CALLABLE
  constexpr explicit Vec(T *data) : x(data[0]), y(data[1]), z(data[2]) {}

  /*! Construct Vec2 from Vec3 by setting z to 0
     @param vec Vec2
  **/
  DEVICE_CALLABLE
  explicit Vec(const Vec<T, 2>& vec): x{vec[0]}, y{vec[1]}, z{0} {}

  /*! Construct Vec2 from Vec3 by specifying z
     @param vec Vec2 x,y values
     @param z z value
  **/
  DEVICE_CALLABLE
  explicit Vec(const Vec<T, 2>& vec, T z): x{vec[0]}, y{vec[1]}, z{z} {}

  /*! Cast operator: static_cast() data of Vec3
  **/
  template<typename T_out>
  DEVICE_CALLABLE
  operator Vec<T_out,3>() const
    { return Vec<T_out,3>(static_cast<T_out>(x), static_cast<T_out>(y), static_cast<T_out>(z)); }

  /*! Subscript operator: access vector components with bracket notation
     @param index of element
     @return index'th element of Vec
  **/
  DEVICE_CALLABLE
  T& operator[] (const size_t index) { return data_[index]; }
  DEVICE_CALLABLE
  T& operator[] (const int index) { return data_[index]; }

  /*! Const subscript operator: access vector components with bracket notation
     @param index of element
     @return index'th element of Vec
  **/
  DEVICE_CALLABLE
  const T& operator[] (const size_t index) const { return data_[index]; }
  DEVICE_CALLABLE
  const T& operator[] (const int index) const { return data_[index]; }
};

/**
Free Operators
**/

/*! vec-vec component wise addition operator
 * @param lhs left side vector to add
 * @param rhs right side vector to add
 * @return lhs[i] + rhs[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator+(const Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs[i] + rhs[i];

  return Vec<T,n>(data);
}

/*! vec-scalar addition operator
 * @param lhs left side vector to add
 * @param rhs right side scalar to add
 * @return lhs[i] + rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator+(const Vec<T,n>& lhs, const T rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs[i] + rhs;

  return Vec<T,n>(data);
}

/*! vec-vec component wise subtraction operator
 * @param lhs left side vector
 * @param rhs right side vector to subtract
 * @return lhs[i] - rhs[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator-(const Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs[i] - rhs[i];

  return Vec<T,n>(data);
}

/*! vec-scalar subtraction operator
 * @param lhs left side vector
 * @param rhs right side scalar to subtract
 * @return lhs[i] - rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator-(const Vec<T,n>& lhs, const T rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs[i] - rhs;

  return Vec<T,n>(data);
}

/*! vec-vec component wise multiplication operator
 * @param lhs left side vector to multiply
 * @param rhs right side vector to multiply
 * @return lhs[i] * rhs[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator*(const Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs[i] * rhs[i];

  return Vec<T,n>(data);
}

/*! vec-scalar multiplication operator
 * @param lhs left side vector to multiply
 * @param rhs right side scalar
 * @return lhs[i] * rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator*(const Vec<T,n>& lhs, const T rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs[i] * rhs;

  return Vec<T,n>(data);
}

/*! scalar-vec multiplication operator
 * @param lhs left side scalar
 * @param rhs right side vector to multiply
 * @return lhs[i] * rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator*(const T lhs, const Vec<T,n>& rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs * rhs[i];

  return Vec<T,n>(data);
}

/*! vec-scalar division operator
 * @param lhs left side vector
 * @param rhs right side scalar to divide by
 * @return lhs[i] / rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> operator/(const Vec<T,n>& lhs, const T rhs) {
  T data[n];
  for(int i=0; i<n; ++i)
    data[i] = lhs[i] / rhs;

  return Vec<T,n>(data);
}

/*! vec-vec component wise accumulate operator
 * @param lhs vector to be accumulated into
 * @param rhs vector to be accumulated by
 * @return lhs[i] += rhs[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator+=(Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] += rhs[i];

  return lhs;
}

/*! vec-scalar component wise accumulate operator
 * @param lhs vector to be accumulated into
 * @param rhs scalar to be accumulated by
 * @return lhs[i] += rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator+=(Vec<T,n>& lhs, const T rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] += rhs;

  return lhs;
}

/*! vec-vec component wise decrement operator
 * @param lhs vector to be decremented into
 * @param rhs vector to be decremented by
 * @return lhs[i] -= rhs[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator-=(Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] -= rhs[i];

  return lhs;
}

/*! vec-scalar component wise decrement operator
 * @param lhs vector to be decremented into
 * @param rhs scalar to be decremented by
 * @return lhs[i] -= rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator-=(Vec<T,n>& lhs, const T rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] -= rhs;

  return lhs;
}

/*! vec-vec component wise multiplication assignment operator
 * @param lhs vector to be multiply assigned into
 * @param rhs vector to be multiplied by
 * @return lhs[i] *= rhs[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator*=(Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] *= rhs[i];

  return lhs;
}

/*! vec-scalar component wise multiplication assignment operator
 * @param lhs vector to be multiply assigned into
 * @param rhs scalar to be multiplied by
 * @return lhs[i] *= rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator*=(Vec<T,n>& lhs, const T& rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] *= rhs;

  return lhs;
}

/*! vec-vec component wise division assignment operator
 * @param lhs vector to be division assigned into
 * @param rhs vector to be divided by
 * @return lhs[i] /= rhs[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator/=(Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] /= rhs[i];

  return lhs;
}

/*! vec-scalar component wise division assignment operator
 * @param lhs vector to be division assigned into
 * @param rhs scalar to be divided by
 * @return lhs[i] /= rhs
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n>& operator/=(Vec<T,n>& lhs, const T& rhs) {
  for(int i=0; i<n; ++i)
    lhs[i] /= rhs;

  return lhs;
}

/*! Vector dot product
 *
 * @param lhs left vector
 * @param rhs right vector
 * @return vector dot product
 */
template<typename T, int n>
DEVICE_CALLABLE
T dot(const Vec<T,n>& lhs, const Vec<T,n>& rhs) {
  T dot = static_cast<T>(0);
  for(int i=0; i<n; ++i)
    dot += lhs[i] * rhs[i];

  return dot;
}

/*! Vector magnitude squared
 * @param vec vector
 * @return magnitude squared of the scalar magnitude
 */
template<typename T, int n>
DEVICE_CALLABLE
T magnitude_squared(const Vec<T,n>& vec) {
  return dot(vec, vec);
}

/*! Vector magnitude
 * @param vec vector
 * @return magnitude of the scalar magnitude
 */
template<typename T, int n>
DEVICE_CALLABLE
T magnitude(const Vec<T,n>& vec) {
  return sqrt(magnitude_squared(vec));
}

/*! Reciprical vector magnitude
 * @param vec vector
 * @return reciprical of the magnitude of the scalar magnitude
 */
template<typename T, int n>
DEVICE_CALLABLE
T inverse_magnitude(const Vec<T,n>& vec) {
  return static_cast<T>(1.0) / sqrt(magnitude_squared(vec));
}

/*! Vector normal
 * @param vec vector to take normal of
 * @return a vector in the same direction of vec but of unit magnitude
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> normal(const Vec<T,n>& vec) {
  return inverse_magnitude(vec) * vec;
}

/*! Normalize vector
 * @param vec vector to normalize
 */
template<typename T, int n>
DEVICE_CALLABLE
void normalize(Vec<T,n>& vec) {
  vec *= inverse_magnitude(vec);
}

/*! Component wise floor
 * @param vec vector to floor
 * @return vector containing floored components
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> floor(const Vec<T,n>& vec) {
   Vec<T,n> floor_vec;
   for(int i=0; i<n; ++i)
     floor_vec[i] = floor(vec[i]);

  return floor_vec;
}

/*! Component wise ceil
 * @param vec vector to ceil
 * @return vector containing ceiled components
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> ceil(const Vec<T,n>& vec) {
   Vec<T,n> floor_vec;
   for(int i=0; i<n; ++i)
     floor_vec[i] = std::ceil(vec[i]);

  return floor_vec;
}

/*! Sum of vector components
 * @param vec vector to provide sum of
 * @return the scalar sum of each component of vector
 */
template<typename T, int n>
DEVICE_CALLABLE
T sum(const Vec<T,n>& vec) {
  T sum = static_cast<T>(0);
  for(int i=0; i<n; ++i)
    sum += vec[i];

  return sum;
}

/*! Component wise product of vector components
 * @param vec vector to provide product of
 * @return the scalar product of each component of the vector
 */
template<typename T, int n>
DEVICE_CALLABLE
T product(const Vec<T,n>& vec) {
  T product = static_cast<T>(1);
  for(int i=0; i<n; ++i)
    product *= vec[i];

  return product;
}

/*! 3D vector cross product
 * @param lhs vector to be crossed into
 * @param rhs vector to cross
 * @return 3D vector cross product
 */
template<typename T>
DEVICE_CALLABLE
Vec<T,three_dimensional> cross(const Vec<T, three_dimensional>& lhs,
                               const Vec<T, three_dimensional>& rhs) {
  Vec<T,three_dimensional> cross;
  cross.x = lhs.y * rhs.z - lhs.z * rhs.y;
  cross.y = lhs.z * rhs.x - lhs.x * rhs.z;
  cross.z = lhs.x * rhs.y - lhs.y * rhs.x;

  return cross;
}

/*! Component wise vector clamp
 * @param vec   Vector to be clamped
 * @param lower Vector containing minimum allowed values
 * @param upper Vector containing maximum allowed values
 * @return      a vector with components clamped between lower[i] and upper[i]
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> clamp(const Vec<T,n>& vec, const Vec<T,n>& lower, const Vec<T,n>& upper) {
   Vec<T,n> clamp_vec;
   for(int i=0; i<n; ++i)
     clamp_vec[i] = Utility::clamp(vec[i], lower[i], upper[i]);

  return clamp_vec;
}

/*! Component wise scalar clamp
 * @param vec   Vector to be clamped
 * @param lower Minimum allowed scalar value
 * @param upper Maximum allowed scalar value
 * @return      a vector with components clamped between lower and upper
 */
template<typename T, int n>
DEVICE_CALLABLE
Vec<T,n> clamp(const Vec<T,n>& vec, const T& lower, const T& upper) {
  Vec<T,n> clamp_vec;
  for(int i=0; i<n; ++i)
    clamp_vec[i] = Utility::clamp(vec[i], lower, upper);

  return clamp_vec;
}

/*! In-place component wise vector clamp
 * @param vec   Vector to be clamped
 * @param lower Vector containing minimum allowed values
 * @param upper Vector containing maximum allowed values
 */
template<typename T, int n>
DEVICE_CALLABLE
void clamp_in_place(Vec<T,n>& vec, const Vec<T,n>& lower, const Vec<T,n>& upper) {
   for(int i=0; i<n; ++i)
     Utility::clamp_in_place(vec[i], lower[i], upper[i]);
}

/*! In-place component wise scalar clamp
 * @param vec   Vector to be clamped
 * @param lower Minimum allowed scalar value
 * @param upper Maximum allowed scalar value
 */
template<typename T, int n>
DEVICE_CALLABLE
void clamp_in_place(Vec<T,n>& vec, const T& lower, const T& upper) {
  for(int i=0; i<n; ++i)
    Utility::clamp_in_place(vec[i], lower, upper);
}

/** left shift operator: allow Vec<> to be printed
 * @param o ostream
 * @param vec vector to print
 * @return modified reference to o
 */
template<typename T, int n>
std::ostream& operator<< (std::ostream& o, const Vec<T,n>& vec)
{
  o << "{";
  for(int i=0; i < n; ++i) {
    if(i != 0)
      o << ", ";
    o << vec[i];
  }
  o << "}";
  return o;
}

typedef Vec<std::size_t, 2> IndexSpan; /**< type to hold begin, end index values **/

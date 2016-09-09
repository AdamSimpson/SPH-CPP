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

#include "catch.hpp"
#include "aabb.h"

SCENARIO( "AABBs provide basic axis aligned bounding box functionality", "[AABB]" ) {
  GIVEN("an AABB<float,3> with min(0.0, -1.0, 2.0) and max(1.0, 2.0, 3.0)") {
    AABB<float,3> a;
    a.min = Vec<float,3>(0.0f, -1.0f, -2.0f);
    a.max = Vec<float,3>(1.0f, 2.0f, -1.0f);

    WHEN("the length is taken") {
      const float l = a.length();
      THEN("1.0f is returned") {
        REQUIRE( l == Approx(1.0f) );
      }
    }
    WHEN("the depth is taken") {
      const float d = a.depth();
      THEN("5.0f is returned") {
        REQUIRE( d == Approx(1.0f) );
      }
    }
    WHEN("the height is taken") {
      const float h = a.height();
      THEN("3.0f is returned") {
        REQUIRE( h == Approx(3.0f) );
      }
    }
    WHEN("the volume is taken") {
      const float v = a.volume();
      THEN("3.0f is returned") {
        REQUIRE( v == Approx(3.0f) );
      }
    }
    WHEN("the extent is taken") {
      const Vec<float,3> v = a.extent();
      THEN("a Vec<float,3>(1.0, 3.0, 1.0) is returned") {
        REQUIRE( v.x == Approx(1.0f) );
        REQUIRE( v.y == Approx(3.0f) );
        REQUIRE( v.z == Approx(1.0f) );
      }
    }
    WHEN("the center is taken") {
      const Vec<float,3> v = a.center();
      THEN("a Vec<float,3>(0.5, 0.5, -0.5) is returned") {
        REQUIRE( v.x == Approx(0.5f) );
        REQUIRE( v.y == Approx(0.5f) );
        REQUIRE( v.z == Approx(-1.5f) );
      }
    }
  }
}

SCENARIO( "The bin count can be computed", "[AABB]" ) {
  GIVEN("an AABB<float,3> with min(0.0, -1.0, 2.0) and max(1.0, 2.0, 3.0) and a bin width of 1.0f") {
    AABB<float,3> a;
    a.min = Vec<float,3>(0.0f, -1.0f, -2.0f);
    a.max = Vec<float,3>(1.0f, 2.0f, -1.0f);
    float s = 1.0f;

    WHEN("the bin count is computed") {
      const Vec<std::size_t,3> c = bin_count_in_volume(a, s);
      THEN("the bin count should be Vec(1, 3, 1)") {
        REQUIRE( c.x == 1 );
        REQUIRE( c.y == 3 );
        REQUIRE( c.z == 1 );
      }
    }
  }
  GIVEN("an AABB<float,3> with min(0.0, 0.0, 0.0) and max(0.5, 1.4, 1.6) and a bin width of 0.5f") {
    AABB<float,3> a;
    a.min = Vec<float,3>(0.0f, 0.0f, 0.0f);
    a.max = Vec<float,3>(0.5f, 1.4f, 1.6f);
    float s = 0.5f;

    WHEN("the bin count is computed") {
      const Vec<std::size_t,3> c = bin_count_in_volume(a, s);
      THEN("the bin count should be Vec(1, 2, 3)") {
        REQUIRE( c.x == 1 );
        REQUIRE( c.y == 2 );
        REQUIRE( c.z == 3 );
      }
    }
  }
}

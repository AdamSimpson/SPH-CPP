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
#include "vec.h"
#include "utility_math.h"
#include <type_traits>

SCENARIO( "Vecs can be constructed", "[Vec]" ) {

  GIVEN("Nothing") {
    WHEN("a Vec<float,2> is constructed") {
      Vec<float,2> v;
      THEN("no error occurs") {}
    }

    WHEN("a Vec<float,3> is constructed") {
      Vec<float,3>();
      THEN("no error occurs") {}
    }
  }

  GIVEN("a pointer to 3 floats") {
    float data[3] = {0.0f, 1.0f, 2.0f};
    float *p = data;

    WHEN("a Vec<float,2> is constructed from it") {
      Vec<float,2> v(p);
      THEN("the vec data should equal the pointed to data") {
        REQUIRE( v[0] == data[0] );
        REQUIRE( v[1] == data[1] );
      }
    }

    WHEN("a Vec<float,3> is constructed from it") {
      Vec<float,3> v(p);
      THEN("the vec data should equal the pointed to data") {
        REQUIRE( v[0] == data[0] );
        REQUIRE( v[1] == data[1] );
        REQUIRE( v[2] == data[2] );
      }
    }
  }

  GIVEN("a float") {
    float f = 7.0f;

    WHEN("a Vec<float,2> is constructed from it") {
      Vec<float,2> v(f);
      THEN("the vec data should equal the float") {
        REQUIRE( v[0] == f );
        REQUIRE( v[1] == f );
      }
    }

    WHEN("a Vec<float,3> is constructed from it") {
      Vec<float,3> v(f);
      THEN("the vec data should equal the float") {
        REQUIRE( v[0] == f );
        REQUIRE( v[1] == f );
        REQUIRE( v[2] == f );
      }
    }
  }

  GIVEN("three floats") {
    float f1 = 1.0f;
    float f2 = 2.0f;
    float f3 = 3.0f;

    WHEN("a Vec<float,2> is constructed from them") {
      Vec<float,2> v(f1,f2);
      THEN("the vec data should equal the floats") {
        REQUIRE( v[0] == f1 );
        REQUIRE( v[1] == f2 );
      }
    }

    WHEN("a Vec<float,3> is constructed from them") {
      Vec<float,3> v(f1,f2,f3);
      THEN("the vec data should equal the floats") {
        REQUIRE( v[0] == f1 );
        REQUIRE( v[1] == f2 );
        REQUIRE( v[2] == f3 );
      }
    }
  }

  GIVEN("a Vec<float,2> v") {
    Vec<float,2> v(0.0f ,1.0f);
    WHEN("a Vec<float,3> w is constructed from it") {
      Vec<float,3> w(v);
      THEN("the first two elements should be from the v and the last 0.0") {
        REQUIRE( w[0] == v[0] );
        REQUIRE( w[1] == v[1] );
        REQUIRE( w[2] == Approx(0.0f) );
      }
    }
  }
} // END Vecs can be constructed

SCENARIO( "Vecs can be static_cast", "[Vec]" ) {
  GIVEN("a Vec<float,3>") {
    Vec<float,3> v(0.0f, 1.0f, 2.0f);

    WHEN("it's cast to an Vec<int,2>") {
      auto c_v = static_cast<Vec<int,2>>(v);
      THEN("The resultant vector should be a Vec<int,2>") {
        bool same_type = std::is_same<decltype(c_v), Vec<int,2> >::value;
        REQUIRE( same_type );
        REQUIRE( c_v[0] == static_cast<int>(v[0]) );
        REQUIRE( c_v[1] == static_cast<int>(v[1]) );
      }
    }
  }
} // END Vecs can be static_cast

SCENARIO( "Vecs can be accessed with bracket operator []", "[Vec]" ) {
  GIVEN("a vec<float,3>") {
    Vec<float,3> v(0.0, 1.0, 2.0);

    WHEN("the data is accessed with brackets []") {
      THEN("The operator returns the correct data") {
        REQUIRE( v[0] == v.data_[0] );
        REQUIRE( v[1] == v.data_[1] );
        REQUIRE( v[2] == v.data_[2] );
      }
    }
  }
} // END Vecs can be accessed with bracket operator []

SCENARIO( "Vecs can be added and subtracted", "[Vec]" ) {
  GIVEN("two Vec<float,3> and a float scalar") {
    const Vec<float,3> v(0.0f, 1.0f, 2.0f);
    const Vec<float,3> w(3.0f, 4.0f, 5.0f);
    const float scalar = 1.0f;

    WHEN("vectors are added") {
      const auto z = v + w;
      THEN("addition should be component wise") {
        REQUIRE( z[0] == Approx(v[0] + w[0]) );
        REQUIRE( z[1] == Approx(v[1] + w[1]) );
        REQUIRE( z[2] == Approx(v[2] + w[2]) );
      }
    }

    WHEN("a scalar is added to a vector") {
      const auto z = v + scalar;
      THEN("the scalar should be added to each component") {
        REQUIRE( z[0] == Approx(v[0] + scalar) );
        REQUIRE( z[1] == Approx(v[1] + scalar) );
        REQUIRE( z[2] == Approx(v[2] + scalar) );
      }
    }

    WHEN("vectors are subtracted") {
      const auto z = v - w;
      THEN("subtraction should be component wise") {
        REQUIRE( z[0] == Approx(v[0] - w[0]) );
        REQUIRE( z[1] == Approx(v[1] - w[1]) );
        REQUIRE( z[2] == Approx(v[2] - w[2]) );
      }
    }

    WHEN("a scalar is subtracted from a vector") {
      const auto z = v - scalar;
      THEN("the scalar should be added to each component") {
        REQUIRE( z[0] == Approx(v[0] - scalar) );
        REQUIRE( z[1] == Approx(v[1] - scalar) );
        REQUIRE( z[2] == Approx(v[2] - scalar) );
      }
    }
  }
} // END Vecs can be added and subtracted


SCENARIO( "Vecs can be multiplied and divided", "[Vec]" ) {
  GIVEN("two Vec<float,3> and a float scalar") {
    const Vec<float,3> v(0.0f, 1.0f, 2.0f);
    const Vec<float,3> w(3.0f, 4.0f, 5.0f);
    const float scalar = 1.0f;

    WHEN("the vectors are multiplied") {
      const auto z = v * w;
      THEN("they are multiplied component wise") {
        REQUIRE( z[0] == Approx(v[0] * w[0]) );
        REQUIRE( z[1] == Approx(v[1] * w[1]) );
        REQUIRE( z[2] == Approx(v[2] * w[2]) );
      }
    }

    WHEN("the vector is multiplied by the scalar") {
      const auto z = v * scalar;
      THEN("the vector is multiplied component wise") {
        REQUIRE( z[0] == Approx(v[0] * scalar) );
        REQUIRE( z[1] == Approx(v[1] * scalar) );
        REQUIRE( z[2] == Approx(v[2] * scalar) );
      }
    }

    WHEN("the scalar is multiplied by the vector") {
      const auto z = scalar * v;
      THEN("the vector is multiplied component wise") {
        REQUIRE( z[0] == Approx(v[0] * scalar) );
        REQUIRE( z[1] == Approx(v[1] * scalar) );
        REQUIRE( z[2] == Approx(v[2] * scalar) );
      }
    }

    WHEN("the vector is divided by the scalar") {\
      const auto z = v / scalar;
      THEN("the vector is divided component wise") {
        REQUIRE( z[0] == Approx(v[0] / scalar) );
        REQUIRE( z[1] == Approx(v[1] / scalar) );
        REQUIRE( z[2] == Approx(v[2] / scalar) );
      }
    }
  }
} // END Vecs can be multiplied and divided

SCENARIO( "Vecs can be decremented and incremented", "[Vec]" ) {
  GIVEN("two Vec<float,3> v,w") {
    Vec<float,3> v(0.0f, 1.0f, 2.0f);
    const Vec<float,3> w(1.0f, 2.0f, 3.0f);
    WHEN("v is incremented by w") {
      const auto v_orig(v);
      v += w;
      THEN("v should be incremented component wise by w") {
        REQUIRE( v[0] == Approx(v_orig[0] + w[0]) );
        REQUIRE( v[1] == Approx(v_orig[1] + w[1]) );
        REQUIRE( v[2] == Approx(v_orig[2] + w[2]) );
      }
    }
    WHEN("v is decremented by w") {
      const auto v_orig(v);
      v -= w;
      THEN("v should be incremented component wise by w") {
        REQUIRE( v[0] == Approx(v_orig[0] - w[0]) );
        REQUIRE( v[1] == Approx(v_orig[1] - w[1]) );
        REQUIRE( v[2] == Approx(v_orig[2] - w[2]) );
      }
    }
  }

  GIVEN("a Vec<float,3> v and a float s") {
    Vec<float,3> v(0.0f, 1.0f, 2.0f);
    const float s = 1.0f;
    WHEN("v is incremented by s") {
      const auto v_orig(v);
      v += s;
      THEN("v should be incremented component wise by s") {
        REQUIRE( v[0] == Approx(v_orig[0] + s) );
        REQUIRE( v[1] == Approx(v_orig[1] + s) );
        REQUIRE( v[2] == Approx(v_orig[2] + s) );
      }
    }
    WHEN("v is decremented by s") {
      const auto v_orig(v);
      v -= s;
      THEN("v should be decremented component wise by s") {
        REQUIRE( v[0] == Approx(v_orig[0] - s) );
        REQUIRE( v[1] == Approx(v_orig[1] - s) );
        REQUIRE( v[2] == Approx(v_orig[2] - s) );
      }
    }
  }
} // END Vecs can be decremented and incrimented

SCENARIO( "Vecs can be multiply and divide equaled", "[Vec]" ) {
  GIVEN("two Vec<float,3> v,w") {
    Vec<float,3> v(0.0f, 1.0f, 2.0f);
    const Vec<float,3> w(1.0f, 2.0f, 3.0f);
    WHEN("v is multiply equaled by w") {
      const auto v_orig(v);
      v *= w;
      THEN("v should be incremented component wise by w") {
        REQUIRE( v[0] == Approx(v_orig[0] * w[0]) );
        REQUIRE( v[1] == Approx(v_orig[1] * w[1]) );
        REQUIRE( v[2] == Approx(v_orig[2] * w[2]) );
      }
    }
    WHEN("v is divide equaled by w") {
      const auto v_orig(v);
      v /= w;
      THEN("v should be divided component wise by w") {
        REQUIRE( v[0] == Approx(v_orig[0] / w[0]) );
        REQUIRE( v[1] == Approx(v_orig[1] / w[1]) );
        REQUIRE( v[2] == Approx(v_orig[2] / w[2]) );
      }
    }
  }

  GIVEN("a Vec<float,3> v and a float s") {
    Vec<float,3> v(0.0f, 1.0f, 2.0f);
    const float s = 1.0f;
    WHEN("v is multiplied equaled by s") {
      const auto v_orig(v);
      v *= s;
      THEN("v should be multiplied component wise by s") {
        REQUIRE( v[0] == Approx(v_orig[0] * s) );
        REQUIRE( v[1] == Approx(v_orig[1] * s) );
        REQUIRE( v[2] == Approx(v_orig[2] * s) );
      }
    }
    WHEN("v is divide equaled by s") {
      const auto v_orig(v);
      v /= s;
      THEN("v should be divided component wise by s") {
        REQUIRE( v[0] == Approx(v_orig[0] / s) );
        REQUIRE( v[1] == Approx(v_orig[1] / s) );
        REQUIRE( v[2] == Approx(v_orig[2] / s) );
      }
    }
  }
} // END Vecs can be multiply and divide equaled

/*
SCENARIO( "Vecs can use dot product based operations", "[Vec]" ) {
  GIVEN("two Vec<float,3> v,w") {
    Vec<float,3> v(0.0f, 1.0f, 2.0f);
    const Vec<float,3> w(3.0f, 4.0f, 5.0f);
    WHEN("the dot product of v and w is taken") {
      const float d = dot(v,w);
      THEN("the result should be correct") {
        const float dot = v[0]*w[0] + v[1]*w[1] + v[2]*w[2];
        REQUIRE( d == dot );
      }
    }
    WHEN("the magnitude squared is taken of v") {
      const float ms = magnitude_squared(v);
      THEN("the result should be correct") {
        const float mag_sqr = v[0]*v[0] + v[1]*v[1] + v[2]+v[2];
        REQUIRE ( ms == mag_sqr );
      }
    }
    WHEN("the magnitude is taken of v") {
      const float m = magnitude(v);
      THEN("the result should be correct") {
        const float mag = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]+v[2]);
        REQUIRE ( m == mag );
      }
    }
    WHEN("the inverse magnitude is taken of v") {
      const float i_m = inverse_magnitude(v);
      THEN("the result should be correct") {
        const float inv_mag = 1.0f / sqrt(v[0]*v[0] + v[1]*v[1] + v[2]+v[2]);
        REQUIRE ( i_m == inv_mag );
      }
    }
    WHEN("the normal vector of v is calculated") {
      const Vec<float,3> n = normal(v);
      THEN("the result should be correct") {
        const Vec<float,3> normal = v / sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]+v[2]);
        REQUIRE ( n[0] == normal[0] );
        REQUIRE ( n[1] == normal[1] );
        REQUIRE ( n[2] == normal[2] );
      }
    }
    WHEN("v is normalized") {
      const Vec<float,3> v_orig(v);
      normalize(v);
      THEN("the result should be correct") {
        const Vec<float,3> normalized = v_orig / sqrtf(v_orig[0]*v_orig[0] +
                                                       v_orig[1]*v_orig[1] +
                                                       v_orig[2]+v_orig[2]);
        REQUIRE ( v[0] == normalized[0] );
        REQUIRE ( v[1] == normalized[1] );
        REQUIRE ( v[2] == normalized[2] );
      }
    }
  }
}
*/

SCENARIO( "Vecs have utility math functions", "[Vec]" ) {
  GIVEN("a Vec<float,3> v") {
    const Vec<float,3> v(0.0f, 1.2f, 2.6f);
    WHEN("the floor of v is taken") {
      const Vec<float,3> f = floor(v);
      THEN("floor should be applied component wise") {
        REQUIRE ( f[0] == floorf(v[0]) );
        REQUIRE ( f[1] == floorf(v[1]) );
        REQUIRE ( f[2] == floorf(v[2]) );
      }
    }
    WHEN("the ceil of v is taken") {
      const Vec<float,3> f = ceil(v);
      THEN("ceil should be applied component wise") {
        REQUIRE ( f[0] == ceilf(v[0]) );
        REQUIRE ( f[1] == ceilf(v[1]) );
        REQUIRE ( f[2] == ceilf(v[2]) );
      }
    }
    WHEN("the sum of v is taken") {
      const float s = sum(v);
      THEN("the sum of the components should be returned") {
        REQUIRE ( s == v[0] + v[1] + v[2] );
      }
    }
    WHEN("the product of v is taken") {
      const float p = product(v);
      THEN("the product of the components should be returned") {
        REQUIRE ( p == v[0] * v[1] * v[2] );
      }
    }
  }
  GIVEN("three Vec<float,3> v,v_l,v_u and two floats l,u") {
    Vec<float,3> v(0.0f, 1.2f, 9.6f);
    const Vec<float,3> v_l(-1.1f, 2.5f, 4.6f);
    const Vec<float,3> v_u(0.1f, 2.6f, 5.0f);

    const float l = 1.2f;
    const float u = 3.0f;
    WHEN("the v is clamped between l and u") {
      const Vec<float,3> c_v = clamp(v, l, u);
      THEN("each component should be greater than or equal l") {
        REQUIRE( c_v[0] >= l );
        REQUIRE( c_v[1] >= l );
        REQUIRE( c_v[2] >= l );
      }
      AND_THEN("each component should be less than or equal to u") {
        REQUIRE( c_v[0] <= u );
        REQUIRE( c_v[1] <= u );
        REQUIRE( c_v[2] <= u );
      }
    }
    WHEN("v is clamped in place between v_l and v_u") {
      clamp_in_place(v, v_l, v_u);
      THEN("each component should be greater than the corresponding component in v_l") {
        REQUIRE( v[0] >= v_l[0] );
        REQUIRE( v[1] >= v_l[1] );
        REQUIRE( v[2] >= v_l[2] );
      }
      AND_THEN("each component should be less than or equal to the corresponding component in v_u") {
        REQUIRE( v[0] <= v_u[0] );
        REQUIRE( v[1] <= v_u[1] );
        REQUIRE( v[2] <= v_u[2] );
      }
    }
    WHEN("v is clamped in place between l and u") {
      clamp_in_place(v, l, u);
      THEN("each component should be greater than l") {
        REQUIRE( v[0] >= l );
        REQUIRE( v[1] >= l );
        REQUIRE( v[2] >= l );
      }
      AND_THEN("each component should be less than or equal to u") {
        REQUIRE( v[0] <= u );
        REQUIRE( v[1] <= u );
        REQUIRE( v[2] <= u );
      }
    }
  }
}

SCENARIO("a cross product of two vectors can be calculated") {
  GIVEN("two Vec<float,3> v,w") {
    const Vec<float,3> v(-1.1f, 2.5f, 4.6f);
    const Vec<float,3> w(0.1f, 2.6f, 5.0f);
    WHEN("the cross product of v with respect to w is computed") {
      const Vec<float,3> c = cross(v,w);
      THEN("the result should be correct") {
        Vec<float,3> v_cross_w;
        v_cross_w.x = v[1]*w[2] - v[2]*w[1];
        v_cross_w.y = v[2]*w[0] - v[0]*w[2];
        v_cross_w.z = v[0]*w[1] - v[1]*w[0];
        REQUIRE( c.x == Approx(v_cross_w.x) );
        REQUIRE( c.y == Approx(v_cross_w.y) );
        REQUIRE( c.z == Approx(v_cross_w.z) );

      }
    }
    WHEN("v cross w and w cross v is computed") {
      const Vec<float,3> v_w = cross(v,w);
      const Vec<float,3> w_v = cross(w,v);

      THEN("the result should be negative of eachother") {
        REQUIRE( v_w.x == Approx(-1.0f * w_v.x) );
        REQUIRE( v_w.y == Approx(-1.0f * w_v.y) );
        REQUIRE( v_w.z == Approx(-1.0f * w_v.z) );
      }
    }
  }
}

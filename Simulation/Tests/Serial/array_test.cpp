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
#include "array.h"

SCENARIO("Arrays can be constructed", "[Array]") {
  GIVEN("an Array with capacity of 1,000,000 floats") {
    sim::Array<float> a(1000000);
    WHEN("the capacity is queried") {
      const auto c = a.capacity();
      THEN("it should return 1,000,000") {
        REQUIRE( c == 1000000 );
      }
    }
    AND_WHEN("the size is queried") {
      const auto s = a.size();
      THEN("it should return 0") {
        REQUIRE( s == 0 );
      }
    }
    WHEN("the availability is queried") {
      const auto av = a.available();
      THEN("it should return 1,000,000") {
        REQUIRE( av == 1000000 );
      }
    }
  }
}

SCENARIO("Arrays can have values pushed/popped from them", "[Array]") {
  GIVEN("an Array, a, with capacity of 1,000,000 floats and a pointer, p, to 10 floats") {
    sim::Array<float> a(1000000);
    float p[10] = {9.0,9.0,9.0,9.0,9.0,9.0,9.0,9.0,9.0,9.0};

    WHEN("a 7.0 is pushed to the array") {
      a.push_back(7.0f);
      THEN("the size should increase by one") {
        REQUIRE( a.size() == 1);
      }
      AND_THEN("the element should equal 7.0f") {
        REQUIRE( a[a.size()-1] == 7.0f );
      }
    }

    WHEN("3.0 is pushed back with a count of 10") {
      a.push_back(3.0f, 10);
      THEN("the size should be 10") {
        REQUIRE( a.size() == 10 );
      }
      AND_THEN("each element should equal 3.0f") {
        for(int i=0; i<10; i++) {
          REQUIRE( a[i] == 3.0f );
        }
      }
    }

    WHEN("an a pointer to 10 floats with value 9.0 is pushed back") {
      a.push_back(p, 10);
      THEN("the size should be 10") {
        REQUIRE( a.size() == 10 );
      }
      AND_THEN("each element should equal 9.0f") {
        for(int i=0; i<10; i++) {
          REQUIRE( a[i] == 9.0f );
        }
      }
    }
  }

  GIVEN("an Array, a, with a size of 10") {
    sim::Array<float> a(10);
    a.push_back(1.0, 10);

    WHEN("pop_back is called") {
      a.pop_back();
      THEN("the size should be 9") {
        REQUIRE( a.size() == 9 );
      }
    }

    WHEN("pop_back is called with the argument 8") {
      a.pop_back(8);
      THEN("the size should be 2") {
        REQUIRE( a.size() == 2 );
      }
    }
  }

}

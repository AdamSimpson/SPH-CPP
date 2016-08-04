#include "catch.hpp"
#include "kernels.h"
#include "vec.h"
#include <vector>
#include "builders.h"

SCENARIO("Poly6 Kernels function correctly") {
  GIVEN("a Poly6<float,2>, k with h = 0.05") {
    const float h = 0.05f;
    sim::Poly6<float,2> k(h);
    WHEN("it is evaluated at r_mag = h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h + h*0.01) == Approx(0.0) );
      }
    }
  }

  GIVEN("a Poly6<float,3>, k with h = 0.05") {
    const float h = 0.05f;
    sim::Poly6<float,3> k(h);
    WHEN("it is evaluated at r_mag = h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h + h*0.01) == Approx(0.0) );
      }
    }
  }

  GIVEN("a Poly6<float,2>, k with h = 0.05 and a 1001 x 1001 set of points, p") {
    const float h = 0.05f;
    const sim::Poly6<float,2> k(h);
    const uint64_t dim = 1001;
    const float point_spacing = 2.0*h/(dim-1);
    const auto p = construct_points(dim, dim, point_spacing);
    WHEN("the points are integrated over relative to the center point") {
      // Integrate over all points relative to the center point
      float area = pow(point_spacing, 2.0);
      const uint64_t mid_index = floor(dim*dim/2.0);
      float sum = 0.0;
      for(uint64_t i=0; i<dim*dim; i++) {
        const auto r_mag = magnitude(p[mid_index]- p[i]);
        sum += k(r_mag) * area;
      }
      THEN("then the result is 1.0 as k is normalized") {
        REQUIRE( sum == Approx(1.0f).epsilon(0.001) );
      }
    }
  }

  GIVEN("a Poly6<float,3>, k with h = 0.05 and a 101 x 101 x 101 set of points, p") {
    const float h = 0.05f;
    const sim::Poly6<float,3> k(h);
    const uint64_t dim = 101;
    const float point_spacing = 2.0*h/(dim-1);
    const auto p = construct_points(dim, dim, dim, point_spacing);
    WHEN("the points are integrated over relative to the center point") {
      // Integrate over all points relative to the center point
      float volume = pow(point_spacing, 3.0);
      const uint64_t mid_index = dim*dim*floor(dim/2.0) + floor(dim*dim/2.0);
      float sum = 0.0;
      for(uint64_t i=0; i<dim*dim*dim; i++) {
        const auto r_mag = magnitude(p[mid_index]- p[i]);
        sum += k(r_mag) * volume;
      }
      THEN("then the result is 1.0 as k is normalized") {
        REQUIRE( sum == Approx(1.0f).epsilon(0.001) );
      }
    }
  }
}

SCENARIO("Del_Poly6 Kernels function correctly") {
  GIVEN("a Del_Poly6<float,2>, k with h = 0.05") {
    const float h = 0.05f;
    sim::Del_Poly6<float,2> k(h);
    WHEN("it is evaluated at r_mag = h") {
      Vec<float,2> p{0.0};
      Vec<float,2> q{h/sqrt(2.0), h/sqrt(2.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      Vec<float,2> p{0.0};
      Vec<float,2> q{h/sqrt(2.0) + 0.001, h/sqrt(2.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag == 0.0") {
      Vec<float,2> p{h/sqrt(2.0) + 0.001, h/sqrt(2.0)};
      Vec<float,2> q{h/sqrt(2.0) + 0.001, h/sqrt(2.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
  }

  GIVEN("a Del_Poly6<float,3>, k with h = 0.05") {
    const float h = 0.05f;
    sim::Del_Poly6<float,3> k(h);
    WHEN("it is evaluated at r_mag = h") {
      Vec<float,3> p{0.0};
      Vec<float,3> q{h/sqrt(3.0), h/sqrt(3.0), h/sqrt(3.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      Vec<float,3> p{0.0};
      Vec<float,3> q{h/sqrt(3.0) + 0.001, h/sqrt(3.0), h/sqrt(3.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag == 0.0") {
      Vec<float,3> p{h/sqrt(2.0) + 0.001, h/sqrt(2.0), h};
      Vec<float,3> q{h/sqrt(2.0) + 0.001, h/sqrt(2.0), h};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
  }
}

SCENARIO("Del_Spikey Kernels function correctly") {
  GIVEN("a Del_Spikey<float,2>, k with h = 0.05") {
    const float h = 0.05f;
    sim::Del_Poly6<float,2> k(h);
    WHEN("it is evaluated at r_mag = h") {
      Vec<float,2> p{0.0};
      Vec<float,2> q{h/sqrt(2.0), h/sqrt(2.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      Vec<float,2> p{0.0};
      Vec<float,2> q{h/sqrt(2.0) + 0.001, h/sqrt(2.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
  }

  GIVEN("a Del_Spikey<float,3>, k with h = 0.05") {
    const float h = 0.05f;
    sim::Del_Spikey<float,3> k(h);
    WHEN("it is evaluated at r_mag = h") {
      Vec<float,3> p{0.0};
      Vec<float,3> q{h/sqrt(3.0), h/sqrt(3.0), h/sqrt(3.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      Vec<float,3> p{0.0};
      Vec<float,3> q{h/sqrt(3.0) + 0.001, h/sqrt(3.0), h/sqrt(3.0)};
      const auto result = k(p, q);
      THEN("the result should be 0.0") {
        REQUIRE( magnitude(result) == Approx(0.0) );
      }
    }
  }
}

SCENARIO("C_Spline Kernels function correctly") {
  GIVEN("a C_Spline<float,2>, k with h = 0.05") {
    const float h = 0.05f;
    sim::C_Spline<float,2> k(h);
    WHEN("it is evaluated at r_mag = h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h + h*0.01) == Approx(0.0) );
      }
    }
  }

  GIVEN("a C_Spline<float,3>, k with h = 0.05") {
    const float h = 0.05f;
    sim::C_Spline<float,3> k(h);
    WHEN("it is evaluated at r_mag = h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h) == Approx(0.0) );
      }
    }
    WHEN("it's evaluated at r_mag > h") {
      THEN("the result should be 0.0") {
        REQUIRE( k(h + h*0.01) == Approx(0.0) );
      }
    }
  }
}

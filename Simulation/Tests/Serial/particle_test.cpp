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
#include "parameters.h"
#include "particles.h"

SCENARIO("Particles can be created") {
  GIVEN("Particles<float,2> particles constructed from particle_test.ini") {
    sim::Parameters<float,2> params{"particle_test.ini"};
    sim::Particles<float, 2> particles{params};

    WHEN("When the local_count is queried") {
      const auto count = particles.local_count();
      THEN("the count should equal 0") {
        REQUIRE( count == 0 );
      }
    }

    WHEN("When the max_count_local is queried") {
      const auto count = particles.max_local_count();
      THEN("the count should equal 100000") {
        REQUIRE( count == 100000 );
      }
    }

    WHEN("When the available count is queried") {
      const auto count = particles.available();
      THEN("the count should equal max_count - local_count(100000 - 0)") {
        REQUIRE( count == 100000 - 0 );
      }
    }
  }

  GIVEN("Particles<float,3> particles constructed from particle_test.ini") {
    sim::Parameters<float,3> params{"particle_test.ini"};
    sim::Particles<float, 3> particles{params};

    WHEN("When the local_count is queried") {
      const auto count = particles.local_count();
      THEN("the count should equal 0") {
        REQUIRE( count == 0 );
      }
    }

    WHEN("When the max_count_local is queried") {
      const auto count = particles.max_local_count();
      THEN("the count should equal 100000") {
        REQUIRE( count == 100000 );
      }
    }

    WHEN("When the available count is queried") {
      const auto count = particles.available();
      THEN("the count should equal max_count - local_count(100000 - 0)") {
        REQUIRE( count == 100000 - 0 );
      }
    }
  }

}

SCENARIO("Particles can be added to the system") {
  GIVEN("Particles<float,2> particles constructed from particle_test.ini") {
    sim::Parameters<float,2> params{"particle_test.ini"};
    sim::Particles<float, 2> particles{params};

    WHEN("the initial fluid is constructed") {
      particles.construct_fluid(params.initial_fluid());
      THEN("the local_count should equal 28900") {
        REQUIRE( particles.local_count() == 28900 );
      }
      AND_THEN("the max_count_local should equal 100000") {
        REQUIRE( particles.max_local_count() == 100000 );
      }
      AND_THEN("the available count should equal 100000 - 28900") {
        REQUIRE( particles.available() == 100000-28900 );
      }
    }
  }

  GIVEN("Particles<float,3> particles constructed from particle_test.ini") {
    sim::Parameters<float,3> params{"particle_test.ini"};
    sim::Particles<float, 3> particles{params};

    WHEN("the initial fluid is constructed") {
      particles.construct_fluid(params.initial_fluid());
      THEN("the local_count should equal 27000") {
        REQUIRE( particles.local_count() == 27000 );
      }
      AND_THEN("the max_count_local should equal 100000") {
        REQUIRE( particles.max_local_count() == 100000 );
      }
      AND_THEN("the available count should equal 100000 - 27000") {
        REQUIRE( particles.available() == 100000-27000 );
      }
    }
  }
}

SCENARIO("external forces can be applied") {
  GIVEN("Particles<float,2> particles constructed from particle_test.ini") {
    sim::Parameters<float, 2> params{"particle_test.ini"};
    sim::Particles<float, 2> particles{params};
    particles.construct_fluid(params.initial_fluid());

    WHEN("external forces are applied") {
      IndexSpan span{0, particles.local_count()};
      const auto previous_velocities{particles.velocities()};
      particles.apply_external_forces(span);
      THEN("The velocity should increase by g*dt") {
        for(int i=0; i<particles.local_count(); i++) {
          REQUIRE( particles.velocities()[i].x == Approx(previous_velocities[i].x) );
          REQUIRE( particles.velocities()[i].y == Approx(previous_velocities[i].y
                                                         + params.gravity() * params.time_step() ));
        }
      }
    }
  }

  GIVEN("Particles<float,3> particles constructed from particle_test.ini") {
    sim::Parameters<float, 3> params{"particle_test.ini"};
    sim::Particles<float, 3> particles{params};
    particles.construct_fluid(params.initial_fluid());

    WHEN("external forces are applied") {
      IndexSpan span{0, particles.local_count()};
      const auto previous_velocities{particles.velocities()};
      particles.apply_external_forces(span);
      THEN("The velocity should increase by g*dt") {
        for(int i=0; i<particles.local_count(); i++) {
          REQUIRE( particles.velocities()[i].x == Approx(previous_velocities[i].x) );
          REQUIRE( particles.velocities()[i].y == Approx(previous_velocities[i].y
                                                         + params.gravity() * params.time_step() ));
          REQUIRE( particles.velocities()[i].z == Approx(previous_velocities[i].z) );
        }
      }
    }
  }
}

SCENARIO("positions can be predicted") {
  GIVEN("Particles<float,3> particles constructed from particle_test.ini") {
    sim::Parameters<float, 3> params{"particle_test.ini"};
    sim::Particles<float, 3> particles{params};
    particles.construct_fluid(params.initial_fluid());

    WHEN("velocities are set to (1.0, -1.0, 1.3) and positions are predicted") {
      for(int i=0; i<particles.local_count(); i++) {
        particles.velocities()[i].x = 1.0;
        particles.velocities()[i].y = -1.0;
        particles.velocities()[i].z = 1.3;
      }

      IndexSpan span{0, particles.local_count()};
      particles.predict_positions(span);

      THEN("The position should increase by (-)1.0 * dt") {
        for(int i=0; i<particles.local_count(); i++) {
          REQUIRE( particles.position_stars()[i].x == Approx(particles.positions()[i].x
                                                             + 1.0 * params.time_step() ));
          REQUIRE( particles.position_stars()[i].y == Approx(particles.positions()[i].y
                                                             - 1.0 * params.time_step() ));
          REQUIRE( particles.position_stars()[i].z == Approx(particles.positions()[i].z
                                                             + 1.3 * params.time_step() ));
        }
      }
    }

    WHEN("A particle is predicted out of bounds") {
      particles.positions()[0].x = 0.0;
      particles.positions()[0].y = 0.0;
      particles.velocities()[0].x = -1.0;
      particles.velocities()[0].y = -1.0;

      IndexSpan span{0, 1};
      particles.predict_positions(span);

      THEN("The particle should not go out of bounds") {
        REQUIRE( particles.positions()[0].x == Approx(params.boundary().min.x ) );
        REQUIRE( particles.positions()[0].y == Approx(params.boundary().min.y ) );
      }
    }

    WHEN("A particle is predicted out of bounds") {
      particles.positions()[0].x = 0.0;
      particles.positions()[0].y = 0.0;
      particles.positions()[0].z = 0.0;
      particles.velocities()[0].x = -1.0;
      particles.velocities()[0].y = -2.0;
      particles.velocities()[0].z = -3.0;

      IndexSpan span{0, 1};
      particles.predict_positions(span);

      THEN("The particle should not go out of bounds") {
        REQUIRE( particles.positions()[0].x == Approx(params.boundary().min.x ) );
        REQUIRE( particles.positions()[0].y == Approx(params.boundary().min.y ) );
        REQUIRE( particles.positions()[0].z == Approx(params.boundary().min.z ) );
      }
    }

  }
}

SCENARIO("densities can be computed") {
  GIVEN("Particles<float,2> particles constructed from particle_test.ini") {
    sim::Parameters<float, 2> params{"particle_test.ini"};
    sim::Particles<float, 2> particles{params};
    particles.construct_fluid(params.initial_fluid());
    IndexSpan span{0, particles.local_count()};
    particles.find_neighbors(span, span);
    particles.compute_densities(span);

    WHEN("the average density is calculated") {
      float avg_density = 0.0;
      for(const auto density : particles.densities()) {
        avg_density += density;
      }
      avg_density /= particles.local_count();
      THEN("the average density should be within 5% of the rest density") {
        REQUIRE(std::abs(avg_density - params.rest_density()) < params.rest_density()/20.0);
      }
    }
  }

  GIVEN("Particles<float,3> particles constructed from particle_test.ini") {
    sim::Parameters<float, 3> params{"particle_test.ini"};
    sim::Particles<float, 3> particles{params};
    particles.construct_fluid(params.initial_fluid());
    IndexSpan span{0, particles.local_count()};
    particles.find_neighbors(span, span);
    particles.compute_densities(span);

    WHEN("the average density is calculated") {
      float avg_density = 0.0;
      for(const auto density : particles.densities()) {
        avg_density += density;
      }
      avg_density /= particles.local_count();
      THEN("the average density should be within 5% of the rest density") {
        REQUIRE(std::abs(avg_density - params.rest_density()) < params.rest_density()/20.0);
      }
    }
  }
}

SCENARIO("pressure lambdas can be computed") {
}


SCENARIO("pressure deltas can be computed") {
}


SCENARIO("position stars can be updated") {
}

SCENARIO("velocities can be updated") {
}


SCENARIO("positions can be updated") {
}

SCENARIO("surface tension can be applied") {
}

SCENARIO("viscosity can be applied") {
}

SCENARIO("vorticity can be computed") {
}

SCENARIO("vorticity can be applied") {
}
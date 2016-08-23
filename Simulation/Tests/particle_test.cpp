#include "catch.hpp"
#include "parameters.h"
#include "particles.h"

SCENARIO("Particles can be created") {
  GIVEN("Particles<float,2> particles constructed from particle_test.ini") {
    auto params  = new sim::Parameters<float, three_dimensional>;
    auto particles = new sim::Particles<float, three_dimensional>{*params};
    WHEN("Some stuff happens") {
      THEN("Some good stuff comes out") {
      }
    }
  }
}
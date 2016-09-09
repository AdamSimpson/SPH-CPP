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
#include "distributor.h"

// Create 12 x 4( x 1) particle initial fluid
// Smoothing radius is set to the particle rest spacing to simplify thing

SCENARIO("A distributor can be constructed") {
  GIVEN("an initilized distibutor<float,2> with 3 processes") {
    sim::Distributor<float, 2> d{false};
    sim::Parameters<float, 2> params{"distributor_test.ini"};
    sim::Particles<float, 2> particles{params};
    WHEN("comm_compute_ is queried for basic information") {
      THEN("comm_compute knows about its neighbors") {
        REQUIRE(d.comm_compute_.size() == 3);
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.is_first_domain());
          REQUIRE(d.domain_to_left() == MPI_PROC_NULL);
          REQUIRE(d.domain_to_right() == 1);
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(d.domain_to_left() == 0);
          REQUIRE(d.domain_to_right() == 2);
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(d.is_last_domain());
          REQUIRE(d.domain_to_left() == 1);
          REQUIRE(d.domain_to_right() == MPI_PROC_NULL);
        }
      }
    }
  }

  GIVEN("an initilized distibutor<float,3> with 3 processes") {
    sim::Distributor<float, 3> d{false};
    sim::Parameters<float, 3> params{"distributor_test.ini"};
    sim::Particles<float, 3> particles{params};
    WHEN("comm_compute_ is queried for basic information") {
      THEN("comm_compute knows about its neighbors") {
        REQUIRE(d.comm_compute_.size() == 3);
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.is_first_domain());
          REQUIRE(d.domain_to_left() == MPI_PROC_NULL);
          REQUIRE(d.domain_to_right() == 1);
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(d.domain_to_left() == 0);
          REQUIRE(d.domain_to_right() == 2);
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(d.is_last_domain());
          REQUIRE(d.domain_to_left() == 1);
          REQUIRE(d.domain_to_right() == MPI_PROC_NULL);
        }
      }
    }
  }
}

SCENARIO("A distributor can initialize fluid") {
  GIVEN("an initialized distibutor<float,2> with 3 processes") {
    sim::Distributor<float, 2> d{false};
    sim::Parameters<float, 2> params{"distributor_test.ini"};
    sim::Particles<float, 2> particles{params};
    WHEN("the distributor initlizes the fluid") {
      d.initilize_fluid(particles, params);
      THEN("the domain bounds are correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.domain_.begin == Approx(0.0f));
          REQUIRE(d.domain_.end == Approx(3.0f));
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(d.domain_.begin == Approx(3.0f));
          REQUIRE(d.domain_.end == Approx(6.0f));
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(d.domain_.begin == Approx(6.0f));
          REQUIRE(d.domain_.end == Approx(11.0f));
        }
      }
    }
  }

  GIVEN("an initialized distibutor<float,3> with 3 processes") {
    sim::Distributor<float, 3> d{false};
    sim::Parameters<float, 3> params{"distributor_test.ini"};
    sim::Particles<float, 3> particles{params};
    WHEN("the distributor initializes the fluid") {
      d.initilize_fluid(particles, params);
      THEN("the domain bounds are correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.domain_.begin == Approx(0.0f));
          REQUIRE(d.domain_.end == Approx(3.0f));
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(d.domain_.begin == Approx(3.0f));
          REQUIRE(d.domain_.end == Approx(6.0f));
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(d.domain_.begin == Approx(6.0f));
          REQUIRE(d.domain_.end == Approx(11.0f));
        }
      }
    }
  }

  GIVEN("an initialized distibutor<float,2> with 3 processes") {
    sim::Distributor<float, 2> d{false};
    sim::Parameters<float, 2> params{"distributor_test.ini"};
    sim::Particles<float, 2> particles{params};
    WHEN("the distributor initlizes the fluid") {
      d.initilize_fluid(particles, params);
      THEN("the particle count should be correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(particles.local_count() == 12);
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(particles.local_count() == 12);
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(particles.local_count() == 12);
        }
      }
    }
  }

  GIVEN("an initialized distributor<float,3> with 3 processes") {
    sim::Distributor<float, 3> d{false};
    sim::Parameters<float, 3> params{"distributor_test.ini"};
    sim::Particles<float, 3> particles{params};
    WHEN("the distributor initializes the fluid") {
      d.initilize_fluid(particles, params);
      THEN("the particle count should be correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(particles.local_count() == 12);
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(particles.local_count() == 12);
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(particles.local_count() == 12);
        }
      }
    }
  }
}

SCENARIO("When halos are exchanged spans and counts are computed correct") {
  GIVEN("an initialized distributor<float,3> with 3 processes") {
    sim::Distributor<float, 3> d{false};
    sim::Parameters<float, 3> params{"distributor_test.ini"};
    sim::Particles<float, 3> particles{params};
    d.initilize_fluid(particles, params);
    WHEN("the domains are synced so edge and halo particles are known") {
      d.invalidate_halo(particles);
      d.domain_sync(particles);

      THEN("the local_span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 20);
          REQUIRE(d.local_count() == 20);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
      }

      AND_THEN("the resident count and span is correct") {
        REQUIRE(d.resident_span().begin == 0);
        REQUIRE(d.resident_span().end == 12);
        REQUIRE(d.resident_count() == 12);
      }

      AND_THEN("the edge count and span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.edge_count() == 8);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 12);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.edge_count() == 8);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 12);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.edge_count() == 4);
          REQUIRE(d.edge_span().begin == 8);
          REQUIRE(d.edge_span().end == 12);
        }
      }

      AND_THEN("the halo span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.halo_count() == 4);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.halo_count() == 8);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 20);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.halo_count() == 4);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 16);
        }
      }

      AND_THEN("the interior span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 8);
          REQUIRE(d.interior_count() == 8);
        }
      }
    }
  }

  GIVEN("an initialized distributor<float,2> with 3 processes") {
    sim::Distributor<float, 2> d{false};
    sim::Parameters<float, 2> params{"distributor_test.ini"};
    sim::Particles<float, 2> particles{params};
    d.initilize_fluid(particles, params);
    WHEN("the domains are synced so edge and halo particles are known") {
      d.invalidate_halo(particles);
      d.domain_sync(particles);

      THEN("the local_span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 20);
          REQUIRE(d.local_count() == 20);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
      }

      AND_THEN("the resident count and span is correct") {
        REQUIRE(d.resident_span().begin == 0);
        REQUIRE(d.resident_span().end == 12);
        REQUIRE(d.resident_count() == 12);
      }

      AND_THEN("the edge count and span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.edge_count() == 8);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 12);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.edge_count() == 8);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 12);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.edge_count() == 4);
          REQUIRE(d.edge_span().begin == 8);
          REQUIRE(d.edge_span().end == 12);
        }
      }

      AND_THEN("the halo span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.halo_count() == 4);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.halo_count() == 8);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 20);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.halo_count() == 4);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 16);
        }
      }

      AND_THEN("the interior span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 8);
          REQUIRE(d.interior_count() == 8);
        }
      }
    }
  }
}

SCENARIO("When out of bound particles are exchanged spans and counts are computed correct") {
  GIVEN("an initialized distributor<float,3> with 3 processes") {
    sim::Distributor<float, 3> d{false};
    sim::Parameters<float, 3> params{"distributor_test.ini"};
    sim::Particles<float, 3> particles{params};
    d.initilize_fluid(particles, params);
    WHEN("the a left most particle is moved from rank 0 to rank 1 and domains are synced") {
      if(d.comm_compute_.rank() == 0) {
        particles.position_stars()[0].x = 3.1;
      }
      d.invalidate_halo(particles);
      d.domain_sync(particles);

      THEN("the local_span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 21);
          REQUIRE(d.local_count() == 21);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
      }

      AND_THEN("the resident count and span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.resident_span().begin == 0);
          REQUIRE(d.resident_span().end == 11);
          REQUIRE(d.resident_count() == 11);
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(d.resident_span().begin == 0);
          REQUIRE(d.resident_span().end == 13);
          REQUIRE(d.resident_count() == 13);
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(d.resident_span().begin == 0);
          REQUIRE(d.resident_span().end == 12);
          REQUIRE(d.resident_count() == 12);
        }
      }

      AND_THEN("the edge count and span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.edge_count() == 7);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 11);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.edge_count() == 9);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 13);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.edge_count() == 4);
          REQUIRE(d.edge_span().begin == 8);
          REQUIRE(d.edge_span().end == 12);
        }
      }

      AND_THEN("the halo span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.halo_count() == 5);
          REQUIRE(d.halo_span().begin == 11);
          REQUIRE(d.halo_span().end == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.halo_count() == 8);
          REQUIRE(d.halo_span().begin == 13);
          REQUIRE(d.halo_span().end == 21);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.halo_count() == 4);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 16);
        }
      }

      AND_THEN("the interior span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 8);
          REQUIRE(d.interior_count() == 8);
        }
      }
    }
  }

  GIVEN("an initialized distributor<float,2> with 2 processes") {
    sim::Distributor<float, 2> d{false};
    sim::Parameters<float, 2> params{"distributor_test.ini"};
    sim::Particles<float, 2> particles{params};
    d.initilize_fluid(particles, params);
    WHEN("the a left most particle is moved from rank 0 to rank 1 and domains are synced") {
      if(d.comm_compute_.rank() == 0) {
        particles.position_stars()[0].x = 3.1;
      }
      d.invalidate_halo(particles);
      d.domain_sync(particles);

      THEN("the local_span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 21);
          REQUIRE(d.local_count() == 21);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.local_span().begin == 0);
          REQUIRE(d.local_span().end == 16);
          REQUIRE(d.local_count() == 16);
        }
      }

      AND_THEN("the resident count and span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.resident_span().begin == 0);
          REQUIRE(d.resident_span().end == 11);
          REQUIRE(d.resident_count() == 11);
        }
        if (d.comm_compute_.rank() == 1) {
          REQUIRE(d.resident_span().begin == 0);
          REQUIRE(d.resident_span().end == 13);
          REQUIRE(d.resident_count() == 13);
        }
        if (d.comm_compute_.rank() == 2) {
          REQUIRE(d.resident_span().begin == 0);
          REQUIRE(d.resident_span().end == 12);
          REQUIRE(d.resident_count() == 12);
        }
      }

      AND_THEN("the edge count and span is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.edge_count() == 7);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 11);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.edge_count() == 9);
          REQUIRE(d.edge_span().begin == 4);
          REQUIRE(d.edge_span().end == 13);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.edge_count() == 4);
          REQUIRE(d.edge_span().begin == 8);
          REQUIRE(d.edge_span().end == 12);
        }
      }

      AND_THEN("the halo span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.halo_count() == 5);
          REQUIRE(d.halo_span().begin == 11);
          REQUIRE(d.halo_span().end == 16);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.halo_count() == 8);
          REQUIRE(d.halo_span().begin == 13);
          REQUIRE(d.halo_span().end == 21);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.halo_count() == 4);
          REQUIRE(d.halo_span().begin == 12);
          REQUIRE(d.halo_span().end == 16);
        }
      }

      AND_THEN("the interior span and count is correct") {
        if (d.comm_compute_.rank() == 0) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 1) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 4);
          REQUIRE(d.interior_count() == 4);
        }
        if(d.comm_compute_.rank() == 2) {
          REQUIRE(d.interior_span().begin == 0);
          REQUIRE(d.interior_span().end == 8);
          REQUIRE(d.interior_count() == 8);
        }
      }
    }
  }
}
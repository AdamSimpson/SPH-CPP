#include "catch.hpp"
#include "parameters.h"
#include "neighbors.h"
#include "builders.h"

// Drawing a picture works very well here

SCENARIO("Neighbors are constructed correctly") {
  GIVEN("An input file ../Tests/neighbor_test.ini specifying a 5.0 x 5.0 \
           boundary with 1.0 neighbor bin spacing") {
    sim::Parameters<float,2> p{"../Tests/neighbor_test.ini"};
    sim::Neighbors<float,2> n{p};
    WHEN("The neighbor grid size is checked") {
      THEN("The bin dimensions should be 7x7") {
        REQUIRE(n.bin_dimensions().x == 7);
        REQUIRE(n.bin_dimensions().y == 7);
      }
    }
  }
}

SCENARIO("Bin id's are correctly calculated") {
  GIVEN("A neighbor grid of 5x5(without padding)") {
    sim::Parameters<float,2> p{"../Tests/neighbor_test.ini"};
    sim::Neighbors<float,2> n{p};

    WHEN("The bin id for point (-1.0, -1.0) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,2>{-1.0});
      THEN("The bin id is 0") {
        REQUIRE(bid == 0);
      }
    }

    WHEN("The bin id for point (0.0, 0.0) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,2>{0.0});
      THEN("The bin id is 8") {
        REQUIRE(bid == 8);
      }
    }

    WHEN("The bin id for point (1.0, 1.0) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,2>{1.0});
      THEN("The bin id is 16") {
        REQUIRE(bid == 16);
      }
    }

    WHEN("The bin id for point (2.5, 2.5) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,2>{2.5});
      THEN("The bin id is 24") {
        REQUIRE(bid == 24);
      }
    }

    WHEN("The bin id for point (4.999, 4.999) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,2>{4.999});
      THEN("The bin id is 40") {
        REQUIRE(bid == 40);
      }
    }

  }

  GIVEN("A neighbor grid of 5x5x5 (7x7x7 with padding)") {
    sim::Parameters<float,3> p{"../Tests/neighbor_test.ini"};
    sim::Neighbors<float,3> n{p};

    WHEN("The bin id for point (-1.0, -1.0, -1.0) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,3>{-1.0});
      THEN("The bin id is 0") {
        REQUIRE(bid == 0);
      }
    }

    WHEN("The bin id for point (0.0, 0.0, 0.0) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,3>{0.0});
      THEN("The bin id is 49+9-1") {
        REQUIRE(bid == 49+9-1);
      }
    }

    WHEN("The bin id for point (1.0, 1.0, 1.0) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,3>{1.0});
      THEN("The bin id is 49+49+17-1") {
        REQUIRE(bid == 49+49+17-1);
      }
    }

    WHEN("The bin id for point (4.999, 4.999, 4.999) is calculated") {
      const auto bid = n.calculate_bin_id(Vec<float,3>{4.999});
      THEN("The bin id is 49*7-1") {
        REQUIRE(bid == 49*6-8-1);
      }
    }

  }
}

// Each bin will be filled with 4 particles equally spaced
SCENARIO("Neighbors are correctly calculated") {
  GIVEN("An input file ../Tests/neighbor_test.ini specifying a 5.0 x 5.0 \
           boundary with 1.0 neighbor bin spacing") {
    sim::Parameters<float,2> p{"../Tests/neighbor_test.ini"};
    sim::Neighbors<float,2> n{p};
    WHEN("particles are placed in the bins such that their rest spacing is half of the bin spacing") {
      const auto particles = construct_points(10, 10, 0.5);
      IndexSpan span{0, 100};
      n.find(span, span, particles.data());

      THEN("All interior particles should have 8 neighbors") {
        for(int j=1; j<9; j++) {
          for(int i=1; i<9; i++) {
            const std::size_t index = j*10 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 8);
          }
        }
      }

      THEN("All edge particles, besides the 4 corners, should have 5 neighbors") {
        // Left column
        for(int j=1; j<9; j++) {
            const std::size_t index = j*10 + 0;
            REQUIRE(end(n[index]) - begin(n[index]) == 5);
        }
        // Right column
        for(int j=1; j<9; j++) {
            const std::size_t index = j*10 + 9;
            REQUIRE(end(n[index]) - begin(n[index]) == 5);
        }
        // Bottom row
        for(int i=1; i<9; i++) {
            const std::size_t index = i;
            REQUIRE(end(n[index]) - begin(n[index]) == 5);
        }
        // Top row
        for(int i=1; i<9; i++) {
            const std::size_t index = 9*10 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 5);
        }
      }

      THEN("All corner particles should have 3 neighbors") {
        REQUIRE(end(n[0])  - begin(n[0]) == 3);
        REQUIRE(end(n[99]) - begin(n[99]) == 3);
        REQUIRE(end(n[9])  - begin(n[9]) == 3);
        REQUIRE(end(n[90]) - begin(n[90]) == 3);
      }

    }
  }

  GIVEN("An input file ../Tests/neighbor_test.ini specifying a 5.0 x 5.0 x 5.0 \
           boundary with 1.0 neighbor bin spacing") {
    sim::Parameters<float,3> p{"../Tests/neighbor_test.ini"};
    sim::Neighbors<float,3> n{p};
    WHEN("particles are placed in the bins such that their rest spacing is half of the bin spacing") {
      const auto particles = construct_points(10, 10, 10, 0.5);
      IndexSpan span{0, 1000};
      n.find(span, span, particles.data());

      THEN("All interior particles should have 26 neighbors") {
        for(int k=1; k<9; k++) {
          for(int j=1; j<9; j++) {
            for(int i=1; i<9; i++) {
              const std::size_t index = k*100 + j*10 + i;
              REQUIRE(end(n[index]) - begin(n[index]) == 26);
            }
          }
        }
      }

      THEN("All edge row/columns, besides the 8 corners, should have 13 neighbors") {
        // Left front column
        for(int j=1; j<9; j++) {
            const std::size_t index = j*10 + 0;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
        // Right front column
        for(int j=1; j<9; j++) {
            const std::size_t index = j*10 + 9;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
        // Left back column
        for(int j=1; j<9; j++) {
            const std::size_t index = 9*100 + j*10 + 0;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
        // Right back column
        for(int j=1; j<9; j++) {
            const std::size_t index =  9*100 + j*10 + 9;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
        // Bottom front row
        for(int i=1; i<9; i++) {
            const std::size_t index = i;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
        // Top front row
        for(int i=1; i<9; i++) {
            const std::size_t index = 9*10 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
        // Bottom back row
        for(int i=1; i<9; i++) {
            const std::size_t index = 9*100 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
        // Top back row
        for(int i=1; i<9; i++) {
            const std::size_t index = 9*100 + 9*10 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 11);
        }
      }

      THEN("All faces, besides the edge row/columns particles, should have 17 neighbors") {
        // Front face
        for(int j=1; j<9; j++) {
          for(int i=1; i<9; i++) {
            const std::size_t index = j*10 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 17);
          }
        }
        // Back face
        for(int j=1; j<9; j++) {
          for(int i=1; i<9; i++) {
            const std::size_t index = 9*100 + j*10 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 17);
          }
        }
        // Top face
        for(int k=1; k<9; k++) {
          for(int i=1; i<9; i++) {
            const std::size_t index = k*100 + 9*10 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 17);
          }
        }
        // Bottom face
        for(int k=1; k<9; k++) {
          for(int i=1; i<9; i++) {
            const std::size_t index = k*100 + i;
            REQUIRE(end(n[index]) - begin(n[index]) == 17);
          }
        }
        // Left face
        for(int k=1; k<9; k++) {
          for(int j=1; j<9; j++) {
            const std::size_t index = k*100 + j*10;
            REQUIRE(end(n[index]) - begin(n[index]) == 17);
          }
        }
        // Right face
        for(int k=1; k<9; k++) {
          for(int j=1; j<9; j++) {
            const std::size_t index = k*100 + j*10 + 9;
            REQUIRE(end(n[index]) - begin(n[index]) == 17);
          }
        }
      }

      THEN("All corner particles should have 7 neighbors") {
        // Front corners
        REQUIRE(end(n[0])  - begin(n[0]) == 7);
        REQUIRE(end(n[99]) - begin(n[99]) == 7);
        REQUIRE(end(n[9])  - begin(n[9]) == 7);
        REQUIRE(end(n[90]) - begin(n[90]) == 7);
        // Back corners
        REQUIRE(end(n[900])  - begin(n[900]) == 7);
        REQUIRE(end(n[999]) - begin(n[999]) == 7);
        REQUIRE(end(n[909])  - begin(n[909]) == 7);
        REQUIRE(end(n[990]) - begin(n[990]) == 7);
      }

    }
  }
}

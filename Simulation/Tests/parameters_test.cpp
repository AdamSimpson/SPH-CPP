#include "catch.hpp"
#include "parameters.h"
#include "execution_mode.h"

SCENARIO("Parameters can be created") {
  GIVEN("params_test.ini") {
    WHEN("parameters<float,3>, p, are initialized with the file") {
      sim::Parameters<float,3> p{"../Tests/params_test.ini"};
      THEN("then p should be correctly initialized") {
        REQUIRE( p.max_particles_local_ == 100000 );
        REQUIRE( p.initial_global_particle_count_ == 30000 );
        REQUIRE( p.solve_step_count_ == 4 );

        REQUIRE( p.particle_rest_spacing_ == Approx(0.03218f) );
        REQUIRE( p.particle_radius_ == Approx(0.5f * p.particle_rest_spacing_) );
        REQUIRE( p.smoothing_radius_ == Approx(1.8f * p.particle_rest_spacing_) );
        REQUIRE( p.neighbor_bin_spacing_ == Approx(1.2f * p.smoothing_radius_ ) );
        REQUIRE( p.rest_density_  == Approx(29999.99609f) );
        REQUIRE( p.rest_mass_ == Approx(1.0f) );
        REQUIRE( p.gravity_  == Approx(-9.8f) );
        REQUIRE( p.gamma_ == Approx(600.0f) );
        REQUIRE( p.lambda_epsilon_ == Approx(10.0f) );
        REQUIRE( p.k_stiff_ == Approx(1.0f) );
        REQUIRE( p.visc_c_ == Approx(0.03f) );
        REQUIRE( p.time_step_ == Approx(0.008f) );
        REQUIRE( p.max_speed_ == Approx(14.48234f) );
        REQUIRE( p.vorticity_coef_ == Approx(0.00001f) );

        REQUIRE( p.boundary_.min.x == Approx(0.0f) );
        REQUIRE( p.boundary_.min.y == Approx(0.0f) );
        REQUIRE( p.boundary_.min.z == Approx(0.0f) );
        REQUIRE( p.boundary_.max.x == Approx(1.5f) );
        REQUIRE( p.boundary_.max.y == Approx(2.0f) );
        REQUIRE( p.boundary_.max.z == Approx(1.5f) );

        REQUIRE( p.initial_fluid_.min.x == Approx(0.0f) );
        REQUIRE( p.initial_fluid_.min.y == Approx(0.0f) );
        REQUIRE( p.initial_fluid_.min.z == Approx(0.0f) );
        REQUIRE( p.initial_fluid_.max.x == Approx(1.0f) );
        REQUIRE( p.initial_fluid_.max.y == Approx(1.0f) );
        REQUIRE( p.initial_fluid_.max.z == Approx(1.0f) );

        bool sim_mode_equal_edit_view = p.simulation_mode_ == sim::Parameters<float,3>::Mode::EDIT_VIEW;
        REQUIRE( sim_mode_equal_edit_view );
        REQUIRE( p.execution_mode_ == sim::ExecutionMode::GPU );

        REQUIRE( p.emitter_center_.x == Approx(p.boundary_.center().x) );
        REQUIRE( p.emitter_center_.y == Approx(p.boundary_.center().y) );
        REQUIRE( p.emitter_center_.z == Approx(p.boundary_.center().z) );

        REQUIRE( p.emitter_velocity_.x == Approx(0.0f) );
        REQUIRE( p.emitter_velocity_.y == Approx(0.0f) );
        REQUIRE( p.emitter_velocity_.z == Approx(0.0f) );

        REQUIRE( p.mover_center_.x == Approx(-1.5f) );
        REQUIRE( p.mover_center_.y == Approx(-2.0f) );
        REQUIRE( p.mover_center_.z == Approx(-1.5f) );
      }
    }
  }
}

SCENARIO("The simulation_mode can be modified") {
  GIVEN("parameters<float,3>, p, are intialized with params_test.ini") {
    sim::Parameters<float,3> p{"../Tests/params_test.ini"};

    WHEN("The parameters are initialized") {
      THEN("the simulation should be active") {
        REQUIRE( p.simulation_active() == true );
      }
      AND_THEN("the simulation should be in view edit mode") {
        REQUIRE( p.edit_view() == true );
      }
      AND_THEN("the compute mode should be active") {
        REQUIRE( p.compute_active() == true );
      }
    }

    WHEN("exit_simulation is called") {
      p.exit_simulation();
      THEN("the simulation should not be active") {
        REQUIRE( p.simulation_active() == false );
      }
    }

    WHEN("the compute is paused") {
      p.pause_compute();
      THEN("the simulation should not be active") {
        REQUIRE( p.compute_active() == false );
      }
    }
    AND_WHEN("the compute is activated") {
      p.activate_compute();
      THEN("the simulation should be active") {
        REQUIRE( p.compute_active() == true );
      }
    }
    AND_WHEN("the compute is paused and then toggled") {
      p.pause_compute();
      p.toggle_compute_paused();
      THEN("the simulation should be active") {
        REQUIRE( p.compute_active() == true );
      }
    }

    WHEN("the execution mode is set to CPU") {
      p.enable_cpu_execution_mode();
      THEN("the execution mode should be CPU") {
        REQUIRE( p.execution_mode() == sim::ExecutionMode::CPU );
      }
      AND_THEN("the execution mode should not be GPU") {
        REQUIRE( p.execution_mode() != sim::ExecutionMode::GPU );
      }
    }

    WHEN("the execution mode is set to GPU") {
      p.enable_gpu_execution_mode();
      THEN("the execution mode should be GPU") {
        REQUIRE( p.execution_mode() == sim::ExecutionMode::GPU );
      }
      AND_THEN("the execution mode should not be CPU") {
        REQUIRE( p.execution_mode() != sim::ExecutionMode::CPU );
      }
    }

    WHEN("emitter active is toggled") {
      const bool original = p.emitter_active();
      p.toggle_emitter_active();
      THEN("the emitter active state should be toggled") {
        REQUIRE( original == !p.emitter_active() );
      }
    }

    WHEN("emitter edit is disabled") {
      p.disable_edit_emitter();
      THEN("the emitter edit mode should be disabled") {
        REQUIRE( p.edit_emitter() == false );
      }
    }
    WHEN("emitter edit is enabled") {
      p.enable_edit_emitter();
      THEN("the emitter edit mode should be enabled") {
        REQUIRE( p.edit_emitter() == true );
      }
    }
    AND_WHEN("emitter edit is enabled and then toggled") {
      p.enable_edit_emitter();
      p.toggle_edit_emitter();
      THEN("the emitter edit mode should be disabled") {
        REQUIRE( p.edit_emitter() == false );
      }
    }

    WHEN("view edit is disabled") {
      p.disable_edit_view();
      THEN("the view edit mode should be disabled") {
        REQUIRE( p.edit_view() == false );
      }
    }
    WHEN("view edit is enabled") {
      p.enable_edit_view();
      THEN("the view edit mode should be enabled") {
        REQUIRE( p.edit_view() == true );
      }
    }
    AND_WHEN("view edit is toggled") {
      p.toggle_edit_view();
      THEN("the view edit mode should be disabled") {
        REQUIRE( p.edit_view() == false );
      }
    }

  }
}

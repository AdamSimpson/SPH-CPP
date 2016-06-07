#include "dimension.h"
#include <exception>
#include <iostream>
#include <chrono>

#include "user_input.h"
#include "visualizer.h"
#include "distributor.h"
#include "particles.h"
#include "parameters.h"
#include "emitter.h"
#include "mover.h"
#include "overlay.h"

// Set simulation values
using Real = float;
static const Dimension Dim = three_dimensional;

double update_fps(std::chrono::time_point<std::chrono::high_resolution_clock> &clock_start,
                  Overlay<Real,Dim>& overlay) {
  double fps;
  auto clock_end{std::chrono::high_resolution_clock::now()};
  fps = 1.0 / std::chrono::duration<double>(clock_end - clock_start).count();
  clock_start = clock_end;
  overlay.set_fps(fps);
  std::cout<<"Compute FPS: "<<fps<<std::endl;
}

int main(int argc, char *argv[]) {
  try {
    Distributor<Real, Dim> distributor;
    Parameters<Real, Dim> parameters{"../../../Common/params.ini"};
    Emitter<Real, Dim> emitter{parameters};
    UserInput user_input;

    // @todo remove parameters from visualizer and just pass in world boundary
    Visualizer<Real, Dim> visualizer{parameters};
    Particles particles;
    Container container{static_cast<AABB<Real, Dim>>(parameters.boundary_) };
    Overlay<Real, Dim> overlay{parameters, visualizer.screen_pixel_dimensions()};
    Mover<Real, Dim> mover{parameters};

    visualizer.add_drawable(particles);
    visualizer.add_drawable(container);
    visualizer.add_drawable(overlay);
    visualizer.add_drawable(mover);

    std::future<void> compute_future;
    auto clock_start{std::chrono::high_resolution_clock::now()};

    while(parameters.simulation_active()) {
      user_input.update();
      visualizer.process_input(user_input);
      emitter.process_input(user_input);
      overlay.process_input(user_input);

      // Sync compute and render processes if neccessary
      if(Utility::is_ready(compute_future) && parameters.compute_active()) {
        update_fps(clock_start, overlay);

        // sync particles between distributors copy and renderer
        particles.set_particles(distributor.particle_positions(),
                                distributor.particle_counts(),
                                parameters.particle_radius()/1.5f);

        // launch request for updated compute data asynchronously
        compute_future = std::async(std::launch::async, [&] {
          // Only update mover each compute time step
          mover.process_input(user_input);
          distributor.sync_to_computes(parameters);
          distributor.sync_particles();
        });
      }

      visualizer.draw_scene();
    }

    // Wait for last future to complete if needed
    compute_future.wait();
    // Sync last set of parameters to compute so they know to exit simulation
    distributor.sync_to_computes(parameters);

  } catch(std::exception const& exception) {
      std::cout << "Aborting: " << exception.what() << std::endl;
      abort();
  } catch(...) {
    std::cout << "Aborting: unknown exception" <<std::endl;
    abort();
  }

  return 0;
}

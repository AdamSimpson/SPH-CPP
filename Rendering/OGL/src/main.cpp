#include "dimension.h"
#include <exception>
#include <iostream>

#include "user_input.h"
#include "visualizer.h"
#include "distributor.h"
#include "particles.h"
#include "overlay.h"

int main(int argc, char *argv[]) {
  try {
    Distributor<float, three_dimensional> distributor;
    Parameters<float,three_dimensional> parameters{"../../../Common/params.ini"};
    UserInput user_input;
    // @todo remove parameters from visualizer and just pass in world boundary
    Visualizer<float,three_dimensional> visualizer{parameters};
    Particles particles;
    Container container{static_cast<AABB<float, three_dimensional>>(parameters.boundary_) };
    Overlay<float, three_dimensional> overlay{parameters, visualizer.screen_pixel_dimensions()};

    visualizer.add_drawable(particles);
    visualizer.add_drawable(container);
    visualizer.add_drawable(overlay);

    std::future<void> compute_future;

    while(parameters.simulation_active()) {
      user_input.update();
      visualizer.process_input(user_input);
      overlay.process_input(user_input);

      // Sync compute and render processes if neccessary
      if(Utility::is_ready(compute_future) && parameters.compute_active()) {
        // sync particles between distributors copy and renderer
        particles.set_particles(distributor.particle_positions(), parameters.particle_radius()/1.5f);

        // launch request for updated compute data asynchronously
        compute_future = std::async(std::launch::async, [&] {
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

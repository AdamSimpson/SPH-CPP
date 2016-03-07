#include "dimension.h"
#include <exception>
#include <iostream>
#include <future>

#include "visualizer.h"
#include "distributor.h"
#include "particles.h"

// Test if future has completed
template<typename R>
bool is_ready(std::future<R> const& f) {
  if(!f.valid()) // future hasn't been assigned yet
    return true;
  else
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

int main(int argc, char *argv[]) {
  try {
    Distributor<float, three_dimensional> distributor;
    Parameters<float,three_dimensional> parameters{"../../../Common/params.ini"};
    Visualizer<float,three_dimensional> visualizer{parameters};
    std::future<void> compute_future;

    while(parameters.simulation_active()) {
      visualizer.process_input();

      // Sync compute and render processes if neccessary
      if(is_ready(compute_future) && parameters.compute_active()) {
        // sync particles between distributors copy and renderer
        visualizer.particles().clear();
        visualizer.particles().add_particles(distributor.particle_positions());

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

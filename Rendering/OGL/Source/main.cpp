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

void update_fps(std::chrono::time_point<std::chrono::high_resolution_clock> &clock_start,
                  sim::Overlay<Real,Dim>& overlay) {
  double fps;
  auto clock_end{std::chrono::high_resolution_clock::now()};
  fps = 1.0 / std::chrono::duration<double>(clock_end - clock_start).count();
  clock_start = clock_end;
  overlay.set_fps(fps);
  std::cout<<"Compute FPS: "<<fps<<std::endl;
}

int main(int argc, char *argv[]) {
  try {
    sim::Distributor<Real, Dim> distributor;
    sim::Parameters<Real, Dim> parameters{"../../../Common/params.ini"};
    sim::Emitter<Real, Dim> emitter{parameters};
    sim::UserInput user_input;

    // @todo remove parameters from visualizer and just pass in world boundary
    sim::Visualizer<Real, Dim> visualizer{parameters};
    sim::Particles particles;
    // @todo get rid of this hack
    AABB<float, Dim> container_boundary{parameters.boundary_.min, parameters.boundary_.max};
//    Container container{static_cast<AABB<float, Dim>>(parameters.boundary_) };
    sim::Container container{container_boundary};
    sim::Overlay<Real, Dim> overlay{parameters, visualizer.screen_pixel_dimensions()};
    sim::Mover<Real, Dim> mover{parameters};

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

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

#include <exception>
#include <iostream>
#include "parameters.h"
#include "particles.h"
#include "distributor.h"
#include "adios_writer.h"

int main(int argc, char *argv[]) {
  try {
    // Probably should have some kind of builder
    sim::Distributor <float,three_dimensional> distributor;

    auto parameters  = new sim::Parameters<float, three_dimensional>{"params.ini"};

    // The use of new isn't enforced but is required as managed memory is used
    auto particles = new sim::Particles<float, three_dimensional>(*parameters);

    sim::AdiosWriter<float, three_dimensional> writer{"adios_writer.xml",distributor};

    // After particles have been created construct initial fluid
    distributor.initilize_fluid(*particles, *parameters);

    int64_t step = 0;

    // Number of simulation steps per render step
    const int target_fps = 60;
    const int steps_per_save = (int)std::round(1.0 / parameters->time_step() / target_fps);

    // Main time step loop
    const int step_count = 300;
    for(int i=0; i<step_count; i++) {
      if(parameters->compute_active()) {
        distributor.process_parameters(*parameters, *particles);

        particles->apply_external_forces(distributor.resident_span());

        particles->predict_positions(distributor.resident_span());

        distributor.balance_domains();

        distributor.domain_sync(*particles);

        particles->find_neighbors(distributor.local_span(),
                                 distributor.resident_span());

        for(int sub=0; sub<parameters->solve_step_count(); sub++) {

          particles->compute_densities(distributor.resident_span());

          particles->compute_pressure_lambdas(distributor.resident_span());
          distributor.initiate_sync_halo_scalar(particles->lambdas());
          distributor.finalize_sync_halo_scalar();

          particles->compute_pressure_dps(distributor.resident_span(), sub);

          particles->update_position_stars(distributor.resident_span());
          distributor.initiate_sync_halo_vec(particles->position_stars());
          distributor.finalize_sync_halo_vec();

          particles->apply_surface_tension(distributor.local_span(), distributor.resident_span());
        }

        particles->update_velocities(distributor.local_span());

        distributor.initiate_sync_halo_scalar(particles->densities());
        distributor.finalize_sync_halo_scalar();

        particles->apply_surface_tension(distributor.local_span(), distributor.resident_span());

        particles->apply_viscosity(distributor.resident_span());

        distributor.initiate_sync_halo_vec(particles->velocities());
        distributor.finalize_sync_halo_vec();

        particles->compute_vorticity(distributor.resident_span());

        distributor.initiate_sync_halo_vec(particles->scratch());
        distributor.finalize_sync_halo_vec();

        particles->apply_vorticity(distributor.resident_span());

        particles->apply_viscosity(distributor.resident_span());

        particles->update_positions(distributor.resident_span());

        // Needs to be done once per rendered step
        if(step % steps_per_save == 0) {
          std::cout<<"Writing step "<< step << std::endl;
          writer.write_particles(*particles);
        }

        step++;

        distributor.invalidate_halo(*particles);
      }

    }
    delete parameters;
    delete particles;

  } catch(std::exception const& exception) {
      std::cout << "Aborting: " << exception.what() << std::endl;
      return 1;
  } catch(...) {
    std::cout << "Aborting: unknown exception" <<std::endl;
    return 1;
  }

  return 0;
}

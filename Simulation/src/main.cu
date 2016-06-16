#include <exception>
#include <iostream>
#include "particles.h"
#include "distributor.h"

int main(int argc, char *argv[]) {
  try {
    // Probably should have some kind of builder
    Distributor <float,three_dimensional> distributor;

    auto parameters  = new Parameters<float, three_dimensional>;

    distributor.sync_from_renderer(*parameters);

    // @todo enforce dynamic construction requirment
    // The use of new isn't enforced but is required as managed memory is used
    auto particles = new Particles<float, three_dimensional>(*parameters);

    // After particles have been created construct initial fluid
    distributor.initilize_fluid(*particles, *parameters);

    // Sync initial particle configuration
    distributor.sync_to_renderer(*particles);

    int64_t frame = 0;

    // Number of simulation frames per render frame
    const int target_fps = 60;
    const int frames_per_update = (int)std::round(1.0 / parameters->time_step() / target_fps);
    std::cout<<"Compute updating renderer every "<<frames_per_update<<" frames"<<std::endl;

    // Main time step loop
    while(parameters->simulation_active()) {
      if(frame % frames_per_update == 0)
        distributor.sync_from_renderer(*parameters);

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

          //        particles_.compute_surface_lambdas(distributor_.local_span());
          //        particles_.compute_surface_dps(distributor_.local_span(), sub);
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

        particles->update_positions(distributor.resident_span());

        // Needs to be done once per rendered frame
        if(frame % frames_per_update == 0)
          distributor.sync_to_renderer(*particles);

        frame++;

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

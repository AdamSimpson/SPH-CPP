#include <exception>
#include <iostream>
#include "particles.h"
#include "distributor.h"

int main(int argc, char *argv[]) {
  try {
    // Probably should have some kind of builder
    Distributor <float,three_dimensional> distributor;

    Parameters  <float,three_dimensional> parameters;
    distributor.sync_from_renderer(parameters);

    Particles <float,three_dimensional> particles{parameters};

    // After particles have been created construct initial fluid
    distributor.initilize_fluid(particles, parameters);

    // Sync initial particle configuration
    distributor.sync_to_renderer(particles);

    int64_t frame = 0;

    // Number of simulation frames per render frame
    int frames_per_update = (int)std::round(1.0 / parameters.time_step() / 60.0);

    // Main time step loop
    while(parameters.simulation_active()) {
      if(frame % frames_per_update == 0)
        distributor.sync_from_renderer(parameters);

      if(parameters.compute_active()) {

        // Add a few particles
        if(frame%5 == 0) {
          AABB<float, three_dimensional> add_volume;
          add_volume.min = Vec<float, three_dimensional>{0.1, 2.0, 0.7};
          add_volume.max = Vec<float, three_dimensional>{0.1 + parameters.particle_rest_spacing(), 2.5, 1.1};
          const auto velocity = parameters.particle_rest_spacing()/parameters.time_step()/5;
          distributor.distribute_fluid(add_volume, particles, parameters.particle_rest_spacing(),
                                        Vec<float,3>{velocity, 0.0, 0.0});
        }

        particles.apply_external_forces(distributor.resident_span());
        particles.predict_positions(distributor.resident_span());

        distributor.balance_domains();

        distributor.domain_sync(particles);

        particles.find_neighbors(distributor.local_span(),
                                 distributor.resident_span());

        for(int sub=0; sub<parameters.solve_step_count(); sub++) {
          particles.compute_densities(distributor.resident_span());

          particles.compute_pressure_lambdas(distributor.resident_span());
          distributor.initiate_sync_halo_scalar(particles.lambdas());
          distributor.finalize_sync_halo_scalar();

          particles.compute_pressure_dps(distributor.resident_span(), sub);

          particles.update_position_stars(distributor.resident_span());
          distributor.initiate_sync_halo_vec(particles.position_stars());
          distributor.finalize_sync_halo_vec();

          //        particles_.compute_surface_lambdas(distributor_.local_span());
          //        particles_.compute_surface_dps(distributor_.local_span(), sub);
        }

        particles.update_velocities(distributor.resident_span());

        distributor.initiate_sync_halo_scalar(particles.densities());
        distributor.finalize_sync_halo_scalar();

        particles.apply_surface_tension(distributor.local_span(), distributor.resident_span());
/*
        particles.apply_viscosity(distributor.resident_span());

        particles.compute_vorticity(distributor.resident_span());

        particles.apply_vorticity(distributor.resident_span());
*/
        particles.update_positions(distributor.resident_span());

        // Needs to be done once per rendered frame
        if(frame % frames_per_update == 0)
          distributor.sync_to_renderer(particles);

        frame++;

        distributor.invalidate_halo(particles);
      }
    }

  } catch(std::exception const& exception) {
      std::cout << "Aborting: " << exception.what() << std::endl;
      return 1;
  } catch(...) {
    std::cout << "Aborting: unknown exception" <<std::endl;
    return 1;
  }

  return 0;
}

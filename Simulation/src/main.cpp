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

    Particles   <float,three_dimensional> particles{parameters};

    // After particles have been created construct initial fluid
    distributor.initilize_fluid(particles, parameters);

    // Sync initial particle configuration
    distributor.sync_to_renderer(particles);

    // Main time step loop
    while(parameters.simulation_active()) {
      distributor.sync_from_renderer(parameters);

      if(parameters.compute_active()) {
        particles.apply_external_forces(distributor.resident_span());
        particles.predict_positions(distributor.resident_span());

        // @todo Balance nodes

        //      distributor_.domain_sync();

        particles.find_neighbors(distributor.local_span(),
                                 distributor.resident_span());

        for(int sub=0; sub<parameters.solve_step_count(); sub++) {
          particles.compute_densities(distributor.local_span());
          // update halo densities
          particles.compute_pressure_lambdas(distributor.local_span());
          // update halo lambdas
          particles.compute_pressure_dps(distributor.local_span(), sub);
          // update halo deltas
          particles.update_position_stars(distributor.local_span());
          // update position star halos?
          //        particles_.compute_surface_lambdas(distributor_.local_span());
          //        particles_.compute_surface_dps(distributor_.local_span(), sub);
          //        particles_.update_position_stars(distributor_.local_span());
        }

        particles.update_velocities(distributor.local_span());

        particles.apply_surface_tension(distributor.local_span());

        particles.apply_viscosity(distributor.local_span());

        // vorticity
        // vorticity confinement

        particles.update_positions(distributor.local_span());

        // Needs to be done once per rendered frame(?)
        distributor.sync_to_renderer(particles);
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

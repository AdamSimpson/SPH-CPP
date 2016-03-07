#pragma once

#include "dimension.h"
#include "particles.h"
#include "parameters.h"
#include <boost/mpi.hpp>
#include <stdexcept>
#include "vec.h"
#include <vector>
#include "boost_mpi_optimizations.h"

template<typename Real, Dimension Dim>
class Distributor {
public:

  /**
    Distributor handles the coupling between the compute processes and render process
  **/
  Distributor() : environment_{boost::mpi::threading::level::multiple, true},
                  comm_render_{comm_world_.split(0)} {

    if(comm_render_.size() != 1 || comm_world_.rank() != 0)
        throw std::runtime_error("Renderer must be rank 0!\n");
  }

  // The following would be simplified by boost MPI_IN_PLACE support
  void sync_particles() {
    // Remove old particle positions
    particle_positions_.clear();

    // Gather number of particle coordinates that will be sent from each compute process
    std::vector<int> particle_counts;
    int particle_count = 0;
    boost::mpi::gather(comm_world_, particle_count, particle_counts, 0);

    uint64_t receive_count = 0;
    for (int n : particle_counts)
      receive_count += n;

    // Ensure enough space is available in receieve buffer
    particle_positions_.resize(receive_count);

    // Gather particle vec coordinates
    Vec<Real,Dim> dummy_input;
    boost::mpi::gatherv(comm_world_, &dummy_input, 0, particle_positions_.data(), particle_counts, 0);
  }

  const std::vector< Vec<Real,Dim> >& particle_positions() const {
    return particle_positions_;
  }

  // Sync simulation parameters
  void sync_to_computes(Parameters<Real,Dim> & parameters) {
    // Broadcast parameters from render node to compute nodes
    boost::mpi::broadcast(comm_world_, parameters ,0);
  }

private:
  boost::mpi::environment environment_;
  boost::mpi::communicator comm_world_;
  boost::mpi::communicator comm_render_;
  std::vector< Vec<Real,Dim> > particle_positions_;
};

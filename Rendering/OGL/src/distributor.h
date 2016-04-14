#pragma once

#include "dimension.h"
#include "particles.h"
#include "parameters.h"
#include <stdexcept>
#include "vec.h"
#include <vector>
#include "mpi++.h"

template<typename Real, Dimension Dim>
class Distributor {
public:

  /**
    Distributor handles the coupling between the compute processes and render process
  **/
  Distributor() : comm_render_{0} {

    if(comm_render_.size() != 1 || comm_world_.rank() != 0)
        throw std::runtime_error("Renderer must be rank 0!\n");

    sim::mpi::create_mpi_types<Real,Dim>(MPI_VEC_, MPI_PARAMETERS_);
  }

  ~Distributor() {
    sim::mpi::free_mpi_types(MPI_VEC_, MPI_PARAMETERS_);
  }

  void sync_particles() {
    // Remove old particle positions
    particle_positions_.clear();

    // Gather number of particle coordinates that will be sent from each compute process
    std::vector<int> particle_counts;
    particle_counts.resize(comm_world_.size());

    particle_counts[0] = 0;
    comm_world_.gather(particle_counts.data(), MPI_INT);

    uint64_t receive_count = 0;
    for (int n : particle_counts)
      receive_count += n;

    // Ensure enough space is available in receieve buffer
    particle_positions_.resize(receive_count);

    // Gather particle vec coordinates
    comm_world_.gatherv(particle_positions_.data(), particle_counts, MPI_VEC_);
  }

  const std::vector< Vec<Real,Dim> >& particle_positions() const {
    return particle_positions_;
  }

  // Sync simulation parameters
  void sync_to_computes(Parameters<Real,Dim>& parameters) {
    // Broadcast parameters from render node to compute nodes
    comm_world_.broadcast(&parameters, MPI_PARAMETERS_, 0);
  }

private:
  const sim::mpi::Environment environment_;
  const sim::mpi::Communicator comm_world_;
  const sim::mpi::Communicator comm_render_;
  std::vector< Vec<Real,Dim> > particle_positions_;
  MPI_Datatype MPI_VEC_;
  MPI_Datatype MPI_PARAMETERS_;
};

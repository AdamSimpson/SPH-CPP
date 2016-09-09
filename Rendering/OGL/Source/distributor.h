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

#pragma once

#include "dimension.h"
#include "particles.h"
#include "parameters.h"
#include <stdexcept>
#include "vec.h"
#include <vector>
#include "mpi++.h"

namespace sim {
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
    particle_counts_.resize(comm_world_.size());
    particle_counts_[0] = 0;
    comm_world_.gather(particle_counts_.data(), sim::mpi::get_mpi_size_t());

    uint64_t receive_count = 0;
    for (std::size_t n : particle_counts_)
      receive_count += n;

    // Ensure enough space is available in receive buffer
    particle_positions_.resize(receive_count);

    // Gather particle vec coordinates
    std::vector<int> particle_counts_int(particle_counts_.begin(), particle_counts_.end());
    comm_world_.gatherv(particle_positions_.data(), particle_counts_int, MPI_VEC_);

    // Remove renderer 0 count from particle_counts
    particle_counts_.erase(particle_counts_.begin());
  }

  const std::vector< Vec<Real,Dim> >& particle_positions() const {
    return particle_positions_;
  }

  const std::vector<std::size_t>& particle_counts() const {
    return particle_counts_;
  }

  // Sync simulation parameters
  void sync_to_computes(Parameters<Real,Dim>& parameters) {
    // Broadcast parameters from render node to compute nodes
    comm_world_.broadcast(&parameters, MPI_PARAMETERS_, 0);
  }

private:
  const sim::mpi::Environment environment_{true};
  const sim::mpi::Communicator comm_world_;
  const sim::mpi::Communicator comm_render_;
  std::vector< Vec<Real,Dim> > particle_positions_;
  std::vector<std::size_t> particle_counts_;
  MPI_Datatype MPI_VEC_;
  MPI_Datatype MPI_PARAMETERS_;
};
}

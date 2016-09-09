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

//#include <boost/mpi.hpp>
#include <utility>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "vec.h"
#include "utility_math.h"
#include "particles.h"
#include "parameters.h"
#include "thrust/execution_policy.h"
#include <thrust/iterator/zip_iterator.h>
#include "parameters.h"
#include "mpi++.h"
#include "device.h"
#include "sim_algorithms.h"

/***
  The distributor is responsible for all domain-to-domain communication as
  well as maintaning the particle array arangement described below.
  Although it seems slightly odd the idea is that the distributor is the
  only class that's aware the simulation is distributed.

  Particle arrays are arranged as such: { interior, edge_left, edge_right, halo_left, halo_right }

  interior: particles which can be fully processed without halo information
  edge: left/right edge particles from neighboring domains
  resident: interor + edge particles(non halo particles)
  halo: left/right edge particles from neighboring domains
  local: resident + halo particles

  index spans are defined for each region such that [begin,end) (end is one past the last index) defines the indices
  of the specified region in the particle arrays. An array_span
  is not appropriate as a particle is not defined by a single struct but by a SoA
***/

namespace sim {
template<typename Real, Dimension Dim>
class Distributor {
public:

  typedef thrust::tuple< const Vec<Real,Dim>&, const Vec<Real,Dim>&, const Vec<Real,Dim>& > Tuple;
  typedef thrust::zip_iterator<Tuple> ZippedTuple;

  /*! Distributor constructor
   *
   * Distributor Constructor: Don't use member references!
   * Distributor must be constructed before any other members
   * due to MPI_Init() call.
   * Requires Initilization step after construction of member references
   * compute_comm is split with color value 1 to differentiate compute processes from I/O, viz processes
   *
   * @param manage_mpi rather the distributor should initilize and finalize the MPI environment
   */
  Distributor(bool manage_mpi=true) :
      environment_{manage_mpi},
      comm_compute_{1},
      domain_{0.0, 0.0},
      resident_count_{0},
      edge_left_count_{0},
      edge_right_count_{0},
      halo_count_left_{0},
      halo_count_right_{0},
      receive_left_index_{0},
      receive_right_index_{0} {
                    sim::mpi::create_mpi_types<Real,Dim>(MPI_VEC_, MPI_PARAMETERS_);
  }

  /*! Default destructor
   */
  ~Distributor()                             = default;

  Distributor(const Distributor&)            = delete;
  Distributor& operator=(const Distributor&) = delete;
  Distributor(Distributor&&) noexcept        = delete;
  Distributor& operator=(Distributor&&)      = delete;

  /*! Initialize fluid distributed amonst multiple processes
   * Note: Handle initilization requiring constructed member references
   */
  void initilize_fluid(Particles<Real,Dim> & particles,
                       const Parameters<Real,Dim> & parameters) {
    this->set_domain_bounds(parameters.initial_fluid(),
                            parameters.boundary());
    edge_width_ = 1.2*parameters.smoothing_radius();
    this->distribute_fluid(parameters.initial_fluid(),
                           particles,
                           parameters.particle_rest_spacing(),
                           Vec<Real,Dim>{(Real)0.0});
  }

  /*! Set domain bounds based upon equal spacing in AABB
   */
  void set_domain_bounds(const AABB<Real,Dim>& initial_fluid,
                         const AABB<Real,Dim>& global_boundary) {
    Real domain_length = initial_fluid.length()/comm_compute_.size();
    domain_.begin = initial_fluid.min.x + comm_compute_.rank() * domain_length;
    domain_.end = domain_.begin + domain_length;

    // Stretch domain of first and last ranks to fit global boundary
    if(this->is_last_domain())
      domain_.end = global_boundary.max.x;
    if(this->is_first_domain())
      domain_.begin = global_boundary.min.x;
  }

  /*! Checks for balanced number of particles in each domain
   */
  void balance_domains() {
    // Particles per domain if evenly divided
    const int64_t even_count = global_resident_count() / comm_compute_.size();
    const int64_t max_diff = even_count * 0.05;

    // Fixed distance to move partition if unbalanced
    const Real dx = edge_width_ * (Real)0.15;

    // Minimum domain width
    const Real min_width = 3.0*edge_width_;

    // Exchange information with neighbors
    const std::size_t my_count = resident_count();
    std::size_t right_count = 0;
    comm_compute_.send_recv(&my_count, this->domain_to_left(),
                            &right_count, this->domain_to_right(),
                            1, sim::mpi::get_mpi_size_t());

    const Real my_length = domain_.end - domain_.begin;
    Real left_length = 0.0;
    Real right_length = 0.0;
    comm_compute_.send_recv(&my_length, this->domain_to_left(),
                            &left_length, this->domain_to_left(),
                            1, sim::mpi::get_mpi_type<Real>());
    comm_compute_.send_recv(&my_length, this->domain_to_right(),
                            &right_length, this->domain_to_right(),
                            1, sim::mpi::get_mpi_type<Real>());

    const int64_t diff = my_count - even_count;
    const int64_t right_diff = right_count - even_count;

    if(domain_to_left() != MPI_PROC_NULL) {
      // domain has too many particles
      if(diff > max_diff && my_length > min_width)
        domain_.begin += dx;
      // domain has too few particles
      if(diff < -max_diff && left_length > min_width)
        domain_.begin -= dx;
    }

    if(domain_to_right() != MPI_PROC_NULL) {
      // domain to right has too many particles
      if(right_diff > max_diff && right_length > min_width)
        domain_.end += dx;
      // domain to right has too few particles
      if(right_diff < -max_diff && my_length > min_width)
        domain_.end -= dx;
    }

//    std::cout<<"bounds for rank: "<<comm_compute_.rank()<<" "<<domain_.begin<<", "<<domain_.end<<std::endl;
  }

  /*! Check if domain is the last, maximum in x direction
   * @return True if last rank in domain
   */
  bool is_last_domain() const {
    return comm_compute_.rank() == comm_compute_.size()-1;
  }

  /*! Check if domain is the first, minimum in x direction
   * @return True if first rank in domain
   */
  bool is_first_domain() const {
    return comm_compute_.rank() == 0;
  }

  /*! Fetch domain to the left of this ranks domain
   * @return Rank of domain to the left or MPI_PROC_NULL
   */
  int domain_to_left() const {
    int left = MPI_PROC_NULL;
    if(comm_compute_.rank() > 0)
      left = comm_compute_.rank() - 1;

    return left;
  }

  /*! Fetch domain to the right of this ranks domain
   * @return Rank of domain to the right or MPI_PROC_NULL
   */
  int domain_to_right() const {
    int right = MPI_PROC_NULL;
    if(comm_compute_.rank() != comm_compute_.size()-1)
      right = comm_compute_.rank() + 1;

    return right;
  }

  /*! Get the MPI rank within the compute communicator
   * @return Rank within the compute communicator
   */
  int compute_rank() const {
    return comm_compute_.rank();
  }

  /*! Get the compute MPI communicator
   * @return the MPI_Comm associated with the compute ranks
   */
  const mpi::Communicator& comm_compute() const {
    return comm_compute_;
  }

  /*! Get the local particle span, resident + halo particles
    * @return local particle span
    */
  IndexSpan local_span() const {
    return IndexSpan{0, this->halo_span().end};
  }

  /*! Get the resident particle span, interor + edge particles(non halo particles)
     @return resident particle span
   */
  IndexSpan resident_span() const {
    return IndexSpan{0, resident_count_};
  }

  /*! Get the current number of resident particles
     @return number of resident particles
   */
  std::size_t resident_count() const {
    return resident_count_;
  }

  /*! Get the current number of edge particles
     @return count of edge particles
   */
  std::size_t edge_count() const {
    return edge_left_count_ + edge_right_count_;
  }

  /*! Get the span defining the edge particle indices:
     @return span of edge particle indices: left/right edge particles from neighboring domains
   */
  IndexSpan edge_span() const {
    return IndexSpan(resident_count_ - edge_count(), resident_count_);
  }

  /*! Get the count of halo particles
     @return count of halo particles
   */
  std::size_t halo_count() const {
    return halo_count_left_ + halo_count_right_;
  }

  /*! Get the span defining halo particles: left/right edge particles from neighboring domains
     @return span of halo indices
   */
  IndexSpan halo_span() const {
    return IndexSpan(this->resident_span().end, this->resident_span().end + halo_count());
  }

  /*! Get the span defining the interior particles: particles which can be fully processed without halo information
     @return span of interior indices
   */
  IndexSpan interior_span() const {
    return IndexSpan{0,  this->resident_span().end - edge_count()};
  }

  /*! Get the count of interior particles
     @return count of interior particles
   */
  std::size_t interior_count() const {
    return this->interior_span().end - this->interior_span().begin;
  }

  /*! Get the count of local particles
     @return count of local particles
   */
  std::size_t local_count() const {
    return this->local_span().end - this->local_span().begin;
  }

  /*! Get the number of global resident particles
     @return number of global resident particles
   */
  std::size_t global_resident_count() const {
    std::size_t global_count = 0;
    std::size_t local_count = this->resident_count();
    comm_compute_.all_reduce(&local_count, &global_count, MPI_UINT64_T, MPI_SUM);
    return global_count;
  }

  /*! Distribute AABB across multiple nodes and fill with particles
   * Construct water volume spread across multiple domains
   * This function requires the domain bounds to be set
   * @todo, this is overcomplicated and each rank should just construct the fluid and reject out of bounds particles
   */
  void distribute_fluid(const AABB<Real,Dim>& global_fluid,
                        Particles<Real,Dim> & particles,
                        Real particle_rest_spacing,
                        const Vec<Real,Dim> velocity) {
    // Create a bounding box with length dimension that coincides with particle spacing
    // |--|--|--|
    // -o--o--o-
    Real spacing = particle_rest_spacing;
    AABB<Real,Dim> local_fluid = global_fluid;

    // If local_fluid not in domain then return
    bool contains_fluid_start = (global_fluid.min.x >= domain_.begin) &&
                                (global_fluid.min.x <= domain_.end);
    bool contains_fluid_end   = (global_fluid.max.x >= domain_.begin) &&
                                (global_fluid.max.x <= domain_.end);
    bool filled_with_fluid    = (global_fluid.min.x <= domain_.begin) &&
                                (global_fluid.max.x >= domain_.begin);
    if(!contains_fluid_start && !contains_fluid_end && !filled_with_fluid)
      return;

    // Ensure that particle spacing is consistent at boundaries
    int x_count_previous = std::max((Real)0.0, std::floor((domain_.begin - global_fluid.min.x)/(Real)spacing));
    local_fluid.min.x = global_fluid.min.x + x_count_previous * spacing;

    if(contains_fluid_end)
      local_fluid.max.x = global_fluid.max.x;
    else
      local_fluid.max.x = domain_.end;

    // Fill AABB with particles
    auto particles_added = particles.construct_fluid(local_fluid, velocity);
    resident_count_ += particles_added;
  }

  /*! Process parameters and apply any changes that must be done distributed
   */
  void process_parameters(const Parameters<Real,Dim>& parameters,
                          Particles<Real,Dim> & particles) {
    if(parameters.emitter_active()) {
      AABB<Real, three_dimensional> add_volume;
      Vec<Real, three_dimensional> emitter_volume_extents{(Real)1.1 * parameters.particle_rest_spacing(),
                                                          (Real)1.1 * parameters.particle_rest_spacing(),
                                                          (Real)1.1 * parameters.particle_rest_spacing()};
      add_volume.min = parameters.emitter_center() - (Real)0.5 * emitter_volume_extents;
      add_volume.max = add_volume.min + emitter_volume_extents;
      this->distribute_fluid(add_volume, particles, parameters.particle_rest_spacing(),
                                   parameters.emitter_velocity());
    }
  }

  // Incoming OOB particles will be appended so we remove old halo particles first
  void invalidate_halo(Particles<Real,Dim> & particles) {
    this->remove_halo_particles(particles);
  }

  /*! Transfer out of bounds particles and update halo
   *  note: invalidate_halo must be called before domain_sync
   */
  void domain_sync(Particles<Real,Dim> & particles) {

    this->initiate_oob_exchange(particles);
    this->finalize_oob_exchange(particles);

    this->initiate_halo_exchange(particles);
    this->finalize_halo_exchange(particles);
  }

  /*! Initiate syncronize of halo scalar values
   * @param halo_values scalar array of values to be synced between left/right domains
   */
  void initiate_sync_halo_scalar(sim::Array<Real>& halo_values) {
    const int max_recv_left = halo_count_left_;
    const int max_recv_right = halo_count_right_;

    const std::size_t receive_left_index = resident_count();
    const std::size_t receive_right_index = receive_left_index + halo_count_left_;

    const std::size_t send_left_index = interior_count();
    const std::size_t send_right_index = send_left_index + edge_left_count_;

    requests_[0] = comm_compute_.i_recv(this->domain_to_left(), 0,
                                       &halo_values[receive_left_index], max_recv_left, sim::mpi::get_mpi_type<Real>());
    requests_[1] = comm_compute_.i_recv(this->domain_to_right(), 1,
                                       &halo_values[receive_right_index], max_recv_right, sim::mpi::get_mpi_type<Real>());

    requests_[2] = comm_compute_.i_send(this->domain_to_left(), 1,
                                       &halo_values[send_left_index], edge_left_count_, sim::mpi::get_mpi_type<Real>());
    requests_[3]  = comm_compute_.i_send(this->domain_to_right(), 0,
                                        &halo_values[send_right_index], edge_right_count_, sim::mpi::get_mpi_type<Real>());
  }

  /*! Finalize halo scarlar sync
   */
  void finalize_sync_halo_scalar() {
    MPI_Status statuses[4];
    sim::mpi::wait_all(requests_, 4, statuses);
  }

  /*! Initiate syncronize of halo vec values
   * @param halo_values vec array of values to be synced between left/right domains
   */
  void initiate_sync_halo_vec(sim::Array<Vec<Real,Dim>> & halo_values) {
    const int max_recv_left = halo_count_left_;
    const int max_recv_right = halo_count_right_;

    const std::size_t receive_left_index = resident_count();
    const std::size_t receive_right_index = receive_left_index + halo_count_left_;

    const std::size_t send_left_index = interior_count();
    const std::size_t send_right_index = send_left_index + edge_left_count_;

    requests_[0] = comm_compute_.i_recv(this->domain_to_left(), 0,
                                       &halo_values[receive_left_index], max_recv_left, MPI_VEC_);
    requests_[1] = comm_compute_.i_recv(this->domain_to_right(), 1,
                                       &halo_values[receive_right_index], max_recv_right, MPI_VEC_);

    requests_[2] = comm_compute_.i_send(this->domain_to_left(), 1,
                                       &halo_values[send_left_index], edge_left_count_, MPI_VEC_);
    requests_[3]  = comm_compute_.i_send(this->domain_to_right(), 0,
                                        &halo_values[send_right_index], edge_right_count_, MPI_VEC_);
  }

  /*! Finalize halo vec sync
   */
  void finalize_sync_halo_vec() {
    MPI_Status statuses[4];
    sim::mpi::wait_all(requests_, 4, statuses);
  }

public:
// @todo wait for nvidia to allow private access
// currently DEVICE_CALLABLE lambdas don't allow private access
// private:

  const sim::mpi::Environment environment_;    /**< MPI environment */
  const sim::mpi::Communicator comm_world_;    /**< World communicator */
  const sim::mpi::Communicator comm_compute_;  /**< Compute subset of simulation */
  Vec<Real,2>  domain_;                        /**< X coordinate of domain domain range */
  Real edge_width_;                            /**< Width from domain bounds which is needed by neighboring domains */

  std::size_t resident_count_;                 /**< Count of interor + edge particles(non halo particles) */
  std::size_t edge_left_count_;                /**< Count of particles within edge_width_ of left domain boundary */
  std::size_t edge_right_count_;               /**< Count of particles within edge_width_ of right domain boundary */
  std::size_t halo_count_left_;                /**< Count of left domains right edge particles residing on current domain */
  std::size_t halo_count_right_;               /**< Count of right domains left edge particles residing on current domain */
  std::size_t receive_left_index_;             /**< Index in which to receieve OOB particles from left domain */
  std::size_t receive_right_index_;            /**< Index in which to receieve OOB particles from right domain */
  std::size_t oob_left_count_;                 /**< Count of particles which have left current domain to the left */
  std::size_t oob_right_count_;                /**< Count of particles which have left current domain to the right */

  MPI_Request requests_[12];                   /**< Array of requests to keep track of async MPI calls */

  MPI_Datatype MPI_VEC_;                       /**< Vec<Real,Dim> MPI type */
  MPI_Datatype MPI_PARAMETERS_;                /**< MPI_Parameters<Real,Dim> MPI type */

  /*! Invalidates halo particles
  **/
  void remove_halo_particles(Particles<Real,Dim> & particles) {
    particles.remove(this->halo_count());
    halo_count_left_ = 0;
    halo_count_right_ = 0;
  }

  /*! Remove resident particles
   * @param particles Particles in which to remove from
   * @param count count of particles to remove
   */
  void remove_resident_particles(Particles<Real,Dim> & particles,
                                 std::size_t count) {
    particles.remove(count);
    resident_count_ -= count;
  }

  /*! Add resident particles
   * @param particles Particles in which to add to
   * @param new_positions Pointer to new positions
   * @param new_position_stars Pointer to new position_stars
   * @param new_velocities Pointer to new velocities
   * @param count Number of new particles to add
   */
  void add_resident_particles(Particles<Real,Dim> & particles,
                              const Vec<Real,Dim>* new_positions,
                              const Vec<Real,Dim>* new_position_stars,
                              const Vec<Real,Dim>* new_velocities,
                              std::size_t count) {
    particles.add(new_positions, new_position_stars, new_velocities, count);
    resident_count_ += count;
  }

  /*! Add halo particles from domain to the left
   * @param particles Particles in which to add halos to
   * @param new_positions Pointer to new halo positions
   * @param new_position_stars Pointer to new halo position_stars
   * @param new_velocities Pointer to new halo velocities
   * @param count Number of new halo particles to add
   */
  void add_halo_particles_left(Particles<Real,Dim> & particles,
                          const Vec<Real,Dim>* new_positions,
                          const Vec<Real,Dim>* new_position_stars,
                          const Vec<Real,Dim>* new_velocities,
                          std::size_t count) {
    particles.add(new_positions, new_position_stars, new_velocities, count);
    halo_count_left_ += count;
  }

  /*! Add halo particles from domain to the right
   * @param particles Particles in which to add halos to
   * @param new_positions Pointer to new halo positions
   * @param new_position_stars Pointer to new halo position_stars
   * @param new_velocities Pointer to new halo velocities
   * @param count Number of new halo particles to add
   */
  void add_halo_particles_right(Particles<Real,Dim> & particles,
                          const Vec<Real,Dim>* new_positions,
                          const Vec<Real,Dim>* new_position_stars,
                          const Vec<Real,Dim>* new_velocities,
                          std::size_t count) {
    particles.add(new_positions, new_position_stars, new_velocities, count);
    halo_count_right_ += count;
  }

  /*! Initiate syncronize of particles that have gone out of the domain bounds(OOB)
   */
  void initiate_oob_exchange(Particles<Real,Dim> & particles) {
    // Zip iterator of pointers to particle quantities to update
    const auto begin = thrust::make_zip_iterator(thrust::make_tuple(particles.position_stars().data(),
                                                                    particles.positions().data(),
                                                                    particles.velocities().data()));
    const auto end = begin + this->resident_count_;

    // These must be unpacked as domain isn't available in lambda
    const auto domain_begin = domain_.begin;
    const auto domain_end = domain_.end;

    // Move oob-left/right particles to end of arrays
    auto oob_begin = sim::algorithms::partition(begin, end, [=] DEVICE_CALLABLE (const Tuple& tuple) {
      const auto position_star = thrust::get<0>(tuple);
      const auto x_star = position_star.x;
      return (x_star >= domain_begin && x_star <= domain_end); // True if not OOB
    });
    // Move oob-right to end of array, arrays now {staying,oob-left,oob-right}
    auto oob_right_begin = sim::algorithms::partition(oob_begin, end, [=] DEVICE_CALLABLE (const Tuple& tuple) {
      const auto position_star = thrust::get<0>(tuple);
      const auto x_star = position_star.x;
      return (x_star <= domain_begin); // True if oob-left
    });

    oob_left_count_  = oob_right_begin - oob_begin;
    oob_right_count_ = end - oob_right_begin;

    const int max_recv_per_side = static_cast<int>(particles.available()/2);

    receive_left_index_ = end - begin;
    receive_right_index_ = receive_left_index_ + max_recv_per_side;
    std::size_t send_left_index = oob_begin - begin;
    std::size_t send_right_index = oob_right_begin - begin;

//    std::cout<<"rank "<<comm_compute_.rank()<<" receive indices: "<<receive_left_index_<<", "<<receive_right_index_<<" send indices: "<<send_left_index<<", "<<send_right_index<<std::endl;

    requests_[0] = comm_compute_.i_recv(this->domain_to_left(), 0,
                                       &(particles.position_stars()[receive_left_index_]), max_recv_per_side, MPI_VEC_);
    requests_[1] = comm_compute_.i_recv(this->domain_to_left(), 1,
                                       &(particles.positions()[receive_left_index_]), max_recv_per_side, MPI_VEC_);
    requests_[2] = comm_compute_.i_recv(this->domain_to_left(), 2,
                                       &(particles.velocities()[receive_left_index_]), max_recv_per_side, MPI_VEC_);

    requests_[3] = comm_compute_.i_recv(this->domain_to_right(), 3,
                                       &(particles.position_stars()[receive_right_index_]), max_recv_per_side, MPI_VEC_);
    requests_[4] = comm_compute_.i_recv(this->domain_to_right(), 4,
                                       &(particles.positions()[receive_right_index_]), max_recv_per_side, MPI_VEC_);
    requests_[5] = comm_compute_.i_recv(this->domain_to_right(), 5,
                                       &(particles.velocities()[receive_right_index_]), max_recv_per_side, MPI_VEC_);

    requests_[6] = comm_compute_.i_send(this->domain_to_left(), 3,
                                       &(particles.position_stars()[send_left_index]), oob_left_count_, MPI_VEC_);
    requests_[7] = comm_compute_.i_send(this->domain_to_left(), 4,
                                       &(particles.positions()[send_left_index]), oob_left_count_, MPI_VEC_);
    requests_[8] = comm_compute_.i_send(this->domain_to_left(), 5,
                                       &(particles.velocities()[send_left_index]), oob_left_count_, MPI_VEC_);

    requests_[9]  = comm_compute_.i_send(this->domain_to_right(), 0,
                                        &(particles.position_stars()[send_right_index]), oob_right_count_, MPI_VEC_);
    requests_[10] = comm_compute_.i_send(this->domain_to_right(), 1,
                                        &(particles.positions()[send_right_index]), oob_right_count_, MPI_VEC_);
    requests_[11] = comm_compute_.i_send(this->domain_to_right(), 2,
                                        &(particles.velocities()[send_right_index]), oob_right_count_, MPI_VEC_);

//   std::cout<<"rank : "<<comm_compute_.rank()<<" sending "<<oob_left_count_<<" to rank "<<this->domain_to_left()<<" and "<<oob_right_count_<<" to rank "<<this->domain_to_right()<<std::endl;
  }

  /*! Finalize OOB sync
  */
  void finalize_oob_exchange(Particles<Real,Dim> & particles) {
    MPI_Status statuses[12];
    sim::mpi::wait_all(requests_, 12, statuses);

    // copy recieved left/right to correct position in particle array
    int received_left_count, received_right_count;
    MPI_Get_count(&statuses[0], MPI_VEC_, &received_left_count);
    MPI_Get_count(&statuses[3], MPI_VEC_, &received_right_count);
    const int sent_count = oob_left_count_ + oob_right_count_;

//    std::cout<<"rank "<<comm_compute_.rank()<<" sent "<<oob_left_count_<<" to rank "<<this->domain_to_left()<<" and "<<oob_right_count_<<" to rank "<<this->domain_to_right()<<std::endl;
//    std::cout<<"rank "<<comm_compute_.rank()<<" received "<<received_left_count<<" from "<<this->domain_to_left()<<" and "<<received_right_count<<" from "<<this->domain_to_right()<<std::endl;

    this->remove_resident_particles(particles, sent_count);

    this->add_resident_particles(particles,
                                 &particles.positions()[receive_left_index_],
                                 &particles.position_stars()[receive_left_index_],
                                 &particles.velocities()[receive_left_index_],
                                 received_left_count);

    this->add_resident_particles(particles,
                                 &particles.positions()[receive_right_index_],
                                 &particles.position_stars()[receive_right_index_],
                                 &particles.velocities()[receive_right_index_],
                                 received_right_count);

//    std::cout<<"rank "<<comm_compute_.rank()<<" resident count: "<<resident_count()<<" local count: "<<local_count()<<std::endl;
  }

  /*! Initiate syncronize of halo particles
      Partition edge particles.
      Send edge particles and receive halo particles from neighbor

      Edge particles will need to be accessed multiple times
      And so they are placed continously at the end of the array
   */
  void initiate_halo_exchange(Particles<Real,Dim> & particles) {
    // Zip iterator of pointers to particle quantities to update
    const auto begin = thrust::make_zip_iterator(thrust::make_tuple(particles.position_stars().data(),
                                                                    particles.positions().data(),
                                                                    particles.velocities().data() ));
    const auto end = begin + this->resident_count_;

    const auto edge_left = domain_.begin + edge_width_;
    const auto edge_right = domain_.end - edge_width_;

    // Move left/right edge particles to end of arrays
    auto edge_begin = sim::algorithms::partition(begin, end, [=] DEVICE_CALLABLE  (const Tuple& tuple) {
      const auto position_star = thrust::get<0>(tuple);
      const auto x_star = position_star.x;
      return (x_star >= edge_left && x_star <= edge_right ); // True if not edge
    });
    // Move right edge to end of array, arrays now {interior, edge-left, edge-right}
    auto edge_right_begin = sim::algorithms::partition(edge_begin, end, [=] DEVICE_CALLABLE  (const Tuple& tuple) {
      const auto position_star = thrust::get<0>(tuple);
      const auto x_star = position_star.x;
      return (x_star <= edge_left); // True if edge-left
    });

    edge_left_count_ = edge_right_begin - edge_begin;
    edge_right_count_ = end - edge_right_begin;

    const int max_recv_per_side = static_cast<int>(particles.available()/2);

    receive_left_index_ = this->resident_count_;
    receive_right_index_ = receive_left_index_ + max_recv_per_side;
    const std::size_t send_left_index = edge_begin - begin;
    const std::size_t send_right_index = edge_right_begin - begin;

    requests_[0] = comm_compute_.i_recv(this->domain_to_left(), 0,
                                       &particles.position_stars()[receive_left_index_], max_recv_per_side, MPI_VEC_);
    requests_[1] = comm_compute_.i_recv(this->domain_to_left(), 1,
                                       &particles.positions()[receive_left_index_], max_recv_per_side, MPI_VEC_);
    requests_[2] = comm_compute_.i_recv(this->domain_to_left(), 2,
                                       &particles.velocities()[receive_left_index_], max_recv_per_side, MPI_VEC_);

    requests_[3] = comm_compute_.i_recv(this->domain_to_right(), 3,
                                       &particles.position_stars()[receive_right_index_], max_recv_per_side, MPI_VEC_);
    requests_[4] = comm_compute_.i_recv(this->domain_to_right(), 4,
                                       &particles.positions()[receive_right_index_], max_recv_per_side, MPI_VEC_);
    requests_[5] = comm_compute_.i_recv(this->domain_to_right(), 5,
                                       &particles.velocities()[receive_right_index_], max_recv_per_side, MPI_VEC_);

    requests_[6] = comm_compute_.i_send(this->domain_to_left(), 3,
                                       &particles.position_stars()[send_left_index], edge_left_count_, MPI_VEC_);
    requests_[7] = comm_compute_.i_send(this->domain_to_left(), 4,
                                       &particles.positions()[send_left_index], edge_left_count_, MPI_VEC_);
    requests_[8] = comm_compute_.i_send(this->domain_to_left(), 5,
                                       &particles.velocities()[send_left_index], edge_left_count_, MPI_VEC_);

    requests_[9]  = comm_compute_.i_send(this->domain_to_right(), 0,
                                        &particles.position_stars()[send_right_index], edge_right_count_, MPI_VEC_);
    requests_[10] = comm_compute_.i_send(this->domain_to_right(), 1,
                                        &particles.positions()[send_right_index], edge_right_count_, MPI_VEC_);
    requests_[11] = comm_compute_.i_send(this->domain_to_right(), 2,
                                        &particles.velocities()[send_right_index], edge_right_count_, MPI_VEC_);
}

  /*! Finalize halo sync
   */
  void finalize_halo_exchange(Particles<Real,Dim> & particles) {
    MPI_Status statuses[12];
    sim::mpi::wait_all(requests_, 12, statuses);

    // copy recieved left/right to correct position in particle array

    int received_left_count, received_right_count;//, sent_left_count, sent_right_count;
    MPI_Get_count(&statuses[0], MPI_VEC_, &received_left_count);
    MPI_Get_count(&statuses[3], MPI_VEC_, &received_right_count);

    // Left halo doesn't need to be copied but does need to incriment particle counts
    this->add_halo_particles_left(particles,
                             &particles.positions()[receive_left_index_],
                             &particles.position_stars()[receive_left_index_],
                             &particles.velocities()[receive_left_index_],
                             received_left_count);

    this->add_halo_particles_right(particles,
                             &particles.positions()[receive_right_index_],
                             &particles.position_stars()[receive_right_index_],
                             &particles.velocities()[receive_right_index_],
                             received_right_count);
  }

public:
  /*! Update the render process with particle coordinates
   * @param particles Particles to sync to renderer
   */
  void sync_to_renderer(const Particles<Real,Dim> & particles) {
    // Gather number of particle coordinates that will be sent to render process
    const std::size_t particle_count = resident_count_;

    comm_world_.gather(&particle_count, sim::mpi::get_mpi_size_t(), 0);

    // Gather particle coordinates on render process
    comm_world_.gatherv(particles.positions().data(), particle_count, MPI_VEC_, 0);
  }

  /*! Receive updated parameters from render node
   * @param parameters in which to update from render node copy
   */
  void sync_from_renderer(Parameters<Real,Dim> & parameters) {
    comm_world_.broadcast(&parameters, MPI_PARAMETERS_, 0);
  }

};
}

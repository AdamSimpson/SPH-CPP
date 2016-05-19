#pragma once

//#include <boost/mpi.hpp>
#include <utility>
#include <stdexcept>
#include <string>
#include "vec.h"
#include "utility_math.h"
#include "particles.h"
#include "parameters.h"
#include "thrust/execution_policy.h"
#include <thrust/iterator/zip_iterator.h>
#include <thrust/partition.h>
#include "parameters.h"
//#include "boost_mpi_optimizations.h"
#include "mpi++.h"

/***
  The distributor is responsible for all domain-to-domain communication as
  well as maintaning the particle array arangement described below.
  Although it seems slightly odd the idea is that the distributor is the
  only class that's aware the simulation is distributed.

  Particle arrays are arranged as such: { interior, edge_left, edge_right, halo_left, halo_right }

  interior: particles which can be fully processed without halo information
  edge: particles within the smoothing radius of domain left/right boundary
  resident: interor + edge particles(non halo particles)
  halo: left/right edge particles from neighboring domains
  local: resident + halo particles

  index spans are defined for each region such that [begin,end) defines the indices
  of the specified region in the particle arrays. An array_span
  is not appropriate as a particle is not defined by a single pointer but by a SoA
***/

template<typename Real, Dimension Dim>
class Distributor {
public:
  /**
    Distributor Constructor: Don't use member references!
    Distributor must be constructed before any other members
    due to MPI_Init() call.

    Requires Initilization step after construction of member references

    compute_comm is split with color value 1 to differentiate compute processes from I/O, viz processes
  **/

  typedef thrust::tuple< const Vec<Real,Dim>&, const Vec<Real,Dim>&, const Vec<Real,Dim>& > Tuple;
  typedef thrust::zip_iterator<Tuple> ZippedTuple;

  Distributor() :
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

  ~Distributor()                             = default;
  Distributor(const Distributor&)            = delete;
  Distributor& operator=(const Distributor&) = delete;
  Distributor(Distributor&&) noexcept        = delete;
  Distributor& operator=(Distributor&&)      = delete;

  /**
    @brief Handle initilization requiring constructed member references
  **/
  void initilize_fluid(Particles<Real,Dim> & particles,
                       const Parameters<Real,Dim> & parameters) {
    this->set_domain_bounds(parameters.initial_fluid(),
                            parameters.boundary());
    edge_width_ = 1.2*parameters.smoothing_radius();
    this->distribute_initial_fluid(parameters.initial_fluid(),
                                   particles,
                                   parameters.particle_rest_spacing());
  }

  /**
    @brief Set domain bounds based upon equal spacing in AABB
  **/
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

  // Checks for balanced number of particles in each domain
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

    std::cout<<"bounds for rank: "<<comm_compute_.rank()<<" "<<domain_.begin<<", "<<domain_.end<<std::endl;
  }

  /**
     @return Return true if last rank in domain
  **/
  bool is_last_domain() const {
    return comm_compute_.rank() == comm_compute_.size()-1;
  }

  /**
    @return Return true if first rank in domain
  **/
  bool is_first_domain() const {
    return comm_compute_.rank() == 0;
  }

  /**
    @return Return rank of domain to the left or MPI_PROC_NULL
  **/
  int domain_to_left() const {
    int left = MPI_PROC_NULL;
    if(comm_compute_.rank() > 0)
      left = comm_compute_.rank() - 1;

    return left;
  }

  /**
    @brief Return rank of domain to the right or MPI_PROC_NULL
  **/
  int domain_to_right() const {
    int right = MPI_PROC_NULL;
    if(comm_compute_.rank() != comm_compute_.size()-1)
      right = comm_compute_.rank() + 1;

    return right;
  }

  /**
    @brief getter for compute integer rank
  **/
  int compute_rank() const {
    return comm_compute_.rank();
  }

  /**
     @return local particle span
   **/
  IndexSpan local_span() const {
    return IndexSpan{0, this->halo_span().end};
  }

  /**
     @return resident particle span
  **/
  IndexSpan resident_span() const {
    return IndexSpan{0, resident_count_};
  }

  /**
    @return number of resident particles
  **/
  std::size_t resident_count() const {
    return resident_count_;
  }

  /**
    @return count of edge particles
  **/
  std::size_t edge_count() const {
    return edge_left_count_ + edge_right_count_;
  }

  /**
    @return span of edge indices
  **/
  IndexSpan edge_span() const {
    return IndexSpan(resident_count_ - edge_count(), resident_count_);
  }

  /**
    @return count of edge particles
  **/
  std::size_t halo_count() const {
    return halo_count_left_ + halo_count_right_;
  }

  /**
    @return span of halo indices
  **/
  IndexSpan halo_span() const {
    return IndexSpan(this->resident_span().end, this->resident_span().end + halo_count());
  }

  /**
    @return interior span
  **/
  IndexSpan interior_span() const {
    return IndexSpan{0,this->resident_count_ - edge_count()};
  }

  /**
    @return count of interior particles
  **/
  std::size_t interior_count() const {
    return this->interior_span().end - this->interior_span().begin;
  }

  std::size_t local_count() const {
    return this->local_span().end - this->local_span().begin;
  }

  /**
    @return number of global resident particles
  **/
  std::size_t global_resident_count() const {
    std::size_t global_count = 0;
    std::size_t local_count = this->resident_count();
    comm_compute_.all_reduce(&local_count, &global_count, MPI_UINT64_T, MPI_SUM);
    return global_count;
  }

  /**
    Construct water volume spread across multiple domains
  **/
  void distribute_initial_fluid(const AABB<Real,Dim>& global_fluid,
                                Particles<Real,Dim> & particles,
                                Real particle_rest_spacing) {
    // Create a bounding box with length dimension that coincides with particle spacing
    // |--|--|--|
    // -o--o--o-
    Real spacing = particle_rest_spacing;
    AABB<Real,Dim> local_fluid{global_fluid};
    size_t global_particle_count_x = floor(global_fluid.length()/spacing);

    // Append particles on last rank if they can't be evenly distributed
    int particle_count_x = global_particle_count_x / comm_compute_.size();

    // Base length of fluid for each node, with the exception of the last node
    Real base_fluid_length = particle_count_x * spacing;

    if(this->is_last_domain()) {
      int remaining = global_particle_count_x - (particle_count_x*comm_compute_.size());
      particle_count_x += remaining;
    }

    if(particle_count_x < 4)
      throw std::runtime_error("Less than four particles in x dimension!");

    // Length of fluid for current process
    Real local_fluid_length = (particle_count_x) * spacing;

    local_fluid.min.x = global_fluid.min.x + base_fluid_length*comm_compute_.rank();

    // small delta added to ensure no round off error
    local_fluid.max.x = local_fluid.min.x + local_fluid_length + spacing/10.0;

    // Fill AABB with particles
    resident_count_ = particles.construct_initial_fluid(local_fluid);
  }

  /**
    Sync domains: transfer out of bounds particles and update halo
  **/
  void domain_sync(Particles<Real,Dim> & particles){
    // Incoming OOB particles will be appended so we remove old halo particles first
    this->remove_halo_particles(particles);

    this->initiate_oob_exchange(particles);
    this->finalize_oob_exchange(particles);

    this->initiate_halo_exchange(particles);
    this->finalize_halo_exchange(particles);
  }

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

  void finalize_sync_halo_scalar() {
    MPI_Status statuses[4];
    sim::mpi::wait_all(requests_, 4, statuses);
  }

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

  void finalize_sync_halo_vec() {
    MPI_Status statuses[4];
    sim::mpi::wait_all(requests_, 4, statuses);
  }

private:
  const sim::mpi::Environment environment_;
  const sim::mpi::Communicator comm_world_;          /**< world communicator **/
  const sim::mpi::Communicator comm_compute_;  /**< compute subset of simulation **/
  Vec<Real,2>  domain_;                              /** x coordinate of domain domain range **/
  Real edge_width_;

  std::size_t resident_count_;
  std::size_t edge_left_count_;
  std::size_t edge_right_count_;
  std::size_t halo_count_left_;
  std::size_t halo_count_right_;
  std::size_t receive_left_index_;
  std::size_t receive_right_index_;
  std::size_t oob_left_count_;
  std::size_t oob_right_count_;

  MPI_Request requests_[12];

  MPI_Datatype MPI_VEC_;
  MPI_Datatype MPI_PARAMETERS_;

  /**
    Invalidates halo particles
  **/
  void remove_halo_particles(Particles<Real,Dim> & particles) {
    particles.remove(this->halo_count());
    halo_count_left_ = 0;
    halo_count_right_ = 0;
  }

  void remove_resident_particles(Particles<Real,Dim> & particles,
                                 std::size_t count) {
    particles.remove(count);
    resident_count_ -= count;
  }

  void add_resident_particles(Particles<Real,Dim> & particles,
                              const Vec<Real,Dim>* new_positions,
                              const Vec<Real,Dim>* new_position_stars,
                              const Vec<Real,Dim>* new_velocities,
                              std::size_t count) {
    particles.add(new_positions, new_position_stars, new_velocities, count);
    resident_count_ += count;
  }

  void add_halo_particles_left(Particles<Real,Dim> & particles,
                          const Vec<Real,Dim>* new_positions,
                          const Vec<Real,Dim>* new_position_stars,
                          const Vec<Real,Dim>* new_velocities,
                          std::size_t count) {
    particles.add(new_positions, new_position_stars, new_velocities, count);
    halo_count_left_ += count;
  }

  void add_halo_particles_right(Particles<Real,Dim> & particles,
                          const Vec<Real,Dim>* new_positions,
                          const Vec<Real,Dim>* new_position_stars,
                          const Vec<Real,Dim>* new_velocities,
                          std::size_t count) {
    particles.add(new_positions, new_position_stars, new_velocities, count);
    halo_count_right_ += count;
  }

  /**
    Send particles that have left domain boundary to correct domain
  **/
  void initiate_oob_exchange(Particles<Real,Dim> & particles) {
    // Zip iterator of pointers to particle quantities to update
    const auto begin = thrust::make_zip_iterator(thrust::make_tuple(particles.position_stars().data(),
                                                                    particles.positions().data(),
                                                                    particles.velocities().data()));
    const auto end = begin + this->resident_count_;

    // Move oob-left/right particles to end of arrays
    auto oob_begin = thrust::partition(thrust::device, begin, end, [=] (const Tuple& tuple) {
      const auto position_star = thrust::get<0>(tuple);
      const auto x_star = position_star.x;
      return (x_star >= domain_.begin && x_star <= domain_.end); // True if not OOB
    });
    // Move oob-right to end of array, arrays now {staying,oob-left,oob-right}
    auto oob_right_begin = thrust::partition(thrust::device, oob_begin, end, [=] (const Tuple& tuple) {
      const auto position_star = thrust::get<0>(tuple);
      const auto x_star = position_star.x;
      return (x_star <= domain_.begin); // True if oob-left
    });

    oob_left_count_  = oob_right_begin - oob_begin;
    oob_right_count_ = end - oob_right_begin;

    const int max_recv_per_side = static_cast<int>(particles.available()/2);

    receive_left_index_ = end - begin;
    receive_right_index_ = receive_left_index_ + max_recv_per_side;
    std::size_t send_left_index = oob_begin - begin;
    std::size_t send_right_index = oob_right_begin - begin;

    std::cout<<"rank "<<comm_compute_.rank()<<" receive indices: "<<receive_left_index_<<", "<<receive_right_index_<<" send indices: "<<send_left_index<<", "<<send_right_index<<std::endl;

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

   std::cout<<"rank : "<<comm_compute_.rank()<<" sending "<<oob_left_count_<<" to rank "<<this->domain_to_left()<<" and "<<oob_right_count_<<" to rank "<<this->domain_to_right()<<std::endl;

  }

  void finalize_oob_exchange(Particles<Real,Dim> & particles) {
    MPI_Status statuses[12];
    sim::mpi::wait_all(requests_, 12, statuses);

    // copy recieved left/right to correct position in particle array
    int received_left_count, received_right_count;
    MPI_Get_count(&statuses[0], MPI_VEC_, &received_left_count);
    MPI_Get_count(&statuses[3], MPI_VEC_, &received_right_count);
    const int sent_count = oob_left_count_ + oob_right_count_;

    std::cout<<"rank "<<comm_compute_.rank()<<" sent "<<oob_left_count_<<" to rank "<<this->domain_to_left()<<" and "<<oob_right_count_<<" to rank "<<this->domain_to_right()<<std::endl;
    std::cout<<"rank "<<comm_compute_.rank()<<" received "<<received_left_count<<" from "<<this->domain_to_left()<<" and "<<received_right_count<<" from "<<this->domain_to_right()<<std::endl;

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

    std::cout<<"rank "<<comm_compute_.rank()<<" resident count: "<<resident_count()<<" local count: "<<local_count()<<std::endl;
  }
  /**
    Partition edge particles.
    Send edge particles and receive halo particles from neighbor

    Edge particles will need to be accessed multiple times
    And so they are placed continously at the end of the array
  **/
  void initiate_halo_exchange(Particles<Real,Dim> & particles) {
    // Zip iterator of pointers to particle quantities to update
    const auto begin = thrust::make_zip_iterator(thrust::make_tuple(particles.position_stars().data(),
                                                                    particles.positions().data(),
                                                                    particles.velocities().data() ));
    const auto end = begin + this->resident_count_;

    const auto edge_left = domain_.begin + edge_width_;
    const auto edge_right = domain_.end - edge_width_;

    // Move left/right edge particles to end of arrays
    auto edge_begin = thrust::partition(thrust::device, begin, end, [=] (const Tuple& tuple) {
      const auto position_star = thrust::get<0>(tuple);
      const auto x_star = position_star.x;
      return (x_star >= edge_left && x_star <= edge_right ); // True if not edge
    });
    // Move right edge to end of array, arrays now {interior, edge-left, edge-right}
    auto edge_right_begin = thrust::partition(thrust::device, edge_begin, end, [=] (const Tuple& tuple) {
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

  /**
    Update the render node with particle coordinates
  **/
// @todo refuculate this public nonsense
public:
  void sync_to_renderer(const Particles<Real,Dim> & particles) {
    // Gather number of particle coordinates that will be sent to render process
    const std::size_t particle_count = resident_count_;

    comm_world_.gather(&particle_count, sim::mpi::get_mpi_size_t(), 0);

    // Gather particle coordinates on render process
    comm_world_.gatherv(particles.positions().data(), particle_count, MPI_VEC_, 0);
  }

  // Receive updated parameters from render node
  void sync_from_renderer(Parameters<Real,Dim> & parameters) {
    comm_world_.broadcast(&parameters, MPI_PARAMETERS_, 0);
  }

};

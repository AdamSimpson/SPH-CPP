#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>
#include <type_traits>
#include "dimension.h"
#include "vec.h"
#include "parameters.h"
#include "aabb.h"
#include "execution_mode.h"

extern "C" {
  #define OMPI_SKIP_MPICXX // Disable C++ bindings
  #include "mpi.h"
}

/**
  lightweight MPI wrapper specialized for SPH
**/
namespace sim { namespace mpi {
  inline void check_return(int return_code) {
    if(return_code != MPI_SUCCESS) {
      char error_string[MPI_MAX_ERROR_STRING];
      int error_length;
      MPI_Error_string(return_code, error_string, &error_length);
      std::string throw_string="MPI Error: ";
      throw_string.append(error_string);
      throw std::runtime_error(throw_string);
    }
  }

  template <typename T>
  MPI_Datatype get_mpi_type() {
    throw std::runtime_error("MPI ERROR: get_mpi_type not supported");
  }

  template<>
  MPI_Datatype get_mpi_type<float>() {
    return MPI_FLOAT;
  }
  template<>
  MPI_Datatype get_mpi_type<double>() {
    return MPI_DOUBLE;
  }
  template<>
  MPI_Datatype get_mpi_type<char>() {
    return MPI_CHAR;
    }

  MPI_Datatype get_mpi_size_t() {
    if(sizeof(std::size_t) == sizeof(uint32_t))
      return MPI_UINT32_T;
    else if(sizeof(std::size_t) == sizeof(uint64_t))
      return MPI_UINT64_T;
    else
      throw std::runtime_error("MPI Error: Unknown std::size_t conversion");
  }

  template <typename Real, Dimension Dim>
  void create_vec_type(MPI_Datatype& MPI_VEC) {
    typedef Vec<Real,Dim> Vec_type; // assert,offsetof macros dont do templates with comas
    MPI_Datatype types[Dim];
    MPI_Aint disps[Dim];
    int block_lengths[Dim];

    assert(std::is_pod<Vec_type>::value);

    for(int i=0; i<Dim; ++i) {
      types[i] = get_mpi_type<Real>();
      block_lengths[i] = 1;
      disps[i] = ((MPI_Aint) &(((Vec_type*)0)->data_[i])); // offsetof macro issues so manully get offset
//      disps[i] = offsetof( Vec_type, data_[i]);
    }

    int err;
    err = MPI_Type_create_struct(Dim, block_lengths, disps, types, &MPI_VEC);
    check_return(err);
    err = MPI_Type_commit(&MPI_VEC);
    check_return(err);
  }

  template <typename Real, Dimension Dim>
  void create_aabb_type(const MPI_Datatype& MPI_VEC, MPI_Datatype& MPI_AABB) {
    typedef AABB<Real,Dim> AABB_type;
    MPI_Datatype types[2];
    MPI_Aint disps[2];
    int block_lengths[2];

    assert(std::is_pod<AABB_type>::value);

    types[0] = MPI_VEC;
    types[1] = MPI_VEC;
    block_lengths[0] = 1;
    block_lengths[1] = 1;
    disps[0] = offsetof( AABB_type, min );
    disps[1] = offsetof( AABB_type, max );

    int err;
    err = MPI_Type_create_struct(2, block_lengths, disps, types, &MPI_AABB);
    check_return(err);
    err = MPI_Type_commit(&MPI_AABB);
    check_return(err);
  }

  template <typename Real, Dimension Dim>
  void create_parameters_type(const MPI_Datatype MPI_VEC,
                              const MPI_Datatype MPI_AABB,
                              MPI_Datatype& MPI_PARAMETERS) {
    typedef Parameters<Real,Dim> Parameters_type;
    const int member_count = 23;
    MPI_Datatype types[member_count];
    MPI_Aint disps[member_count];
    int block_lengths[member_count];

    assert(std::is_pod<Parameters_type>::value);

    MPI_Datatype MPI_SIZE_T = get_mpi_size_t();

    types[0] = MPI_SIZE_T;
    block_lengths[0] = 1;
    disps[0] = offsetof(Parameters_type, max_particles_local_);

    types[1] = MPI_SIZE_T;
    block_lengths[1] = 1;
    disps[1] = offsetof(Parameters_type, initial_global_particle_count_);

    types[2] = MPI_SIZE_T;
    block_lengths[2] = 1;
    disps[2] = offsetof(Parameters_type, solve_step_count_);

    types[3] = get_mpi_type<Real>();
    block_lengths[3] = 1;
    disps[3] = offsetof(Parameters_type, particle_rest_spacing_);

    types[4] = get_mpi_type<Real>();
    block_lengths[4] = 1;
    disps[4] = offsetof(Parameters_type, particle_radius_);

    types[5] = get_mpi_type<Real>();
    block_lengths[5] = 1;
    disps[5] = offsetof(Parameters_type, smoothing_radius_);

    types[6] = get_mpi_type<Real>();
    block_lengths[6] = 1;
    disps[6] = offsetof(Parameters_type, rest_density_);

    types[7] = get_mpi_type<Real>();
    block_lengths[7] = 1;
    disps[7] = offsetof(Parameters_type, rest_mass_);

    types[8] = get_mpi_type<Real>();
    block_lengths[8] = 1;
    disps[8] = offsetof(Parameters_type, gravity_);

    types[9] = get_mpi_type<Real>();
    block_lengths[9] = 1;
    disps[9] = offsetof(Parameters_type, gamma_);

    types[10] = get_mpi_type<Real>();
    block_lengths[10] = 1;
    disps[10] = offsetof(Parameters_type, lambda_epsilon_);

    types[11] = get_mpi_type<Real>();
    block_lengths[11] = 1;
    disps[11] = offsetof(Parameters_type, k_stiff_);

    types[12] = get_mpi_type<Real>();
    block_lengths[12] = 1;
    disps[12] = offsetof(Parameters_type, visc_c_);

    types[13] = get_mpi_type<Real>();
    block_lengths[13] = 1;
    disps[13] = offsetof(Parameters_type, time_step_);

    types[14] = get_mpi_type<Real>();
    block_lengths[14] = 1;
    disps[14] = offsetof(Parameters_type, max_speed_);

    types[15] = get_mpi_type<Real>();
    block_lengths[15] = 1;
    disps[15] = offsetof(Parameters_type, vorticity_coef_);

    types[16] = MPI_AABB;
    block_lengths[16] = 1;
    disps[16] = offsetof(Parameters_type, boundary_);

    types[17] = MPI_AABB;
    block_lengths[17] = 1;
    disps[17] = offsetof(Parameters_type, initial_fluid_);

    types[18] = MPI_INT;
    block_lengths[18] = 1;
    disps[18] = offsetof(Parameters_type, simulation_mode_);

    types[19] = MPI_INT;
    block_lengths[19] = 1;
    disps[19] = offsetof(Parameters_type, execution_mode_);

    types[20] = MPI_VEC;
    block_lengths[20] = 1;
    disps[20] = offsetof(Parameters_type, emitter_center_);

    types[21] = MPI_VEC;
    block_lengths[21] = 1;
    disps[21] = offsetof(Parameters_type, emitter_velocity_);

    types[22] = MPI_VEC;
    block_lengths[22] = 1;
    disps[22] = offsetof(Parameters_type, mover_center_);

    int err;
    err = MPI_Type_create_struct(member_count, block_lengths, disps, types, &MPI_PARAMETERS);
    check_return(err);
    err = MPI_Type_commit(&MPI_PARAMETERS);
    check_return(err);
  }

  template <typename Real, Dimension Dim>
  void create_mpi_types(MPI_Datatype& MPI_VEC,
                        MPI_Datatype& MPI_PARAMETERS) {
    MPI_Datatype MPI_AABB;

    create_vec_type<Real,Dim>(MPI_VEC);
    create_aabb_type<Real,Dim>(MPI_VEC, MPI_AABB);
    create_parameters_type<Real,Dim>(MPI_VEC, MPI_AABB, MPI_PARAMETERS);
  }

  void free_mpi_types(MPI_Datatype& MPI_VEC,
                      MPI_Datatype& MPI_PARAMETERS) {

    MPI_Type_free(&MPI_VEC);
    MPI_Type_free(&MPI_PARAMETERS);
  }

  class Environment {
  public:
    Environment(int *argc, char ***argv) {
      int err = MPI_Init(argc, argv);
      check_return(err);
    }

    Environment() {
      int err = MPI_Init(NULL, NULL);
      check_return(err);
    }

    ~Environment() {
      MPI_Finalize();
    }
  };

  class Communicator {
  public:
    Communicator(): comm_{MPI_COMM_WORLD} {}

    Communicator(int color) {
      int world_rank;
      int err = MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
      check_return(err);

      err = MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &comm_);

      check_return(err);
    }

    ~Communicator() {
      if(comm_ != MPI_COMM_WORLD)
        MPI_Comm_free(&comm_);
    }

    // Disable copy constructors
    Communicator(Communicator const &) = delete;
    void operator=(Communicator &t) = delete;

    int rank() const {
      int rank;
      int err = MPI_Comm_rank(comm_, &rank);
      check_return(err);
      return rank;
    }

    int size() const {
      int size;
      int err = MPI_Comm_size(comm_, &size);
      check_return(err);
      return size;
    }

    void barrier() const {
      int err = MPI_Barrier(comm_);
      check_return(err);
    }

    MPI_Comm MPI_comm() const {
      return comm_;
    }

    MPI_Request i_send(int dest, int tag, const void *buf, int count, MPI_Datatype data_type) const {
      MPI_Request request;
      int err = MPI_Isend(buf, count, data_type, dest, tag, comm_, &request);
      check_return(err);

      return request;
    }

    MPI_Request i_recv(int source, int tag, void *buf, int count, MPI_Datatype data_type) const {
      MPI_Request request;
      int err = MPI_Irecv(buf, count, data_type, source, tag, comm_, &request);
      check_return(err);

      return request;
    }

    void all_reduce(const void *send_buf, void *recv_buf, MPI_Datatype data_type, MPI_Op op) const {
      const int count = 1;
      int err = MPI_Allreduce(send_buf, recv_buf, count, data_type, op, comm_);
      check_return(err);
    }

    // Send side of gather with 1 send element per rank
    void gather(const void *send_buf, MPI_Datatype type, int root) const {
      assert(root == 0);

      int send_count = 1;

      int err = MPI_Gather(send_buf, send_count, type, NULL, 0, type, root, comm_);
      check_return(err);
    }

    // Receive side of gather with 1 send element per rank(excluding root which sends 0 elements)
    void gather(void *recv_buff, MPI_Datatype type) const {
      int recv_count = 1;

      int err = MPI_Gather(MPI_IN_PLACE, 0, type, recv_buff, recv_count, type, 0, comm_);
      check_return(err);
    }

    // Send side of gatherv
    void gatherv(const void *send_buf, int send_count, MPI_Datatype type, int root) const {
      int err = MPI_Gatherv(send_buf, send_count, type, NULL, NULL, NULL, type, root, comm_);
      check_return(err);
    }

    // Recv side of gatherv, recv_counts includes
    void gatherv(void *recv_buf, std::vector<int>& recv_counts, MPI_Datatype type) const {
      int displs[recv_counts.size()];

      // gatherv is done in_place so rank 0 send count must be 0
      assert(recv_counts[0] == 0);
      assert(rank() == 0);
      assert(recv_counts.size() == size());

      int displacement = 0;
      for(int i=0; i<size(); i++) {
        displs[i] = displacement;
        displacement += recv_counts[i];
      }

      int err = MPI_Gatherv(MPI_IN_PLACE, 0, type, recv_buf, recv_counts.data(), displs, type, rank(), comm_);
      check_return(err);
    }

    void broadcast(void *buffer, MPI_Datatype datatype, int root) const {
      const int count = 1;
      int err = MPI_Bcast(buffer, count, datatype, root, comm_);
      check_return(err);
    }

    void send_recv(const void *sendbuf, int destination, void *recvbuf, int source,
                   int count, MPI_Datatype datatype) const {
      int tag = 7;
      int err = MPI_Sendrecv(sendbuf, count, datatype, destination, tag, recvbuf, count,
                             datatype, source, tag, comm_, MPI_STATUS_IGNORE);
      check_return(err);
    }

  private:
    MPI_Comm comm_;
  };

  void wait_all(MPI_Request* requests, int count, MPI_Status* statuses) {
    int err = MPI_Waitall(count, requests, statuses);
    check_return(err);
  }

} }

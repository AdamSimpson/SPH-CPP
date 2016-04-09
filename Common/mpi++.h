#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include "dimension.h"
#include "vec.h"
#include "parameters.h"
#include "aabb.h"

extern "C" {
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

    assert(is_standard_layout<Vec_type>::value);

    for(int i=0; i<Dim; ++i) {
      types[i] = get_mpi_type<Real>();
      block_lengths[i] = 1;
      disps[i] = ((std::size_t) &(((Vec_type*)0)->data[i])); // offsetof macro issues so manully get offset
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

    assert(is_standard_layout<AABB_type>);

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
  void create_parameters_type(const MPI_Datatype MPI_AABB,
                              MPI_Datatype& MPI_PARAMETERS) {
    typedef Parameters<Real,Dim> Parameters_type;
    const int member_count = 19;
    MPI_Datatype types[member_count];
    MPI_Aint disps[member_count];
    int block_lengths[member_count];

    assert(is_standard_layout<Parameters_type>);

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

    int err;
    err = MPI_Type_create_struct(Dim, block_lengths, disps, types, &MPI_PARAMETERS);
    check_return(err);
    err = MPI_Type_commit(&MPI_PARAMETERS);
    check_return(err);
  }

  template <typename Real, Dimension Dim>
  void create_mpi_types(MPI_Datatype& MPI_VEC,
                        MPI_Datatype& MPI_AABB,
                        MPI_Datatype& MPI_PARAMETERS) {
    create_vec_type<Real,Dim>(MPI_VEC);
    create_aabb_type<Real,Dim>(MPI_VEC, MPI_AABB);
    create_parameters_type<Real,Dim>(MPI_AABB, MPI_PARAMETERS);
  }

  class Environment {
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
      int err = MPI_Comm_split(MPI_COMM_WORLD, color, this->rank(), &comm_);
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
      MPI_Comm_rank(comm_, &rank);
      return rank;
    }

    int size() const {
      int size;
      MPI_Comm_size(comm_, &size);
      return size;
    }

    MPI_Comm MPI_comm() const {
      return comm_;
    }

    void i_send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
           MPI_Request *request) const {
      int err = MPI_Isend(buf, count, datatype, dest, tag, comm_, request);
      check_return(err);
    }

    void i_recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Request *request) {
      int err = MPI_Irecv(buf, count, datatype, source, tag, comm_, request);
      check_return(err);
    }

  private:
    MPI_Comm comm_;
  };

  void wait_all(int count, MPI_Request* requests, MPI_Status* statuses) {
    int err = MPI_Waitall(count, requests, statuses);
    check_return(err);
  }

} }

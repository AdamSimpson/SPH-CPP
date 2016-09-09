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
#define OMPI_SKIP_MPICXX  // Disable C++ bindings OpenMPI
#define MPICH_SKIP_MPICXX // Disable C++ bindings MPICH
#include "mpi.h"
}

/*! lightweight MPI wrapper specialized for SPH
 */
namespace sim {
  namespace mpi {

    /*! Check MPI return codes
     * @param return_code return code from MPI function
     * @return If return_code indicates success
     * Check MPI return code and throw runtime_error exception is return value is not MPI_SUCCESS
     */
    void check_return(int return_code);

    /*! Get MPI size_t type
     * @return MPI_Datatype corespending to system std::size_t type
     */
    MPI_Datatype get_mpi_size_t();

    /*! Block until all provided MPI Requests are finalized
     * @param requests pointer to MPI_Request array
     * @param count    number of MPI_Requests to wait on
     * @param statuses Pointer to MPI_Status array
     */
    void wait_all(MPI_Request *requests, int count, MPI_Status *statuses);

    /*! Generic template function, specilizations provided below
     * @return static assert should always fail at comepile time
     */
    template<typename T>
    MPI_Datatype get_mpi_type() {
      static_assert(sizeof(T) != sizeof(T), "MPI ERROR: get_mpi_type not supported");
    }

    /*! float to MPI_FLOAT
     * @return MPI_FLOAT
     */
    template<>
    inline MPI_Datatype get_mpi_type<float>() {
      return MPI_FLOAT;
    }

    /*! double to MPI_DOUBLE
     * @return MPI_DOUBLE
     */
    template<>
    inline MPI_Datatype get_mpi_type<double>() {
      return MPI_DOUBLE;
    }

    /*! char to MPI_CHAR
     * @return MPI_CHAR
     */
    template<>
    inline MPI_Datatype get_mpi_type<char>() {
      return MPI_CHAR;
    }

    /*! std::size_t to MPI type
     * @return MPI_Datatype coresponding to std::size_t
     */
    template<>
    inline MPI_Datatype get_mpi_type<std::size_t>() {
      return get_mpi_size_t();
    }

    /*! Create and commit MPI_VEC type allowing Vec<Real,Dim> types
     * @param MPI_VEC reference to MPI_Datatype to be created
     */
    template<typename Real, Dimension Dim>
    void create_vec_type(MPI_Datatype &MPI_VEC) {
      typedef Vec<Real, Dim> Vec_type; // assert,offsetof macros dont do templates with comas
      MPI_Datatype types[Dim];
      MPI_Aint disps[Dim];
      int block_lengths[Dim];

      assert(std::is_pod<Vec_type>::value);

      for (int i = 0; i < Dim; ++i) {
        types[i] = get_mpi_type<Real>();
        block_lengths[i] = 1;
        disps[i] = ((MPI_Aint) &(((Vec_type *) 0)->data_[i])); // offsetof macro issues so manully get offset
//      disps[i] = offsetof( Vec_type, data_[i]);
      }

      int err;
      err = MPI_Type_create_struct(Dim, block_lengths, disps, types, &MPI_VEC);
      check_return(err);
      err = MPI_Type_commit(&MPI_VEC);
      check_return(err);
    }

    /*! Create and commit MPI_AABB type allowing AABB<Real,Dim> types
     * @param MPI_VEC const reference to created MPI_VEC type
     * @param MPI_AABB reference to MPI_Datatype to be created
     */
    template<typename Real, Dimension Dim>
    void create_aabb_type(const MPI_Datatype &MPI_VEC, MPI_Datatype &MPI_AABB) {
      typedef AABB<Real, Dim> AABB_type;
      MPI_Datatype types[2];
      MPI_Aint disps[2];
      int block_lengths[2];

      assert(std::is_pod<AABB_type>::value);

      types[0] = MPI_VEC;
      types[1] = MPI_VEC;
      block_lengths[0] = 1;
      block_lengths[1] = 1;
      disps[0] = offsetof(AABB_type, min);
      disps[1] = offsetof(AABB_type, max);

      int err;
      err = MPI_Type_create_struct(2, block_lengths, disps, types, &MPI_AABB);
      check_return(err);
      err = MPI_Type_commit(&MPI_AABB);
      check_return(err);
    }

    /*! Create and commit MPI_PARAMETERS type allowing Parameters<Real,Dim> types
     * @param MPI_VEC const reference to created MPI_VEC type
     * @param MPI_AABB  const reference to created AABB type
     * @param MPI_PARAMETERS reference to MPI_Datatype to be created
     */
    template<typename Real, Dimension Dim>
    void create_parameters_type(const MPI_Datatype MPI_VEC,
                                const MPI_Datatype MPI_AABB,
                                MPI_Datatype &MPI_PARAMETERS) {
      typedef Parameters<Real, Dim> Parameters_type;
      const int member_count = 24;
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
      disps[6] = offsetof(Parameters_type, neighbor_bin_spacing_);

      types[7] = get_mpi_type<Real>();
      block_lengths[7] = 1;
      disps[7] = offsetof(Parameters_type, rest_density_);

      types[8] = get_mpi_type<Real>();
      block_lengths[8] = 1;
      disps[8] = offsetof(Parameters_type, rest_mass_);

      types[9] = get_mpi_type<Real>();
      block_lengths[9] = 1;
      disps[9] = offsetof(Parameters_type, gravity_);

      types[10] = get_mpi_type<Real>();
      block_lengths[10] = 1;
      disps[10] = offsetof(Parameters_type, gamma_);

      types[11] = get_mpi_type<Real>();
      block_lengths[11] = 1;
      disps[11] = offsetof(Parameters_type, lambda_epsilon_);

      types[12] = get_mpi_type<Real>();
      block_lengths[12] = 1;
      disps[12] = offsetof(Parameters_type, k_stiff_);

      types[13] = get_mpi_type<Real>();
      block_lengths[13] = 1;
      disps[13] = offsetof(Parameters_type, visc_c_);

      types[14] = get_mpi_type<Real>();
      block_lengths[14] = 1;
      disps[14] = offsetof(Parameters_type, time_step_);

      types[15] = get_mpi_type<Real>();
      block_lengths[15] = 1;
      disps[15] = offsetof(Parameters_type, max_speed_);

      types[16] = get_mpi_type<Real>();
      block_lengths[16] = 1;
      disps[16] = offsetof(Parameters_type, vorticity_coef_);

      types[17] = MPI_AABB;
      block_lengths[17] = 1;
      disps[17] = offsetof(Parameters_type, boundary_);

      types[18] = MPI_AABB;
      block_lengths[18] = 1;
      disps[18] = offsetof(Parameters_type, initial_fluid_);

      types[19] = MPI_INT;
      block_lengths[19] = 1;
      disps[19] = offsetof(Parameters_type, simulation_mode_);

      types[20] = MPI_INT;
      block_lengths[20] = 1;
      disps[20] = offsetof(Parameters_type, execution_mode_);

      types[21] = MPI_VEC;
      block_lengths[21] = 1;
      disps[21] = offsetof(Parameters_type, emitter_center_);

      types[22] = MPI_VEC;
      block_lengths[22] = 1;
      disps[22] = offsetof(Parameters_type, emitter_velocity_);

      types[23] = MPI_VEC;
      block_lengths[23] = 1;
      disps[23] = offsetof(Parameters_type, mover_center_);

      int err;
      err = MPI_Type_create_struct(member_count, block_lengths, disps, types, &MPI_PARAMETERS);
      check_return(err);
      err = MPI_Type_commit(&MPI_PARAMETERS);
      check_return(err);
    }

    /*! Create and commit MPI types used for SPH
     * @param MPI_VEC reference to MPI_Datatype to be created
     * @param MPI_PARAMETERS reference to MPI_Datatype to be created
     */
    template<typename Real, Dimension Dim>
    void create_mpi_types(MPI_Datatype &MPI_VEC,
                          MPI_Datatype &MPI_PARAMETERS) {
      MPI_Datatype MPI_AABB;

      create_vec_type<Real, Dim>(MPI_VEC);
      create_aabb_type<Real, Dim>(MPI_VEC, MPI_AABB);
      create_parameters_type<Real, Dim>(MPI_VEC, MPI_AABB, MPI_PARAMETERS);
    }

    void free_mpi_types(MPI_Datatype &MPI_VEC,
                        MPI_Datatype &MPI_PARAMETERS);

    /*! Lightweight wrapper around MPI_Init and MPI_Finalize
     */
    class Environment {
    public:
      /*! Construct Environment and initialize MPI
       * @param argc pointer to argc
       * @param argv pointer to argv
       * @return constructed Environment
       */
      Environment(int *argc, char ***argv);

      Environment() = delete;

      /*! Construct Environment and optinally initialize MPI
       * @param Manage_mpi bool to state if Environment should call MPI_Init and Finalize
       * @return Constructed Environment and initialize MPI if manage_mpi == true
       */
      Environment(bool manage_mpi = true);

      /*! Destruct envrionment and finalize MPI if manage_mpi == false
       */
      ~Environment();

    private:
      const bool manage_mpi_;
    };

    class Communicator {
    public:
      /*! Construct communicator
       * @return Constructed communicator with comm_ == MPI_COMM_WORLD
       */
      Communicator();

      /*! Construct communicator
       * All ranks must call Communicator with this signature if any do
       * @param color of rank in new communicator
       * @return Constructed communicator of specified color
       */
      Communicator(int color);

      /*! Destruct communicator
       *
       */
      ~Communicator();

      // Disable copy constructors
      Communicator(Communicator const &) = delete;
      void operator=(Communicator &t) = delete;

      /*! MPI rank getter
       * @return processes rank within comm_
       */
      int rank() const;

      /*! MPI size getter
       * @return size of comm_
       */
      int size() const;

      /*! MPI barrier
       * block until all ranks in comm_ reach barrier
       */
      void barrier() const;

      /*! raw MPI_Comm getter
       * @return MPI_Comm associated with this
       */
      MPI_Comm MPI_comm() const;


      /*! MPI_Isend wrapper
       * @param dest      destination message rank
       * @param tag       message tag
       * @param buf       pointer to send buffer
       * @param count     number of elements to send
       * @param data_type MPI datatype to send
       * @return MPI_Request object
       */
      MPI_Request i_send(int dest, int tag, const void *buf, int count, MPI_Datatype data_type) const;

      /*! MPI_Irecv wrapper
       * @param source    source message rank
       * @param tag       message tag
       * @param buf       pointer to receiver buffer
       * @param count     maximum number of elements to receieve
       * @param data_type MPI datatype to receieve
       * @return MPI_Request object
       */
      MPI_Request i_recv(int source, int tag, void *buf, int count, MPI_Datatype data_type) const;

      /*! MPI_Allreduce wrapper
       * @param send_buf pointer to send buffer
       * @param recv_buf pointer to receive buffer
       * @param data_type MPI datatype to send and receieve
       * @param op MPI_Op type
       */
      void all_reduce(const void *send_buf, void *recv_buf, MPI_Datatype data_type, MPI_Op op) const;

      /*! Single element MPI_Gatherv wrapper
       * Send side of gather that assumes 1 element is sent per rank, excluding root rank with sends 0 elements
       * @param send_buf pointer to send buffer
       * @param type     MPI datatype of send buffer element
       * @param root     rank of reduction root
       */
      void gather(const void *send_buf, MPI_Datatype type, int root) const;

      /*! Single element MPI_Gatherv wrapper
       *  Receive side of gather that assumes 1 element is sent per rank, excluding root rank with sends 0 elements
       * @param recv_buff pointer to receieve buffer
       * @param type      MPI datatype of receieved elements
       */
      void gather(void *recv_buff, MPI_Datatype type) const;

      /*! Multiple element MPI_Gatherv wrapper
       * @param send_buf   pointer to send buffer
       * @param send_count number of elements each rank will send
       * @param type       MPI datatype of send buffer elements
       * @param root       rank of reduction root
       */
      void gatherv(const void *send_buf, int send_count, MPI_Datatype type, int root) const;

      /*! Multiple element MPI_Gatherv wrapper
       * @param recv_buf    pointer to receieve buffer
       * @param recv_counts vector reference containing number of elements to receieve from each rank
       * @param type        MPI datatype of receieved elements
       */
      void gatherv(void *recv_buf, std::vector<int> &recv_counts, MPI_Datatype type) const;

      /*! MPI_Broadcast wrapper
       * @param buffer   pointer to send buffer on non root, receieve buffer on root
       * @param datatype MPI datatype of send/receieve elements
       * @param root     rank of root
       */
      void broadcast(void *buffer, MPI_Datatype datatype, int root) const;

      /*! MPI_Sendrecv wrapper
       * @param sendbuf     pointer to send buffer
       * @param destination rank of destination
       * @param recvbuf     pointer to receieve buffer
       * @param source      rank of source
       * @param count       number of elements to send/receieve
       * @param datatype    MPI datatype of sent/receieved elements
       * @param datatype    MPI datatype of sent/receieved elements
       */
      void send_recv(const void *sendbuf, int destination, void *recvbuf, int source,
                     int count, MPI_Datatype datatype) const;

      /*! MPI_Scan with MPI_Op == MPI_SUM wrapper
       *
       * @param send_buf A reference to the send element
       * @return The returned sum of the scan
       */
      template<typename T>
      T scan_sum(T &send_buf) const {
        T recv_buf;
        MPI_Scan(&send_buf, &recv_buf, 1, get_mpi_type<T>(), MPI_SUM, comm_);
        return recv_buf;
      }

    private:
      MPI_Comm comm_;
    };

  }
}

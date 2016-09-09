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

#include "mpi++.h"

namespace sim {
  namespace mpi {

    void check_return(int return_code) {
      if (return_code != MPI_SUCCESS) {
        char error_string[MPI_MAX_ERROR_STRING];
        int error_length;
        MPI_Error_string(return_code, error_string, &error_length);
        std::string throw_string = "MPI Error: ";
        throw_string.append(error_string);
        throw std::runtime_error(throw_string);
      }
    }

    MPI_Datatype get_mpi_size_t() {
      if (sizeof(std::size_t) == sizeof(uint32_t))
        return MPI_UINT32_T;
      else if (sizeof(std::size_t) == sizeof(uint64_t))
        return MPI_UINT64_T;
      else
        throw std::runtime_error("MPI Error: Unknown std::size_t conversion");
    }

    void free_mpi_types(MPI_Datatype& MPI_VEC,
                        MPI_Datatype& MPI_PARAMETERS) {

      MPI_Type_free(&MPI_VEC);
      MPI_Type_free(&MPI_PARAMETERS);
    }

    void wait_all(MPI_Request *requests, int count, MPI_Status *statuses) {
      int err = MPI_Waitall(count, requests, statuses);
      check_return(err);
    }

    ///
    /// Environment implimentation
    ///
    Environment::Environment(int *argc, char ***argv): manage_mpi_{true} {
      int err = MPI_Init(argc, argv);
      check_return(err);
    }

    // Testing requires environment to be constructed/destroyed multiple times in application
    // In this case MPI_Init/MPI_Finalize must be called outside of this mpi wrapper
    Environment::Environment(bool manage_mpi): manage_mpi_{manage_mpi} {
      if(manage_mpi_) {
        int err = MPI_Init(NULL, NULL);
        check_return(err);
      }
    }

    Environment::~Environment() {
      if(manage_mpi_) {
        MPI_Finalize();
      }
    }

    ///
    /// Communicator implimentation
    ///
    Communicator::Communicator() : comm_{MPI_COMM_WORLD} {}

    Communicator::Communicator(int color) {
      int world_rank;
      int err = MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
      check_return(err);

      err = MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &comm_);

      check_return(err);
    }

    Communicator::~Communicator() {
      if (comm_ != MPI_COMM_WORLD)
        MPI_Comm_free(&comm_);
    }

    int Communicator::rank() const {
      int rank;
      int err = MPI_Comm_rank(comm_, &rank);
      check_return(err);
      return rank;
    }

    int Communicator::size() const {
      int size;
      int err = MPI_Comm_size(comm_, &size);
      check_return(err);
      return size;
    }

    void Communicator::barrier() const {
      int err = MPI_Barrier(comm_);
      check_return(err);
    }

    MPI_Comm Communicator::MPI_comm() const {
      return comm_;
    }

    MPI_Request Communicator::i_send(int dest, int tag, const void *buf, int count, MPI_Datatype data_type) const {
      MPI_Request request;
      int err = MPI_Isend(buf, count, data_type, dest, tag, comm_, &request);
      check_return(err);

      return request;
    }

    MPI_Request Communicator::i_recv(int source, int tag, void *buf, int count, MPI_Datatype data_type) const {
      MPI_Request request;
      int err = MPI_Irecv(buf, count, data_type, source, tag, comm_, &request);
      check_return(err);

      return request;
    }

    void Communicator::all_reduce(const void *send_buf, void *recv_buf, MPI_Datatype data_type, MPI_Op op) const {
      const int count = 1;
      int err = MPI_Allreduce(send_buf, recv_buf, count, data_type, op, comm_);
      check_return(err);
    }

    // Send side of gather with 1 send element per rank
    void Communicator::gather(const void *send_buf, MPI_Datatype type, int root) const {
      assert(root == 0);

      int send_count = 1;

      int err = MPI_Gather(send_buf, send_count, type, NULL, 0, type, root, comm_);
      check_return(err);
    }

    // Receive side of gather with 1 send element per rank(excluding root which sends 0 elements)
    void Communicator::gather(void *recv_buff, MPI_Datatype type) const {
      int recv_count = 1;

      int err = MPI_Gather(MPI_IN_PLACE, 0, type, recv_buff, recv_count, type, 0, comm_);
      check_return(err);
    }

    // Send side of gatherv
    void Communicator::gatherv(const void *send_buf, int send_count, MPI_Datatype type, int root) const {
      int err = MPI_Gatherv(send_buf, send_count, type, NULL, NULL, NULL, type, root, comm_);
      check_return(err);
    }

    // Recv side of gatherv, recv_counts includes
    void Communicator::gatherv(void *recv_buf, std::vector<int> &recv_counts, MPI_Datatype type) const {
      int displs[recv_counts.size()];

      // gatherv is done in_place so rank 0 send count must be 0
      assert(recv_counts[0] == 0);
      assert(rank() == 0);
      assert(recv_counts.size() == size());

      int displacement = 0;
      for (int i = 0; i < size(); i++) {
        displs[i] = displacement;
        displacement += recv_counts[i];
      }

      int err = MPI_Gatherv(MPI_IN_PLACE, 0, type, recv_buf, recv_counts.data(), displs, type, rank(), comm_);
      check_return(err);
    }

    void Communicator::broadcast(void *buffer, MPI_Datatype datatype, int root) const {
      const int count = 1;
      int err = MPI_Bcast(buffer, count, datatype, root, comm_);
      check_return(err);
    }

    void Communicator::send_recv(const void *sendbuf, int destination, void *recvbuf, int source,
                                 int count, MPI_Datatype datatype) const {
      int tag = 7;
      int err = MPI_Sendrecv(sendbuf, count, datatype, destination, tag, recvbuf, count,
                             datatype, source, tag, comm_, MPI_STATUS_IGNORE);
      check_return(err);
    }
  }
}
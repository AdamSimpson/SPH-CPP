#include "dimension.h"
#include "parameters.h"
#include <boost/mpi.hpp>
#include <stdexcept>
#include "vec.h"
#include <vector>
#include "array.h"
#include "boost_mpi_optimizations.h"

int main(int argc, char**argv) {
  boost::mpi::environment env;
  boost::mpi::communicator comm_world;

  int rank = comm_world.rank();

  if(rank == 1) {
    sim::Array<Vec<float,3>> send_data{10};
    for(int i=0; i<10; i++)
      send_data.push_back(Vec<float,3>{6.0, 7.0, 8.9});

    comm_world.isend(0, 7, &(send_data[0]), 10);
  }
  else if(rank == 0){
    sim::Array<Vec<float,3>> recv_data{10};

    auto request = comm_world.irecv(1, 7, &(recv_data[0]), 10);
    auto status = request.wait();
    std::cout<<"request error: "<<status.error()<<std::endl;
  }
  else {
    sim::Array<Vec<float,3>> send_data{1};
    auto request = comm_world.isend(MPI_PROC_NULL, 7, &send_data[0], 10);
    auto status = request.wait();
    std::cout<<"request error: "<<status.error()<<std::endl;
  }

/*
  if(rank == 1) {
    std::vector< Vec<float,3> > send_data;
    for(int i=0; i<10; i++)
      send_data.push_back(Vec<float,3>{6.0, 7.0, 8.9});

    boost::mpi::gatherv(comm_world, send_data, 0);

    for(auto el : send_data)
      std::cout<<"sent: "<<el<<std::endl;

  }
  else {
    Vec<float,3> dummy_input;

    std::vector<int> counts{0, 10};

    std::vector< Vec<float,3> > storage;
    storage.resize(10);

    boost::mpi::gatherv(comm_world, &dummy_input, 0, storage.data(), counts, 0);

    for(auto el : storage)
      std::cout<<"received: "<<el<<std::endl;

  }

  if(rank == 1) {
    Parameters<float,3> recv;
    boost::mpi::broadcast(comm_world, recv, 0);

    std::cout<<"Gravity on rank 1: "<<recv.gravity()<<std::endl;
  }
  else {
    Parameters<float,3> send;
    send.gravity_ = 9.8;
    boost::mpi::broadcast(comm_world, send, 0);
  }


  if(rank == 1) {
    Parameters<float,3> recv;
    boost::mpi::broadcast(comm_world, recv, 0);

    std::cout<<"mode on recv: "<<recv.simulation_mode_<<std::endl;
  }
  else {
    Parameters<float,3> send;
    send.simulation_mode_ = Parameters<float,3>::Mode::EXIT;
    std::cout<<"mode on send: "<< send.simulation_mode_<<std::endl;

    boost::mpi::broadcast(comm_world, send, 0);
  }
*/
  return 0;
};

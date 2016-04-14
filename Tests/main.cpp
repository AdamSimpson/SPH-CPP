#include "dimension.h"
#include "parameters.h"
#include "vec.h"
#include <vector>
#include "array.h"
#include "mpi++.h"

int main(int argc, char**argv) {
  sim::mpi::Environment env;
  sim::mpi::Communicator comm_world;

  MPI_Datatype MPI_VEC, MPI_PARAMETERS;
  sim::mpi::create_mpi_types<float,3>(MPI_VEC, MPI_PARAMETERS);

  int rank = comm_world.rank();
/*
  if(rank == 1) {
    sim::Array<Vec<float,3>> send_data{10};
    for(int i=0; i<10; i++)
      send_data.push_back(Vec<float,3>{6.0, 7.0, 8.9});

    comm_world.i_send(0, 7, &(send_data[0]), 10);
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
*/
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
*/
  if(rank == 1) {
    Parameters<float,3> recv;
    comm_world.broadcast(&recv, MPI_PARAMETERS, 0);

    std::cout<<"Max local on rank 1: "<<recv.max_particles_local()<<std::endl;
    std::cout<<"initial global count on rank 1: "<<recv.initial_global_particle_count()<<std::endl;
    std::cout<<"Particle rest spacing on rank 1: "<<recv.particle_rest_spacing()<<std::endl;
    std::cout<<"Gravity on rank 1: "<<recv.gravity()<<std::endl;
  }
  else {
    Parameters<float,3> send;
    send.max_particles_local_ = 17;
    send.initial_global_particle_count_ = 7;
    send.particle_rest_spacing_ = 0.1;
    send.gravity_ = 9.8;
    comm_world.broadcast(&send, MPI_PARAMETERS, 0);
  }
/*
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
   if(rank == 1) {
    Vec<float,3> recv;
    comm_world.broadcast(&recv, MPI_VEC, 0);

    std::cout<<"vec on recv: "<<recv<<std::endl;
  }
  else {
    Vec<float,3> send{7.0};
    comm_world.broadcast(&send, MPI_VEC, 0);
  }

  return 0;
};

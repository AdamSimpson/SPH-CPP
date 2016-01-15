#pragma once

#include "dimension.h"
#include <boost/mpi.hpp>

template<Dimension Dim>
class Distributor {
public:

  /**
    Distributor handles the coupling between the compute processes and the render process
  **/
  Distributor() : environment_{false},
                  comm_render_{comm_world_.split(0)} {}

private:
  boost::mpi::environment environment_;
  boost::mpi::communicator comm_world_;
  boost::mpi::communicator comm_render_;
};

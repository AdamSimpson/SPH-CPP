#include "neighbors.h"
#include "device.h"

namespace sim {

// Iterator for range based for loops over neighbor indices
  DEVICE_CALLABLE
  const std::size_t *begin(const NeighborList &list) {
    return list.neighbor_indices;
  }

  DEVICE_CALLABLE
  const std::size_t *end(const NeighborList &list) {
    return list.neighbor_indices + list.count;
  }
}
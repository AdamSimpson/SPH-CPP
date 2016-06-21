#pragma once

namespace sim {
  enum ExecutionMode {
    GPU = (1 << 0),
    CPU = (1 << 1)
  };
}

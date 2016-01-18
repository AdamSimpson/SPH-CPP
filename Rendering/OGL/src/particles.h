#pragma once

#include "world.h"

class Particles {
public:
  Particles(const World& world): world_{world} {}
private:
  const World& world_;
};

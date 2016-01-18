#pragma once

#include "world.h"

class Particles {
public:
  Particles(const World& world);
  void create_buffers();
  void create_program();
private:
  const World& world_;

  GLuint program_;

  GLint position_location_;
  GLint color_location_;
  GLint sphere_radius_location_;

  GLint view_matrices_index_;
  GLint light_index_;

  GLuint vbo_;
  GLuint vao_;
};

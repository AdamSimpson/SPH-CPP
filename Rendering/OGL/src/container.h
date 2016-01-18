#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include "world.h"

class Container {
public:
  Container(const World& world);
  void draw();

private:
  GLuint program_;

  GLint position_location_;
  GLint normal_location_;
  GLint tex_coord_location_;

  // Uniforms
  GLuint color_location_;
  GLint matrices_index_;
  GLint light_index_;

  // Vertex objects
  GLuint vbo_;
  GLuint vao_;

  const World& world_;

  void create_buffers();
  void create_program();
  void create_vertices();
};

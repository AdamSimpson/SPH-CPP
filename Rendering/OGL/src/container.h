#pragma once

#include "drawable.h"
#include "aabb.h"
#include "GL/glew.h"
#include "glm/glm.hpp"

namespace sim {

class Container: public Drawable {
public:
  Container(const AABB<float, three_dimensional>& container_bounds);
  ~Container();
  void draw() const;

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

  void create_buffers();
  void destroy_buffers();
  void create_program();
  void destroy_program();
  void set_vertices(const AABB<float, three_dimensional>& bounds);
};
}

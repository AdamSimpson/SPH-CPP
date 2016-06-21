#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

namespace sim {

class Light {
public:
  static constexpr int binding_index = 1;

  Light();
  void init();
  void set_position(const glm::vec3& world_position);
  void set_position(const glm::vec4& world_position);
  void update(const glm::mat4& view_matrix);
private:
  GLuint light_UBO_;
  glm::vec4 world_position_;          // Position in world space
  glm::vec4 intensity_;
  glm::vec4 intensity_ambient_;
  float attenuation_;
};
}

#pragma once

#include "glm/glm.hpp"
#include "SFML/OpenGL.hpp"

struct Light {
  glm::vec4 position_world_space;
  glm::vec4 intensity;
  glm::vec4 intensity_ambient;
  float attenuation;
};

struct Camera {
  float max_degrees_rotate;
  float zoom_factor;
  glm::vec3 eye_positions;
  glm::vec3 eye_positions_default;
  glm::vec3 look_at_point;
};

class World {
public:
  World();

private:
  Camera camera_;
  Light light_;
  const int matrices_binding_index_;
  const int light_binding_index_;
  GLuint matricies_UBO_;
  GLuint light_UBO_;
};

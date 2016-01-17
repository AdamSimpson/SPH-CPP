#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

struct Light {
  glm::vec4 position;          // Position in world space
  glm::vec4 intensity;
  glm::vec4 intensity_ambient;
  float attenuation;
};

struct Camera {
  float max_degrees_rotate;
  float zoom_factor;
  glm::vec3 eye_position;     // camera position in world space
  glm::vec3 look_at_point;    // world space point camera is looking at
};

/**
  World maintains view matrix and lighting uniforms
**/
class World {
public:
  World(): matrices_binding_index_{0},
           light_binding_index_{1} {};

  /**
    Initialize view matrices and camera uniforms
  **/
  void init(float window_aspect_ratio);

  /**
    Update the view and light uniforms
  **/
  void update();

  int get_light_binding_index() {
    return light_binding_index_;
  }

  int get_matrices_binding_index() {
    return matrices_binding_index_;
  }

private:
  Camera camera_;
  Light light_;
  float aspect_ratio_;
  const int matrices_binding_index_;
  const int light_binding_index_;
  GLuint matricies_UBO_;
  GLuint light_UBO_;
};

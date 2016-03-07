#pragma once

#include "aabb.h"
#include "GL/glew.h"
#include "glm/glm.hpp"

struct Light {
  glm::vec4 position;          // Position in world space
  glm::vec4 intensity;
  glm::vec4 intensity_ambient;
  float attenuation;
};

struct Camera {
  float zoom_factor;
  float speed;
  float pitch;
  float yaw;
  glm::vec3 world_position;     // camera position in world space
  glm::vec3 relative_front;    //  forward point relative to camera
  glm::vec3 relative_up;
  glm::vec3 world_up;
};

/**
  World maintains view matrix and lighting uniforms
**/
class World {
public:
  World(float aspect_ratio, AABB<float, three_dimensional> world_boundary);

  /**
    Update the view matrices and light uniforms
  **/
  void update();

  int light_binding_index() const;
  int matrices_binding_index() const;

  void move_camera_forward();
  void move_camera_back();
  void move_camera_left();
  void move_camera_right();
  void handle_mouse(int x_rel, int y_rel);

private:
  Camera camera_;
  Light light_;
  float aspect_ratio_;
  AABB<float, three_dimensional> world_boundary_;
  const int matrices_binding_index_;
  const int light_binding_index_;
  GLuint matricies_UBO_;
  GLuint light_UBO_;

  bool first_mouse_move_;
};

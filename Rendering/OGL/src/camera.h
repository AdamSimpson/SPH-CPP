#pragma once

#include "user_input.h"
#include "vec.h"
#include "GL/glew.h"
#include "glm/glm.hpp"

class Camera {
public:
  static constexpr int binding_index = 0;

  Camera();
  void init();
  void set_position(glm::vec3 new_position);
  void update(float aspect_ratio);
  void process_input(const UserInput& user_input_);
  glm::mat4& view_matrix();

private:
  GLuint matricies_UBO_;
  float speed_;
  float pitch_;
  float yaw_;
  glm::mat4 view_matrix_;
  glm::vec3 world_position_;     // camera position in world space
  glm::vec3 relative_front_;    //  forward point relative to camera
  glm::vec3 relative_up_;
  glm::vec3 world_up_;

  void move_forward();
  void move_back();
  void move_left();
  void move_right();
  void handle_mouse(int x_rel, int y_rel);
};

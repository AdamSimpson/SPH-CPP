#pragma once

#include "user_input.h"
#include "vec.h"
#include "GL/glew.h"
#include "glm/glm.hpp"

namespace sim {
class Camera {
public:
  static constexpr int binding_index = 0;

  Camera();
  void init();
  void set_position(glm::vec3 new_position);
  void update(float aspect_ratio, float near_clip, float far_clip);
  void process_input(const UserInput& user_input_);
  void set_speed(float max_speed);
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

  void move_forward(float frame_time);
  void move_back(float frame_time);
  void move_left(float frame_time);
  void move_right(float frame_time);
  void handle_mouse(int x_rel, int y_rel);
};
}

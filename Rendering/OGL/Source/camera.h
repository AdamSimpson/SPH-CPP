/*
The MIT License (MIT)

Copyright (c) 2016 Adam Simpson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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

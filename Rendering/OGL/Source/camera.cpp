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

#include "camera.h"

#include "GL/glew.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/norm.hpp"

namespace sim {

Camera::Camera() {
  yaw_ = 0.0f;
  pitch_ = 0.0f;
  speed_ = 0.1f;
  world_position_ = {0.0f, 0.0f, 0.0f};
  relative_front_ = {0.0f, 0.0f, -1.0f};
  relative_up_    = {0.0f, 1.0f, 0.0f};
  world_up_       = {0.0f, 1.0f, 0.0f};
}

void Camera::init() {
  /**
    init view matrix uniform
  **/
  glGenBuffers(1, &matrices_UBO_);

  // Create and Allocate buffer storage
  glBindBuffer(GL_UNIFORM_BUFFER, matrices_UBO_);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_STREAM_DRAW);

  // Attach to binding index
  glBindBufferRange(GL_UNIFORM_BUFFER, Camera::binding_index, matrices_UBO_,
                    0, sizeof(glm::mat4) * 2);
  // Unbind
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::set_position(glm::vec3 new_position) {
  world_position_ = new_position;
}

void Camera::set_speed(float max_speed) {
  speed_ = max_speed;
}

void Camera::move_forward(float frame_time = 0.016f) {
  world_position_ += speed_ * frame_time * relative_front_;
}

void Camera::move_back(float frame_time = 0.016f) {
  world_position_ -= speed_ * frame_time * relative_front_;
}

void Camera::move_left(float frame_time = 0.016f) {
  world_position_ += speed_ * frame_time * glm::normalize(
                                             glm::cross(relative_front_,
                                                        -relative_up_));
}

void Camera::move_right(float frame_time = 0.016f) {
  world_position_ += speed_ * frame_time * glm::normalize(
                                             glm::cross(relative_front_,
                                                        relative_up_));
}

void Camera::handle_mouse(int x_rel, int y_rel) {
  float sensitivity = 0.005f;
  pitch_ -= y_rel * sensitivity;
  yaw_ += x_rel * sensitivity;

  pitch_ = Utility::clamp(pitch_, (float)-M_PI/2.01f, (float)M_PI/2.01f);

  glm::vec3 new_front = {std::cos(pitch_) * std::sin(yaw_),
                         std::sin(pitch_),
                        -std::cos(pitch_) * std::cos(yaw_)};
  relative_front_ = glm::normalize(new_front);

  // Calculate camera up vector
  // @todo check if this is correct...
//  glm::vec3 relative_right = glm::normalize(glm::cross(camera_.relative_front,
//                                                       camera_.world_up ));
//  camera_.relative_up = glm::normalize(glm::cross(relative_right,
//                                                  camera_.relative_front));
}

glm::mat4& Camera::view_matrix() {
  return this->view_matrix_;
}

void Camera::update(float aspect_ratio,
                    float near_clip,
                    float far_clip) {
  // Bind UBO
  glBindBuffer(GL_UNIFORM_BUFFER, matrices_UBO_);

  // Update view matrix
  view_matrix_ = glm::lookAt(world_position_,
                             world_position_ + relative_front_,
                             relative_up_);

  // Buffer matrix uniform data
  // 0.78 radians ~ 70 degrees
  glm::mat4 projection = glm::perspective(0.78f, aspect_ratio, 0.1f, 1000.0f);

  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view_matrix_));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

  // Unbind
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// Process User input
// @todo to keep FPS independent movement pass in frame time step
void Camera::process_input(const UserInput& user_input) {
  if(user_input.key_is_pressed("w"))
    this->move_forward();
  if(user_input.key_is_pressed("a"))
    this->move_left();
  if(user_input.key_is_pressed("s"))
    this->move_back();
  if(user_input.key_is_pressed("d"))
    this->move_right();

  this->handle_mouse(user_input.mouse_delta_x(), user_input.mouse_delta_y());

  // Simulate mouse motion with keyboard
  if(user_input.key_is_pressed("up"))
    this->handle_mouse(0.0f, -speed_);
  if(user_input.key_is_pressed("down"))
    this->handle_mouse(0.0f, speed_);
  if(user_input.key_is_pressed("left"))
    this->handle_mouse(-speed_, 0.0f);
  if(user_input.key_is_pressed("right"))
    this->handle_mouse(speed_, 0.0f);
}
}

#include "light.h"
#include <iostream>

#include "GL/glew.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/norm.hpp"

Light::Light() {
  world_position_ = {0.0f, 0.0f, 0.0f, 1.0};
  intensity_ = {0.8, 0.8, 0.8, 1.0};
  intensity_ambient_ = {0.4, 0.4, 0.4, 1.0};
  attenuation_ = 0.001;
}

void Light::init() {
  /**
    init light uniform
  **/

  glGenBuffers(1, &light_UBO_);

  // Create and Allocate buffer storage
  glBindBuffer(GL_UNIFORM_BUFFER, light_UBO_);
  glBufferData(GL_UNIFORM_BUFFER, 4*sizeof(glm::vec4) + sizeof(float),
               NULL, GL_STREAM_DRAW);

  // Attach to binding index
  glBindBufferRange(GL_UNIFORM_BUFFER, Light::binding_index, light_UBO_,
                    0, 4*sizeof(glm::vec4) + sizeof(float));

  // Unbind
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Light::set_position(const glm::vec3& world_position) {
  this->set_position(glm::vec4(world_position, 1.0f));
}

void Light::set_position(const glm::vec4& world_position) {
  world_position_ = world_position;
}

void Light::update(const glm::mat4& view_matrix) {
  // Create and Allocate buffer storage
  glBindBuffer(GL_UNIFORM_BUFFER, light_UBO_);

  glm::vec4 light_camera_space_position = view_matrix * world_position_;

  // Buffer light uniform data
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), glm::value_ptr(world_position_));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(light_camera_space_position));
  glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(intensity_));
  glBufferSubData(GL_UNIFORM_BUFFER, 3*sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(intensity_ambient_));
  glBufferSubData(GL_UNIFORM_BUFFER, 4*sizeof(glm::vec4), sizeof(float), &attenuation_);

  // Unbind
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

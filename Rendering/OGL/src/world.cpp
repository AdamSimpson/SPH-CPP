#include "world.h"

#include "GL/glew.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/norm.hpp"

World::World(float aspect_ratio): matrices_binding_index_{0},
                                  light_binding_index_{1}
{
  aspect_ratio_ = aspect_ratio;

  camera_.max_degrees_rotate   = 40.0f;
  camera_.zoom_factor          = 1.0f;
  camera_.eye_position         = {0.0f, 0.0f, 0.8f};
  camera_.look_at_point        = {0.0f, -0.5f, -0.7f};

  light_.position = {0.3, 0.5, -0.4, 1.0};
  light_.intensity = {0.8, 0.8, 0.8, 1.0};
  light_.intensity_ambient = {0.1, 0.1, 0.1, 1.0};
  light_.attenuation = 1.0;

  /**
    init view matrix uniform
  **/
  glGenBuffers(1, &matricies_UBO_);

  // Create and Allocate buffer storage
  glBindBuffer(GL_UNIFORM_BUFFER, matricies_UBO_);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_STREAM_DRAW);

  // Attach to binding index
  glBindBufferRange(GL_UNIFORM_BUFFER, matrices_binding_index_, matricies_UBO_,
                    0, sizeof(glm::mat4) * 2);

  /**
    init light uniform
  **/
  glGenBuffers(1, &light_UBO_);

  // Create and Allocate buffer storage
  glBindBuffer(GL_UNIFORM_BUFFER, light_UBO_);
  glBufferData(GL_UNIFORM_BUFFER, 4*sizeof(glm::vec4) + sizeof(float),
               NULL, GL_STREAM_DRAW);

  // Attach to binding index
  glBindBufferRange(GL_UNIFORM_BUFFER, light_binding_index_, light_UBO_,
                    0, 4*sizeof(glm::vec4) + sizeof(float));

  // Set uniform values
  this->update();

  // Unbind
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
};

void World::update() {
  // Bind UBO
  glBindBuffer(GL_UNIFORM_BUFFER, matricies_UBO_);

  // Update view matrix
  glm::mat4 view = glm::lookAt(camera_.eye_position,
                               camera_.look_at_point,
                               glm::vec3(0.0f, 1.0f, 0.0f)  // Up
                               );

  // Buffer view matrix uniform data
  // 1.22 radians ~ 70 degrees
  const float zoom_factor = 1.0;
  glm::mat4 projection = glm::perspective(zoom_factor*1.22f, aspect_ratio_, 0.7f, 10.0f);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));

  // Create and Allocate buffer storage
  glBindBuffer(GL_UNIFORM_BUFFER, light_UBO_);

  glm::vec4 light_camera_space_position = view * light_.position;

  // Buffer light uniform data
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), glm::value_ptr(light_.position));
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(light_camera_space_position));
  glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(light_.intensity));
  glBufferSubData(GL_UNIFORM_BUFFER, 3*sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(light_.intensity_ambient));
  glBufferSubData(GL_UNIFORM_BUFFER, 4*sizeof(glm::vec4), sizeof(float), &light_.attenuation);

  // Unbind
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

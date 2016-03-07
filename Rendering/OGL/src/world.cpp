#include "world.h"
#include "utility_math.h"
#include "iostream"
#include "aabb.h"

#include "GL/glew.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/norm.hpp"

// Initialize the camera and lights
// @todo calculate good starting values based on .ini values
World::World(float aspect_ratio,
             AABB<float, three_dimensional> world_boundary):
                                          aspect_ratio_{aspect_ratio},
                                          world_boundary_{world_boundary},
                                          matrices_binding_index_{0},
                                          light_binding_index_{1},
                                          first_mouse_move_{true}
{
  camera_.zoom_factor = 1.0f;
  camera_.yaw = 0.0;
  camera_.pitch = 0.0;
  camera_.speed = 0.01;
  camera_.world_position = { world_boundary_.length() * 0.5f,
                             world_boundary_.height() * 0.5f,
                             world_boundary_.depth() * 1.0f};
  camera_.relative_front = {0.0f, 0.0f, -1.0f};
  camera_.relative_up = {0.0f, 1.0f, 0.0f};
  camera_.world_up = {0.0f, 1.0f, 0.0f};

  light_.position = {world_boundary_.length() * 0.8f,
                     world_boundary_.height() * 1.5f,
                     world_boundary_.depth() * 1.5f,
                     1.0};
  light_.intensity = {0.8, 0.8, 0.8, 1.0};
  light_.intensity_ambient = {0.4, 0.4, 0.4, 1.0};
  light_.attenuation = 0.001;

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
  glm::mat4 view = glm::lookAt(camera_.world_position,
                               camera_.world_position + camera_.relative_front,
                               camera_.relative_up
                               );

  // Buffer matrix uniform data
  // 0.78 radians ~ 70 degrees
  const float zoom_factor = 1.0;
  // @todo set near and far cliping planes
  glm::mat4 projection = glm::infinitePerspective(zoom_factor*0.78f, aspect_ratio_, 0.1f);

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

int World::light_binding_index() const {
  return light_binding_index_;
}

int World::matrices_binding_index() const {
  return matrices_binding_index_;
}

void World::move_camera_forward() {
  camera_.world_position += camera_.speed * camera_.relative_front;
}

void World::move_camera_back() {
  camera_.world_position -= camera_.speed * camera_.relative_front;
}

void World::move_camera_left() {
  camera_.world_position += camera_.speed *
                            glm::normalize(
                              glm::cross(camera_.relative_front,
                                        -camera_.relative_up));
}

void World::move_camera_right() {
  camera_.world_position += camera_.speed *
                            glm::normalize(
                              glm::cross(camera_.relative_front,
                                         camera_.relative_up));
}

void World::handle_mouse(int x_rel, int y_rel) {
  if(first_mouse_move_) {
    first_mouse_move_ = false;
    return;
  }
  std::cout<<"x, y: "<<x_rel<<", "<<y_rel<<std::endl;

  float sensitivity = 0.005f;
  camera_.pitch -= y_rel * sensitivity;
  camera_.yaw += x_rel * sensitivity;

  std::cout<<"pitch, yaw: "<<camera_.pitch<<", "<<camera_.yaw<<std::endl;

  camera_.pitch = Utility::clamp(camera_.pitch, (float)-M_PI/2.01f, (float)M_PI/2.01f);

  glm::vec3 new_front = {std::cos(camera_.pitch) * std::sin(camera_.yaw),
                         std::sin(camera_.pitch),
                        -std::cos(camera_.pitch) * std::cos(camera_.yaw)};
  camera_.relative_front = glm::normalize(new_front);

  // Calculate camera up vector
  // @todo check if this is correct...
//  glm::vec3 relative_right = glm::normalize(glm::cross(camera_.relative_front,
//                                                       camera_.world_up ));
//  camera_.relative_up = glm::normalize(glm::cross(relative_right,
//                                                  camera_.relative_front));
}

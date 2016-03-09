#include "particles.h"
#include "GL/glew.h"
#include "ogl_utils.h"
#include "light.h"
#include "camera.h"
#include <iostream>

Particles::Particles() {
  this->create_buffers();
  this->create_program();

  // @todo reserve particle vector space
}

Particles::~Particles() {
  this->destroy_buffers();
  this->destroy_program();
}

void Particles::clear() {
  points_.clear();
  colors_.clear();
}

void Particles::draw() const {
  /// Update point and color buffers

  float particle_radius = 0.025f;
  // Set buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo_points_);
  // Orphan current buffer
  glBufferData(GL_ARRAY_BUFFER, points_.size()*sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  // Fill buffer
  glBufferData(GL_ARRAY_BUFFER, points_.size()*sizeof(GLfloat), points_.data(), GL_STREAM_DRAW);
  // Unbind buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Set buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo_colors_);
  // Orphan current buffer
  glBufferData(GL_ARRAY_BUFFER, colors_.size()*sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  // Fill buffer
  glBufferData(GL_ARRAY_BUFFER, colors_.size()*sizeof(GLfloat), colors_.data(), GL_STREAM_DRAW);
  // Unbind buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  /// Draw particles

  // Bind circle shader program
  glUseProgram(program_);

  // Set radius uniform
  glUniform1f(sphere_radius_location_, (GLfloat)particle_radius);
  // Set uniform binding
  glUniformBlockBinding(program_, view_matrices_index_, Camera::binding_index);
  glUniformBlockBinding(program_, light_index_, Light::binding_index);

  // Enable VAO
  glBindVertexArray(vao_);

  // Draw
  const std::size_t particle_count = points_.size()/3;
  glDrawArrays(GL_POINTS, 0, particle_count);

  // Unbind VAO and program
  glBindVertexArray(0);
  glUseProgram(0);
}

void Particles::create_buffers() {
  // Generate array object
  glGenVertexArrays(1, &vao_);
  // Generate vertex buffer
  glGenBuffers(1, &vbo_points_);
  glGenBuffers(1, &vbo_colors_);
}

void Particles::destroy_buffers() {
  glDeleteBuffers(1, &vao_);
  glDeleteBuffers(1, &vbo_points_);
  glDeleteBuffers(1, &vbo_colors_);
}

void Particles::create_program() {
  // Compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  Utility::compile_shader(vertex_shader, "../shaders/particles.vert");

  // Compile frag shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  Utility::compile_shader(fragment_shader, "../shaders/particles.frag");

  // Compile geometry shader
  GLuint geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
  Utility::compile_shader(geometry_shader, "../shaders/particles.geom");

  // Create shader program
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader);
  glAttachShader(program_, fragment_shader);
  glAttachShader(program_, geometry_shader);

  // Link and use program
  Utility::link_program(program_);

  glUseProgram(program_);

  position_location_ = glGetAttribLocation(program_, "position");
  color_location_ = glGetAttribLocation(program_, "color");
  sphere_radius_location_ = glGetUniformLocation(program_, "sphere_radius");
  view_matrices_index_ = glGetUniformBlockIndex(program_, "view_matrices");
  light_index_ = glGetUniformBlockIndex(program_, "light");

  // Setup VAO
  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_points_);
  glVertexAttribPointer(position_location_, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), 0);
  glEnableVertexAttribArray(position_location_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_colors_);
  glVertexAttribPointer(color_location_, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), 0);
  glEnableVertexAttribArray(color_location_);

  // Cleanup
  glDetachShader(program_, vertex_shader);
  glDetachShader(program_, fragment_shader);
  glDetachShader(program_, geometry_shader);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  glDeleteShader(geometry_shader);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

void Particles::destroy_program() {
  glDeleteProgram(program_);
}

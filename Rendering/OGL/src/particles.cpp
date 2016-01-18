#include "particles.h"
#include "ogl_utils.h"
#include "world.h"

Particles::Particles(const World& world): world_{world} {
  this->create_buffers();
  this->create_program();
}

void Particles::create_buffers() {
  // Generate array object
  glGenVertexArrays(1, &vao_);
  // Generate vertex buffer
  glGenBuffers(1, &vbo_);
}

void Particles::create_program() {
  // Compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  compile_shader(vertex_shader, "../shaders/particle.vert");

  // Compile frag shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  compile_shader(fragment_shader, "../shaders/particle.frag");

  // Compile geometry shader
  GLuint geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
  compile_shader(geometry_shader, "../shaders/particle.geom");

  // Create shader program
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader);
  glAttachShader(program_, fragment_shader);
  glAttachShader(program_, geometry_shader);

  // Link and use program
  glLinkProgram(program_);
  print_program_log(program_);
  glUseProgram(program_);

  position_location_ = glGetAttribLocation(program_, "position");
  color_location_ = glGetAttribLocation(program_, "color");
  sphere_radius_location_ = glGetUniformLocation(program_, "sphere_radius");
  view_matrices_index_ = glGetUniformBlockIndex(program_, "view_matrices");
  light_index_ = glGetUniformBlockIndex(program_, "light");

  // Setup VAO
  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glVertexAttribPointer(position_location_, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GL_FLOAT), 0);
  glEnableVertexAttribArray(position_location_);
  glVertexAttribPointer(color_location_, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GL_FLOAT),(void*)(3*sizeof(GL_FLOAT)));
  glEnableVertexAttribArray(color_location_);

  glBindVertexArray(0);
  glUseProgram(0);
}

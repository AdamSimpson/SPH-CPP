#include "container.h"
#include "aabb.h"
#include "ogl_utils.h"
#include "light.h"
#include "camera.h"

Container::Container(const AABB<float, three_dimensional>& container_bounds) {
  this->create_buffers();
  this->create_program();
  this->set_vertices(container_bounds);
}

Container::~Container() {
  this->destroy_buffers();
  this->destroy_program();
}

void Container::draw() const {
  // Setup program
  glUseProgram(program_);

  // set color uniform
  float color[] = {1.0, 1.0, 1.0, 1.0};
  glUniform4fv(color_location_, 1, color);

  // Set uniform binding
  glUniformBlockBinding(program_, matrices_index_, Camera::binding_index);
  glUniformBlockBinding(program_, light_index_, Light::binding_index);

  // Bind VAO
  glBindVertexArray(vao_);

  // Draw container
  glDrawArrays(GL_TRIANGLES, 0, 30);

  // Unbind buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Container::create_program() {
  // Compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  Utility::compile_shader(vertex_shader, "../shaders/container.vert");

  // Compile fragment shader
  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  Utility::compile_shader(fragment_shader, "../shaders/container.frag");

  // Create shader program
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader);
  glAttachShader(program_, fragment_shader);

  // Link  program
  Utility::link_program(program_);

  // Enable program
  glUseProgram(program_);

  // Get position attribute location
  position_location_ = glGetAttribLocation(program_, "position");
  // Get normal attribute location
  normal_location_ = glGetAttribLocation(program_, "normal");
  // Get tex_coord attribute location
  tex_coord_location_ = glGetAttribLocation(program_, "tex_coord");
  // Get color uniform location
  color_location_ = glGetUniformLocation(program_, "color");
  // Get global matrix index
  matrices_index_ = glGetUniformBlockIndex(program_, "view_matrices");
  // Get global light index
  light_index_ = glGetUniformBlockIndex(program_, "light");

  // Setup buffers
  glBindVertexArray(vao_);
  std::size_t vert_size = 8*sizeof(GL_FLOAT);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glVertexAttribPointer(position_location_, 3, GL_FLOAT, GL_FALSE, vert_size, 0);
  glEnableVertexAttribArray(position_location_);
  glVertexAttribPointer(normal_location_, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)(3*sizeof(GL_FLOAT)));
  glEnableVertexAttribArray(normal_location_);
  glVertexAttribPointer(tex_coord_location_, 2, GL_FLOAT, GL_FALSE, vert_size, (void*)(6*sizeof(GL_FLOAT)));
  glEnableVertexAttribArray(tex_coord_location_);

  // Clean up
  glDetachShader(program_, vertex_shader);
  glDetachShader(program_, fragment_shader);
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

void Container::destroy_program() {
  glDeleteProgram(program_);
}

void Container::create_buffers() {
    // Generate VAO
    glGenVertexArrays(1, &vao_);

    // Generate vertex buffer
    glGenBuffers(1, &vbo_);
}

void Container::destroy_buffers() {
  glDeleteBuffers(1, &vao_);
  glDeleteBuffers(1, &vbo_);
}

void Container::set_vertices(const AABB<float, three_dimensional>& bounds) {
  // vert x, y, z, normal x, y, z, tex_coord x, y
  float vertices[] = {
    // Floor
    bounds.min.x, bounds.min.y, bounds.max.z, 0.0, 1.0, 0.0, 0.0, 0.0,
    bounds.min.x, bounds.min.y, bounds.min.z, 0.0, 1.0, 0.0, 0.0, 1.0,
    bounds.max.x, bounds.min.y, bounds.max.z, 0.0, 1.0, 0.0, 1.0, 0.0,

    bounds.max.x, bounds.min.y, bounds.max.z, 0.0, 1.0, 0.0, 1.0, 0.0,
    bounds.min.x, bounds.min.y, bounds.min.z, 0.0, 1.0, 0.0, 0.0, 1.0,
    bounds.max.x, bounds.min.y, bounds.min.z, 0.0, 1.0, 0.0, 1.0, 1.0,

    //right wall
    bounds.max.x, bounds.min.y, bounds.max.z, -1.0, 0.0, 0.0, 1.0, 0.0,
    bounds.max.x, bounds.min.y, bounds.min.z, -1.0, 0.0, 0.0, 0.0, 0.0,
    bounds.max.x, bounds.max.y, bounds.max.z,  -1.0, 0.0, 0.0, 1.0, 1.0,

    bounds.max.x, bounds.max.y, bounds.min.z,  -1.0, 0.0, 0.0, 0.0, 1.0,
    bounds.max.x, bounds.max.y, bounds.max.z,  -1.0, 0.0, 0.0, 1.0, 1.0,
    bounds.max.x, bounds.min.y, bounds.min.z, -1.0, 0.0, 0.0, 0.0, 0.0,

    // Back
    bounds.min.x, bounds.max.y, bounds.min.z, 0.0, 0.0, 1.0, 0.0, 1.0,
    bounds.max.x, bounds.max.y, bounds.min.z, 0.0, 0.0, 1.0, 1.0, 1.0,
    bounds.max.x, bounds.min.y, bounds.min.z, 0.0, 0.0, 1.0, 1.0, 0.0,

    bounds.max.x, bounds.min.y, bounds.min.z, 0.0, 0.0, 1.0, 1.0, 0.0,
    bounds.min.x, bounds.min.y, bounds.min.z, 0.0, 0.0, 1.0, 0.0, 0.0,
    bounds.min.x, bounds.max.y, bounds.min.z, 0.0, 0.0, 1.0, 0.0, 1.0,

    // Front
    bounds.min.x, bounds.max.y, bounds.max.z, 0.0, 0.0, -1.0, 0.0, 1.0,
    bounds.max.x, bounds.min.y, bounds.max.z, 0.0, 0.0, -1.0, 1.0, 0.0,
    bounds.max.x, bounds.max.y, bounds.max.z, 0.0, 0.0, -1.0, 1.0, 1.0,

    bounds.max.x, bounds.min.y, bounds.max.z, 0.0, 0.0, -1.0, 1.0, 0.0,
    bounds.min.x, bounds.max.y, bounds.max.z, 0.0, 0.0, -1.0, 0.0, 1.0,
    bounds.min.x, bounds.min.y, bounds.max.z, 0.0, 0.0, -1.0, 0.0, 0.0,

     // Left wall
    bounds.min.x, bounds.min.y, bounds.max.z, 1.0, 0.0, 0.0, 1.0, 0.0,
    bounds.min.x, bounds.max.y, bounds.max.z,  1.0, 0.0, 0.0, 1.0, 1.0,
    bounds.min.x, bounds.min.y, bounds.min.z, 1.0, 0.0, 0.0, 0.0, 0.0,

    bounds.min.x, bounds.min.y, bounds.min.z, 1.0, 0.0, 0.0, 0.0, 0.0,
    bounds.min.x, bounds.max.y, bounds.max.z,  1.0, 0.0, 0.0, 1.0, 1.0,
    bounds.min.x, bounds.max.y, bounds.min.z,  1.0, 0.0, 0.0, 0.0, 1.0,
  };

  // Set buffer
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  // Fill buffer
  glBufferData(GL_ARRAY_BUFFER, 8*30*sizeof(GLfloat), vertices,
               GL_STATIC_DRAW);
}

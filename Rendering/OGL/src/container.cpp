#include "container.h"
#include "ogl_utils.h"

Container::Container(const World& world): world_{world}  {}

void Container::init() {
  // Compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  compile_shader(vertex_shader, "../shaders/container.vert");

  // Compile fragment shader
  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  compile_shader(frag_shader, "../shaders/container.frag");

  // Create shader program
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader);
  glAttachShader(program_, frag_shader);

  // Link  program
  glLinkProgram(program_);
  print_program_log(program_);

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
  glBindVertexArray(0);
  glUseProgram(0);
}

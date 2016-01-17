#include "GL/glew.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

inline void check_gl() {
  GLenum err = glGetError();
  if(err != GL_NO_ERROR) {
    throw std::runtime_error("OGL Failure: " + err);
  }
}

void print_program_log(GLuint program) {
  char log[1024];
  glGetProgramInfoLog(program, sizeof(log), NULL, log);
  std::cout<<"program log "<< program <<": "<< log << std::endl;
}

void print_shader_log(GLint shader) {
  char log[1024];
  glGetShaderInfoLog(shader,sizeof(log), NULL, log);
  std::cout<<"shader log "<< shader <<": "<< log <<std::endl;
}

void compile_shader(GLuint shader, const std::string& file_name) {
  // Read shader source from file_name
  std::ifstream file(file_name);
  if(!file)
    throw std::runtime_error("Could not open shader: " + file_name);

  auto string_stream = std::ostringstream{};
  string_stream << file.rdbuf();
  string_stream << '\0';
  auto shader_source = string_stream.str();

  // Compile shader
  const GLchar *gl_shader_source = shader_source.c_str();
  glShaderSource(shader, 1, &gl_shader_source, NULL);
  glCompileShader(shader);

  print_shader_log(shader);
}

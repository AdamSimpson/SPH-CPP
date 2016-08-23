#include "ogl_utils.h"
#include "SDL2/SDL.h"
#include "GL/glew.h"
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include "vec.h"

void Utility::check_gl() {
  GLenum err = glGetError();
  if(err != GL_NO_ERROR) {
    std::cout<<"check_gl failure: "<<err<<std::endl;
    throw std::runtime_error("OGL Failure: " + std::to_string(err));
  }
}

void Utility::print_program_log(GLuint program) {
  char log[1024];
  glGetProgramInfoLog(program, sizeof(log), NULL, log);
  std::cout<<"program log "<< program <<": "<< log << std::endl;
}

void Utility::print_shader_log(GLint shader) {
  char log[1024];
  glGetShaderInfoLog(shader,sizeof(log), NULL, log);
  std::cout<<"shader log "<< shader <<": "<< log <<std::endl;
}

void Utility::link_program(GLuint program) {
  glLinkProgram(program);
  GLint is_linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
  if(is_linked == GL_FALSE) {
    print_program_log(program);
    throw std::runtime_error("Could not link program: " + std::to_string(program));
  }
}

void Utility::compile_shader(GLuint shader, const std::string& file_name) {
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

  GLint is_compiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
  if(is_compiled == GL_FALSE) {
    print_shader_log(shader);
    throw std::runtime_error("Could not compile shader: " + file_name);
  }
}

Vec<float, three_dimensional> Utility::hsv_to_rgb(const Vec<float, three_dimensional>& HSV) {
  Vec<float, three_dimensional> RGB;
    float hue, saturation, value, hh, p, q, t, ff, r, g, b;
    long i;

    hue = HSV[0];
    saturation = HSV[1];
    value = HSV[2];

    hh = hue*360.0f;
    if(hh >= 360.0f)
	hh = 0.0f;
    hh /= 60.0f;
    i = (long)hh;
    ff = hh - i;
    p = value * (1.0f - saturation);
    q = value * (1.0f - (saturation * ff));
    t = value * (1.0f - (saturation * (1.0f - ff)));

    switch(i) {
        case 0:
	    r = value;
	    g = t;
	    b = p;
	    break;
	case 1:
	    r = q;
	    g = value;
	    b = p;
	    break;
	case 2:
	    r = p;
	    g = value;
	    b = t;
	    break;
	case 3:
	    r = p;
	    g = q;
	    b = value;
	    break;
        case 4:
	    r = t;
	    g = p;
	    b = value;
	    break;
	case 5:
	    r = value;
	    g = p;
	    b = q;
	    break;
	default:
	    r = value;
	    g = p;
	    b = q;
	    break;
    }

    RGB.r = r;
    RGB.g = g;
    RGB.b = b;

    return RGB;
}

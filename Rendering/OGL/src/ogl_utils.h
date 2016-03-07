#pragma once

#include <string>
#include "SDL2/SDL.h"
#include "GL/glew.h"

namespace Utility {

void check_gl();
void print_program_log(GLuint program);
void print_shader_log(GLint shader);
void link_program(GLuint program);
void compile_shader(GLuint shader, const std::string& file_name);

}

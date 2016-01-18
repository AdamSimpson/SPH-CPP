#pragma once
#include <string>

void check_gl();
void print_program_log(GLuint program);
void print_shader_log(GLint shader);
void compile_shader(GLuint shader, const std::string& file_name);

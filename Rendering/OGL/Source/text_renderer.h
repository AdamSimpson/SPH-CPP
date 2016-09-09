/*
The MIT License (MIT)

Copyright (c) 2016 Adam Simpson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "drawable.h"
#include <map>

namespace sim {

// Structure to hold cache glyph information
struct Character {
    GLuint tex;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

class TextRenderer {
public:
  TextRenderer(glm::vec2 screen_dims, int text_size); // Screen dimensions in pixels
  ~TextRenderer();
  void draw_text(std::string text,
                 GLfloat x, GLfloat y,
                 GLfloat scale, glm::vec3 color) const;
private:
  void create_buffers();
  void create_program();
  void create_character_map();

  FT_Library ft_;
  FT_Face ft_face_;
  std::map<GLchar, Character> characters_;
  glm::vec2 screen_dims_;
  const int text_size_;

  // Program and uniform locations
  GLint coord_location_;
  GLint tex_coord_location_;
  GLuint tex_location_;
  GLuint color_location_;
  GLuint projection_location_;

  GLuint program_;
  GLuint vbo_;
  GLuint vao_;
  GLint view_matrices_index_;
  GLint light_index_;
};
}

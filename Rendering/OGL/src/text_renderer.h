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

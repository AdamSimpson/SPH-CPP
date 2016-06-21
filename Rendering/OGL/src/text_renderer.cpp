#include "text_renderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "GL/glew.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ogl_utils.h"
#include <map>
#include <iostream>

namespace sim {

TextRenderer::TextRenderer(glm::vec2 screen_dims,
                           int text_size=24): screen_dims_{screen_dims},
                                                    text_size_{text_size}
 {
  // Initialize FreeType library
  if(FT_Init_FreeType(&ft_))
    throw std::runtime_error("Error initializing FreeType library\n");

  // Load font face
  if(FT_New_Face(ft_, "../src/DroidSerif-Regular.ttf", 0, &ft_face_))
    throw std::runtime_error("Error loading font face\n");

  // Set font pixel size
  FT_Set_Pixel_Sizes(ft_face_, 0, 24);

  // Setup OpenGL components
  this->create_program();
  this->create_buffers();
  this->create_character_map();
}

// @todo cleanup text
TextRenderer::~TextRenderer() {

}

void TextRenderer::create_buffers() {
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);

  // Setup VBO
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(coord_location_, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(coord_location_);
  glVertexAttribPointer(tex_coord_location_, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
  glEnableVertexAttribArray(tex_coord_location_);

  // Unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void TextRenderer::create_program() {
  // Compile vertex shader
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  Utility::compile_shader(vertex_shader, "../shaders/text.vert");

  // Compile frag shader
  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  Utility::compile_shader(frag_shader, "../shaders/text.frag");

  // Create shader program
  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader);
  glAttachShader(program_, frag_shader);

  // Link  program
  glLinkProgram(program_);

  // Get program and uniform locations
  coord_location_ = glGetAttribLocation(program_, "coord");
  tex_coord_location_   = glGetAttribLocation(program_, "tex_coord");
  tex_location_   = glGetUniformLocation(program_, "tex");
  color_location_ = glGetUniformLocation(program_, "color");
  projection_location_ = glGetUniformLocation(program_, "projection");
}

void TextRenderer::draw_text(std::string text,
                GLfloat x, GLfloat y,
                GLfloat scale, glm::vec3 color) const {

    // Setup program state
    glUseProgram(program_);
    glUniform3f(color_location_, color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(tex_location_, 0);

    // Setup pixel coordinates
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(screen_dims_.x),
                                      0.0f, static_cast<GLfloat>(screen_dims_.y));
    glUniformMatrix4fv(projection_location_, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao_);

    // Iterate through all text characters
    for(const auto& c : text) {
        const Character& ch = characters_.at(c);

        GLfloat xpos = x + ch.bearing.x * scale;
        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

        GLfloat w = ch.size.x * scale;
        GLfloat h = ch.size.y * scale;
        // Update VBO for each character (x,y,tex_x,tex_y)
        GLfloat vertices[] = {
          xpos,     ypos,       0.0, 1.0,
          xpos,     ypos + h,   0.0, 0.0,
          xpos + w, ypos,       1.0, 1.0,

          xpos + w, ypos,       1.0, 1.0,
          xpos,     ypos + h,   0.0, 0.0,
          xpos + w, ypos + h,   1.0, 0.0
        };

        glBindTexture(GL_TEXTURE_2D, ch.tex);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // value in pixels
    }

    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::create_character_map() {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (GLubyte c = 0; c < 128; c++) {
    // Load character
    if (FT_Load_Char(ft_face_, c, FT_LOAD_RENDER))
      throw std::runtime_error("Error loading freetype glyph\n");

    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        ft_face_->glyph->bitmap.width,
        ft_face_->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        ft_face_->glyph->bitmap.buffer
    );

    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Append to map
    Character character = {
        texture,
        glm::ivec2(ft_face_->glyph->bitmap.width, ft_face_->glyph->bitmap.rows),
        glm::ivec2(ft_face_->glyph->bitmap_left, ft_face_->glyph->bitmap_top),
        static_cast<GLuint>(ft_face_->glyph->advance.x)
    };

    characters_.insert({c, character});
  }

  // Unbind texture
  glBindTexture(GL_TEXTURE_2D, 0);

  // Cleanup freetype
  FT_Done_Face(ft_face_);
  FT_Done_FreeType(ft_);
}
}

#pragma once

#include "drawable.h"
#include "parameters.h"
#include "dimension.h"
#include "user_input.h"

#include "utility_math.h"
#include "GL/glew.h"
#include "ogl_utils.h"

/**
  Class to manage modifying mover values
**/
template<typename Real, Dimension Dim>
class Mover: public Drawable {
  public:
    Mover(Parameters<Real, Dim>& parameters):
                         parameters_{parameters},
                         yaw_{0.0},
                         pitch_{0.0},
                         edit_speed_{1.0},
                         relative_front_{0.0f, 0.0f, -1.0f},
                         relative_up_{0.0f, 1.0f, 0.0f} {
      this->create_buffers();
      this->create_program();
};

    ~Mover() {
      this->destroy_buffers();
      this->destroy_program();
    }

    Mover(const Mover&)            = default;
    Mover& operator=(const Mover&) = default;
    Mover(Mover&&) noexcept        = default;
    Mover& operator=(Mover&&)      = default;


    void draw() const {
      /// Update point and color buffers
      GLfloat mover_center[3] = {parameters_.mover_center_.x,
                                 parameters_.mover_center_.y,
                                 parameters_.mover_center_.z};
      GLfloat color[3] = {1.0, 0.0, 0.0};

      // Set buffer
      glBindBuffer(GL_ARRAY_BUFFER, vbo_point_);
      // Orphan current buffer
      glBufferData(GL_ARRAY_BUFFER, sizeof(mover_center), NULL, GL_STREAM_DRAW);
      // Fill buffer
      glBufferData(GL_ARRAY_BUFFER, sizeof(mover_center), mover_center, GL_STREAM_DRAW);
      // Unbind buffer
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      // Set buffer
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_);
      // Orphan current buffer
      glBufferData(GL_ARRAY_BUFFER, sizeof(color), NULL, GL_STREAM_DRAW);
      // Fill buffer
      glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STREAM_DRAW);
      // Unbind buffer
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      /// Draw mover

      // Bind circle shader program
      glUseProgram(program_);

      // Set radius uniform
      GLfloat mover_radius = 0.2 - parameters_.particle_radius()/1.5f;
      glUniform1f(sphere_radius_location_, mover_radius);
      // Set uniform binding
      glUniformBlockBinding(program_, view_matrices_index_, Camera::binding_index);
      glUniformBlockBinding(program_, light_index_, Light::binding_index);

      // Enable VAO
      glBindVertexArray(vao_);

      // Draw
      const std::size_t mover_count = 1;
      glDrawArrays(GL_POINTS, 0, 1);

      // Unbind VAO and program
      glBindVertexArray(0);
      glUseProgram(0);
    }

    void process_input(const UserInput& user_input) {
      if(!parameters_.compute_active())
        return;

      if(user_input.key_was_pressed("m"))
        parameters_.toggle_mover_edit();

      if(parameters_.edit_mover()) {
        const Real delta = parameters_.smoothing_radius() * 0.5;
        if(user_input.key_is_pressed("w"))
          move_forward();
        if(user_input.key_is_pressed("a"))
          move_left();
        if(user_input.key_is_pressed("s"))
          move_back();
        if(user_input.key_is_pressed("d"))
          move_right();

        handle_mouse(user_input.mouse_delta_x(), user_input.mouse_delta_y());
      }
    }

    void handle_mouse(int x_rel, int y_rel) {
      float sensitivity = (Real)0.005;
      pitch_ -= y_rel * sensitivity;
      yaw_ += x_rel * sensitivity;

      pitch_ = Utility::clamp(pitch_, (float)-M_PI/(Real)2.01, (float)M_PI/(Real)2.01);
      yaw_ = Utility::clamp(yaw_, (float)-M_PI/(Real)2.01, (float)M_PI/(Real)2.01);


      Vec<Real,Dim> new_front{std::cos(pitch_) * std::sin(yaw_),
                              std::sin(pitch_),
                              -std::cos(pitch_) * std::cos(yaw_)};
      relative_front_ = normalize(new_front);
    }

    void move_forward(float frame_time = 0.016f) {
      parameters_.mover_center_ += edit_speed_ * frame_time * relative_front_;
    }

    void move_back(float frame_time = 0.016f) {
      parameters_.mover_center_ -= edit_speed_ * frame_time * relative_front_;
    }

    void move_left(float frame_time = 0.016f) {
      parameters_.mover_center_ += edit_speed_ * frame_time * normalize(
                                                 cross(relative_front_,
                                                       (Real)-1.0 * relative_up_));
    }

    void move_right(float frame_time = 0.016f) {
      parameters_.mover_center_ += edit_speed_ * frame_time * normalize(
                                                 cross(relative_front_,
                                                       relative_up_));
    }

  private:
    Parameters<Real,Dim>& parameters_;
    Real edit_speed_; // Movement speed
    Real yaw_;
    Real pitch_;
    Vec<Real,3> relative_front_;
    Vec<Real,3> relative_up_;

    GLuint program_;

    GLint position_location_;
    GLint color_location_;
    GLint sphere_radius_location_;

    GLint view_matrices_index_;
    GLint light_index_;

    GLuint vbo_point_;
    GLuint vbo_color_;
    GLuint vao_;

    void create_buffers() {
      // Generate array object
      glGenVertexArrays(1, &vao_);
      // Generate vertex buffer
      glGenBuffers(1, &vbo_point_);
      glGenBuffers(1, &vbo_color_);
    }

    void destroy_buffers() {
      glDeleteBuffers(1, &vao_);
      glDeleteBuffers(1, &vbo_point_);
      glDeleteBuffers(1, &vbo_color_);
    }

    void create_program() {
      // Compile vertex shader
      GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      Utility::compile_shader(vertex_shader, "../shaders/particles.vert");

      // Compile frag shader
      GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      Utility::compile_shader(fragment_shader, "../shaders/particles.frag");

      // Compile geometry shader
      GLuint geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
      Utility::compile_shader(geometry_shader, "../shaders/particles.geom");

      // Create shader program
      program_ = glCreateProgram();
      glAttachShader(program_, vertex_shader);
      glAttachShader(program_, fragment_shader);
      glAttachShader(program_, geometry_shader);

      // Link and use program
      Utility::link_program(program_);

      glUseProgram(program_);

      position_location_ = glGetAttribLocation(program_, "position");
      color_location_ = glGetAttribLocation(program_, "color");
      sphere_radius_location_ = glGetUniformLocation(program_, "sphere_radius");
      view_matrices_index_ = glGetUniformBlockIndex(program_, "view_matrices");
      light_index_ = glGetUniformBlockIndex(program_, "light");

      // Setup VAO
      glBindVertexArray(vao_);

      glBindBuffer(GL_ARRAY_BUFFER, vbo_point_);
      glVertexAttribPointer(position_location_, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), 0);
      glEnableVertexAttribArray(position_location_);
      glBindBuffer(GL_ARRAY_BUFFER, vbo_color_);
      glVertexAttribPointer(color_location_, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), 0);
      glEnableVertexAttribArray(color_location_);

      // Cleanup
      glDetachShader(program_, vertex_shader);
      glDetachShader(program_, fragment_shader);
      glDetachShader(program_, geometry_shader);
      glDeleteShader(vertex_shader);
      glDeleteShader(fragment_shader);
      glDeleteShader(geometry_shader);
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glUseProgram(0);
    }

    void destroy_program() {
      glDeleteProgram(program_);
    }

};

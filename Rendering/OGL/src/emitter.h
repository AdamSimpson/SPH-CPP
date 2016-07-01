#pragma once

#include "parameters.h"
#include "dimension.h"
#include "user_input.h"

namespace sim {

/**
  Class to manage modifying emitter values
**/
template<typename Real, Dimension Dim>
class Emitter {
  public:
    Emitter(Parameters<Real, Dim>& parameters):
                         parameters_{parameters},
                         yaw_{0.0},
                         pitch_{0.0},
                         edit_speed_{0.15},
                         speed_{parameters.particle_rest_spacing()/parameters.time_step()},
                         relative_front_{0.0f, 0.0f, -1.0f},
                         relative_up_{0.0f, 1.0f, 0.0f} {
      parameters_.emitter_velocity_ = speed_ * relative_front_;
    };

    ~Emitter()                                    = default;
    Emitter(const Emitter&)            = default;
    Emitter& operator=(const Emitter&) = default;
    Emitter(Emitter&&) noexcept        = default;
    Emitter& operator=(Emitter&&)      = default;

    void process_input(const UserInput& user_input) {
      if(user_input.key_was_pressed("e")) {
        parameters_.toggle_emitter_active();
        parameters_.disable_emitter_edit();
      }

     if(user_input.key_was_pressed("tab"))
       parameters_.toggle_emitter_edit();

      if(parameters_.edit_emitter()) {
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

      // Limit movement of stream to make it more controllable
      pitch_ = Utility::clamp(pitch_, (float)-M_PI/(Real)10.0, (float)M_PI/(Real)10.0);
      yaw_ = Utility::clamp(yaw_, (float)-M_PI/(Real)10.0, (float)M_PI/(Real)10.0);


      Vec<Real,Dim> new_front{std::cos(pitch_) * std::sin(yaw_),
                              std::sin(pitch_),
                              -std::cos(pitch_) * std::cos(yaw_)};
      relative_front_ = normal(new_front);

      parameters_.emitter_velocity_ = speed_ * normal(relative_front_);
    }

    void move_forward(float frame_time = 0.016f) {
      parameters_.emitter_center_ += edit_speed_ * frame_time * relative_front_;
    }

    void move_back(float frame_time = 0.016f) {
      parameters_.emitter_center_ -= edit_speed_ * frame_time * relative_front_;
    }

    void move_left(float frame_time = 0.016f) {
      parameters_.emitter_center_ += edit_speed_ * frame_time * normal(
                                                 cross(relative_front_,
                                                       (Real)-1.0 * relative_up_));
    }

    void move_right(float frame_time = 0.016f) {
      parameters_.emitter_center_ += edit_speed_ * frame_time * normal(
                                                 cross(relative_front_,
                                                       relative_up_));
    }

  private:
    Parameters<Real,Dim>& parameters_;
    Real edit_speed_; // Movement speed
    Real speed_;
    Real yaw_;
    Real pitch_;
    Vec<Real,3> relative_front_;
    Vec<Real,3> relative_up_;
};
}

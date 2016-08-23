#pragma once

#include "drawable.h"
#include "text_renderer.h"
#include "dimension.h"
#include "user_input.h"
#include <map>
#include <algorithm>
#include "glm/glm.hpp"
#include <functional>

namespace sim {

template <typename Real, Dimension Dim>
class Overlay: public Drawable {
public:
  Overlay(Parameters<Real, Dim>& parameters,
          glm::vec2 screen_dims): parameters_{parameters},
                                  text_size_{24},
                                  screen_dims_{screen_dims},
                                  text_renderer_{screen_dims, text_size_}
  {
    this->add_tunable("gravity", std::bind(&Parameters<Real,Dim>::gravity, &parameters),
                                 std::bind(&Parameters<Real,Dim>::increase_gravity, &parameters),
                                 std::bind(&Parameters<Real,Dim>::decrease_gravity, &parameters));

    this->add_tunable("rest density", std::bind(&Parameters<Real,Dim>::rest_density, &parameters),
                                 std::bind(&Parameters<Real,Dim>::increase_rest_density, &parameters),
                                 std::bind(&Parameters<Real,Dim>::decrease_rest_density, &parameters));

    this->add_tunable("visc_C",  std::bind(&Parameters<Real,Dim>::visc_c, &parameters),
                                 std::bind(&Parameters<Real,Dim>::increase_visc_c, &parameters),
                                 std::bind(&Parameters<Real,Dim>::decrease_visc_c, &parameters));

    this->add_tunable("gamma",   std::bind(&Parameters<Real,Dim>::gamma, &parameters),
                                 std::bind(&Parameters<Real,Dim>::increase_gamma, &parameters),
                                 std::bind(&Parameters<Real,Dim>::decrease_gamma, &parameters));

    this->add_tunable("smoothing radius",   std::bind(&Parameters<Real,Dim>::smoothing_radius, &parameters),
                                            std::bind(&Parameters<Real,Dim>::increase_smoothing_radius, &parameters),
                                            std::bind(&Parameters<Real,Dim>::decrease_smoothing_radius, &parameters));

    selected_tunable_ = "gravity";
  }

  void draw() const {
    // Draw tunable parameters in top left of screen
    int x_offset = text_size_ * 1.2f;
    int y_offset = screen_dims_.y - (text_size_ * 1.2f);

    // Render tunable parameters
    for(const auto& name : tunable_names_) {
      text_renderer_.draw_text(tunable_text(name),
                               x_offset, y_offset,
                               1.0f, this->tunable_color(name));
      y_offset -= text_size_ * 1.2f;
    }

    // Render FPS
    x_offset = screen_dims_.x - (text_size_ * 4.0f);
    y_offset = screen_dims_.y - (text_size_ * 1.2f);
    text_renderer_.draw_text(fps_text(),
                             x_offset, y_offset,
                             1.0f, default_color());
  }

  void move_selected_tunable_up() {
    const auto it = find(tunable_names_.begin(),
                         tunable_names_.end(),
                         selected_tunable_);

    if(selected_tunable_ == tunable_names_[0])
      selected_tunable_ = tunable_names_.back();
    else
      selected_tunable_ = *(it-1);
  }

  void move_selected_tunable_down() {
    const auto it = find(tunable_names_.begin(),
                         tunable_names_.end(),
                         selected_tunable_);

    if(selected_tunable_ == tunable_names_.back())
      selected_tunable_ = tunable_names_[0];
    else
      selected_tunable_ = *(it+1);
  }

  void process_input(const UserInput& user_input) {
    if(user_input.key_was_pressed("h"))
      tunable_increase_.at(selected_tunable_)();
    if(user_input.key_was_pressed("k"))
      tunable_decrease_.at(selected_tunable_)();
    if(user_input.key_was_pressed("j"))
      move_selected_tunable_up();
    if(user_input.key_was_pressed("l"))
      move_selected_tunable_down();
  }

  void set_fps(int fps) {
    fps_ = fps;
  }

private:
  Parameters<Real,Dim>& parameters_;
  glm::vec2 screen_dims_;
  // List of tunable overlay entries
  std::vector<std::string> tunable_names_;

  int fps_;

  // Current selected tunable items name
  std::string selected_tunable_;

  // Map between tunable name and query/increase/decrease function calls
  std::map< std::string, std::function<Real()> > tunable_value_;
  std::map< std::string, std::function<void()> > tunable_increase_;
  std::map< std::string, std::function<void()> > tunable_decrease_;

  const int text_size_; // text size in pixels

  TextRenderer text_renderer_;

  glm::vec3 tunable_color(const std::string& tunable_name) const {
    const glm::vec3 green{0.1f, 0.8f, 0.43f};
    const glm::vec3 white{1.0f, 1.0f, 1.0f};

    return selected(tunable_name) ? green : white;
  }

  glm::vec3 default_color() const {
    return glm::vec3{1.0f, 1.0f, 1.0f};
  }

  bool selected(const std::string& tunable_name) const {
    return tunable_name == selected_tunable_;
  }

  void add_tunable(const std::string& name,
                   std::function<Real()> value_func,
                   std::function<void()> increase_func,
                   std::function<void()> decrease_func) {
    tunable_names_.push_back(name);
    tunable_value_[name]    = value_func;
    tunable_increase_[name] = increase_func;
    tunable_decrease_[name] = decrease_func;
  }

  std::string tunable_text(const std::string& name) const {
    std::ostringstream text;
    text << name << ": " << tunable_value_.at(name)();
    return text.str();
  }

  std::string fps_text() const {
    std::ostringstream text;
    text << "FPS: " << fps_;
    return text.str();
  }

};
}

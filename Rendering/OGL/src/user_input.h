#pragma once

#include <unordered_set>
#include "SDL2/SDL.h"

class UserInput {
public:
  // update input events since last call to update
  void update();

  // return true if key was pressed since last call to update()
  bool key_was_pressed(const char* key) const;

  // return true if key was pressed during last call to update()
  bool key_is_pressed(const char* key) const;

  float mouse_delta_x() const;
  float mouse_delta_y() const;

private:
  // set of keys pressed since last call to update
  std::unordered_set<SDL_Keycode> keys_are_pressed_;

  // set of keys currently pressed during last call to update
  std::unordered_set<SDL_Keycode> keys_were_pressed_;

  // delta mouse motion in pixels since UserInput last updated
  float mouse_dx_;
  float mouse_dy_;
};

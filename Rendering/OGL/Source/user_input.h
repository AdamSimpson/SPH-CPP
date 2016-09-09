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

#include <unordered_set>
#include "SDL2/SDL.h"

namespace sim {

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
}

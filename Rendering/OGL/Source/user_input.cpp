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

#include "user_input.h"
#include "SDL2/SDL.h"

namespace sim {

// To persist state between key up/down events
void UserInput::update() {
  // Reset mouse deltas before accumulating events
  mouse_dx_ = 0.0f;
  mouse_dy_ = 0.0f;

  // Rest array containing keys that were pressed
  keys_were_pressed_.clear();

  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_KEYDOWN:
        keys_are_pressed_.insert(event.key.keysym.sym);
        keys_were_pressed_.insert(event.key.keysym.sym);
        break;
      case SDL_KEYUP:
        keys_are_pressed_.erase(event.key.keysym.sym);
        break;
      case SDL_MOUSEMOTION:
        mouse_dx_ += event.motion.xrel;
        mouse_dy_ += event.motion.yrel;
        break;
    }
  }
}

bool UserInput::key_was_pressed(const char* key) const {
  SDL_Keycode keycode = SDL_GetKeyFromName(key);
  return keys_were_pressed_.count(keycode);
}

bool UserInput::key_is_pressed(const char* key) const {
  SDL_Keycode keycode = SDL_GetKeyFromName(key);
  return keys_are_pressed_.count(keycode);
}

float UserInput::mouse_delta_x() const {
  return mouse_dx_;
}

float UserInput::mouse_delta_y() const {
  return mouse_dy_;
}
}

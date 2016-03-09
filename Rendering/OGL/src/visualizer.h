#pragma once

#include "SDL2/SDL.h"
#include "GL/glew.h"
#include "ogl_utils.h"
#include <unordered_set>
#include <vector>

#include "drawable.h"
#include "distributor.h"
#include "dimension.h"
#include "camera.h"
#include "light.h"
#include "container.h"
#include "particles.h"
#include "parameters.h"

template<typename Real, Dimension Dim>
class Visualizer {
public:
  /**
    create OpenGL window and initialize render components
  **/
  Visualizer(Parameters<Real, Dim>& parameters): parameters_{parameters}
  {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(true); // Enable vsync
    window_ = SDL_CreateWindow("SPH", 0, 0, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if(window_ == NULL)
      throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));

    gl_context_ = SDL_GL_CreateContext(window_);
    if(gl_context_ == NULL)
      throw std::runtime_error("Failed to create SDL GL context: " + std::string(SDL_GetError()));

    if(SDL_GL_MakeCurrent(window_, gl_context_) != 0)
      throw std::runtime_error("Failed to make GL context current: " + std::string(SDL_GetError()));

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError(); // Catch and ignore the "errors" glewInit seems to throw

    // Don't stop mouse at edge of window
    int err = SDL_SetRelativeMouseMode(SDL_TRUE);
    if(err != 0) {
      std::cout<<"Unable to set mouse relative mode!"<<std::endl;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    glEnable(GL_DEPTH_CLAMP);

    // post OGL initilization camera and light initilization
    camera_.init();
    light_.init();
  }

  ~Visualizer() {
    SDL_GL_DeleteContext(gl_context_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
  }

  void display() {
    SDL_GL_SwapWindow(window_);
  }

  float aspect_ratio() {
    int w,h;
    SDL_GL_GetDrawableSize(window_, &w, &h);
    return (float)w /(float)h;
  }

  // Key state is retained in an unordered_set
  // To persist state between key up/down events
  void process_input() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN:
          keys_pressed_.insert(event.key.keysym.sym);
          break;
        case SDL_KEYUP:
          keys_pressed_.erase(event.key.keysym.sym);
          break;
        case SDL_MOUSEMOTION:
          camera_.handle_mouse(event.motion.xrel, event.motion.yrel);
          break;
      }
    }

    // Process pressed keys
    for(auto key : keys_pressed_) {
      switch(key) {
        case SDLK_ESCAPE:
          parameters_.exit_simulation();
          break;
        case SDLK_w:
          camera_.move_forward();
          break;
        case SDLK_a:
          camera_.move_left();
          break;
        case SDLK_s:
          camera_.move_back();
          break;
        case SDLK_d:
          camera_.move_right();
          break;
        case SDLK_p:
          parameters_.toggle_computation();
          break;
        case SDLK_UP:
          camera_.handle_mouse(0, 1);
          break;
        case SDLK_DOWN:
          camera_.handle_mouse(0, -1);
          break;
        case SDLK_RIGHT:
          camera_.handle_mouse(1, 0);
          break;
        case SDLK_LEFT:
          camera_.handle_mouse(-1,0);
          break;
      }
    }

  }

  void add_drawable(const Drawable& drawable) {
    drawables_.push_back(&drawable);
  }

  void draw_scene() {
    // Clear background
    glClearColor(0.15, 0.15, 0.15, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_.update(this->aspect_ratio());
    light_.update(camera_.view_matrix());

    for(const auto drawable : drawables_) {
      drawable->draw();
    }

    // swaps front and back buffers
    this->display();
  }

private:
  SDL_Window* window_;
  SDL_GLContext gl_context_;
  Camera camera_;
  Light light_;
  Parameters<Real,Dim> &parameters_;
  std::vector<Drawable const *> drawables_;
  std::unordered_set<SDL_Keycode> keys_pressed_;
};

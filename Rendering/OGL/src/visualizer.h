#pragma once

#include "SDL2/SDL.h"
#include "GL/glew.h"
#include "ogl_utils.h"
#include <unordered_set>

#include "distributor.h"
#include "dimension.h"
#include "world.h"
#include "container.h"
#include "particles.h"
#include "parameters.h"

template <typename Real, Dimension Dim>
class Visualizer {
public:

  // Initilize OpenGL and create a drawable surface
  // This exists so that OGL is in a usable state before renderable
  // members are constructed
  class GL_System {
    public:

    GL_System() {
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

      const float depthZNear = 0.0f;
      const float depthZFar = 1.0f;

      glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_TRUE);
      glDepthFunc(GL_LEQUAL);
      glDepthRange(depthZNear, depthZFar);
      glEnable(GL_DEPTH_CLAMP);

//      Utility::check_gl();
    }

    ~GL_System() {
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

    SDL_Window* window_;
    SDL_GLContext gl_context_;
  };

  /**
    create OpenGL window and initialize render components
  **/
  Visualizer(Parameters<Real, Dim>& parameters): world_(gl_system_.aspect_ratio(),
                                                  static_cast<AABB<float, three_dimensional>>(parameters.boundary())),
                                                 container_{world_},
                                                 particles_{world_},
                                                 parameters_{parameters} {}

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
          world_.handle_mouse(event.motion.xrel, event.motion.yrel);
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
          world_.move_camera_forward();
          break;
        case SDLK_a:
          world_.move_camera_left();
          break;
        case SDLK_s:
          world_.move_camera_back();
          break;
        case SDLK_d:
          world_.move_camera_right();
          break;
        case SDLK_p:
          parameters_.toggle_computation();
          break;
        case SDLK_UP:
          world_.handle_mouse(0, 1);
          break;
        case SDLK_DOWN:
          world_.handle_mouse(0, -1);
          break;
        case SDLK_RIGHT:
          world_.handle_mouse(1, 0);
          break;
        case SDLK_LEFT:
          world_.handle_mouse(-1,0);
          break;
      }
    }

  }

  void draw_scene() {
    // Clear background
    glClearColor(0.15, 0.15, 0.15, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    world_.update();

    // draw renderable elements
//    container_.draw();
    particles_.draw(parameters_.particle_radius_);

    // swaps front and back buffers
    gl_system_.display();
  }

  Particles& particles() {
    return particles_;
  }

  const Particles& particles() const {
    return particles_;
  }

private:
  GL_System gl_system_;
  World world_;
  Container container_;
  Particles particles_;
  Parameters<Real,Dim> &parameters_;
  std::unordered_set<SDL_Keycode> keys_pressed_;
};

#pragma once

#include "SDL2/SDL.h"
#include "GL/glew.h"
#include "ogl_utils.h"
#include <unordered_set>
#include <vector>
#include <iostream>
#include <memory>

#include "user_input.h"
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
  // @todo remove parameters and just pass in required
  Visualizer(Parameters<Real, Dim>& parameters): parameters_{parameters}
  {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    bool vsync_enabled = true;
    std::cout<< "vsync enabled: " << vsync_enabled << std::endl;
    SDL_GL_SetSwapInterval(vsync_enabled); // Enable vsync
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
    std::cout<<"OGL Winding Clockwise!\n";

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    glEnable(GL_DEPTH_CLAMP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // post OGL initilization camera and light initilization
    camera_.init();
    glm::vec3 camera_position{parameters_.boundary_.length() * 0.5f,
                              parameters_.boundary_.height() * 0.5f,
                              parameters_.boundary_.depth() * 3.5f };
    camera_.set_position(camera_position);
    camera_.set_speed(parameters_.boundary_.length());
    light_.init();
    glm::vec3 light_position{parameters_.boundary_.length() * 1.5f,
                              parameters_.boundary_.height() * 6.5f,
                              parameters_.boundary_.depth() * 2.5f };
    light_.set_position(light_position);

    sdl_ticks_ = 0;
  }

  ~Visualizer() {
    SDL_GL_DeleteContext(gl_context_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
  }

  void display() {
    unsigned int ticks = SDL_GetTicks();
    float seconds = (SDL_GetTicks() - sdl_ticks_)/1000.0;
    sdl_ticks_ = ticks;
    std::cout<<"Render FPS: "<<1.0/seconds<<std::endl;

    SDL_GL_SwapWindow(window_);
  }

  float aspect_ratio() {
    int width, height;
    SDL_GL_GetDrawableSize(window_, &width, &height);
    return (float)width /(float)height;
  }

  glm::vec2 screen_pixel_dimensions() {
    int width, height;
    SDL_GL_GetDrawableSize(window_, &width, &height);
    return glm::vec2{width, height};
  }

  void add_drawable(const Drawable& drawable) {
    drawables_.push_back(&drawable);
  }

  void draw_scene() {
    glClearColor(0.15, 0.15, 0.15, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_.update(this->aspect_ratio(), 0.1f, parameters_.boundary_.depth()*1.1f);
    light_.update(camera_.view_matrix());

    for(const auto drawable : drawables_) {
      drawable->draw();
    }

    this->display();
  }

  void process_input(const UserInput& user_input) {
    if(user_input.key_was_pressed("tab") || user_input.key_was_pressed("m"))
      parameters_.toggle_view_edit();

    if(parameters_.view_edit())
      camera_.process_input(user_input);

    if(user_input.key_was_pressed("escape"))
      parameters_.exit_simulation();

    if(user_input.key_was_pressed("p"))
      parameters_.toggle_computation_active();
  }

private:
  SDL_Window* window_;
  SDL_GLContext gl_context_;
  unsigned int sdl_ticks_;
  Camera camera_;
  Light light_;
  Parameters<Real,Dim>& parameters_;
  std::vector<const Drawable*> drawables_;
};

#pragma once

#include "GL/glew.h"
#include <SFML/Window.hpp>

#include "distributor.h"
#include "world.h"
#include "container.h"
#include "particles.h"
#include "dimension.h"

template<Dimension Dim>
class Renderer {
public:

  // Initilize OpenGL and create sf::window
  // This exists so that OGL is in a usable state before renderables are constructed
  class GL_Initilizer {
    public:
    sf::Window window_;

    GL_Initilizer() {
      sf::ContextSettings settings;
      settings.majorVersion = 3;
      settings.minorVersion = 3;

      const auto modes = sf::VideoMode::getFullscreenModes();
      window_.create(modes[0], "SPH", sf::Style::Fullscreen, settings);

      glewExperimental = GL_TRUE;
      glewInit();
      glGetError(); // Catch and ignore the errors glewInit seems to throw
    }
    sf::Window& window() {
      return window_;
    }
  };

  float aspect_ratio() {
    const auto window_size = window_.getSize();
    return (float)window_size.x /(float)window_size.y;
  }

  /**
    create OpenGL window and initialize render components
  **/
  Renderer(): window_{gl_initilizer_.window()},
              world_{this->aspect_ratio()},
              container_{world_},
              particles_{world_} {}

  /**
    begin main render loop
  **/
  void render_loop() {
    while (window_.isOpen()) {
      this->process_events();
      this->draw_scene();
    }
  }

  void process_events() {
    sf::Event event;
    while (window_.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window_.close();
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window_.close();
  }

  void draw_scene() {
    // Clear background
    glClearColor(0.15, 0.15, 0.15, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    container_.draw();

    // swaps front and back buffers
    window_.display();
  }

private:
  GL_Initilizer gl_initilizer_;
  sf::Window& window_;
  Distributor<Dim> distributor_;
  World world_;
  Container container_;
  Particles particles_;
};

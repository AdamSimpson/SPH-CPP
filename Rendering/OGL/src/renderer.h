#pragma once

#include "GL/glew.h"
#include <SFML/Window.hpp>

#include "distributor.h"
#include "world.h"
#include "container.h"
#include "dimension.h"

template<Dimension Dim>
class Renderer {
public:

  /**
    create OpenGL window and initialize render components
  **/
  Renderer(): container_{world_}
  {
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 3;

    const auto modes = sf::VideoMode::getFullscreenModes();
    window_.create(modes[0], "SPH", sf::Style::Fullscreen, settings);

    glewExperimental = GL_TRUE;
    glewInit();
    glGetError(); // Ignore the errors glewInit seems to throw

    const auto window_size = window_.getSize();
    const auto window_aspect_ratio = (float)window_size.x /(float)window_size.y;

    world_.init(window_aspect_ratio);
    container_.init();

    this->render_loop();
  }

  /**
    begin main render loop
  **/
  void render_loop() {
    while (window_.isOpen()) {
      this->process_events();
      this->update_particles();
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

  void update_particles() {

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
  sf::Window window_;
  Distributor<Dim> distributor_;
  World world_;
  Container container_;
};

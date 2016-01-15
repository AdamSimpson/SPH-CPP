#pragma once

#include "distributor.h"
#include "world_renderer.h"
#include "dimension.h"
#include <SFML/Window.hpp>

template<Dimension Dim>
class Renderer {
public:
  void begin() {
    window_.create(sf::VideoMode(800, 600), "My window");

    while (window_.isOpen()) {
      this->process_events();
    }
  }

  void process_events() {
    sf::Event event;
    while (window_.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window_.close();
    }
  }

  void render_world() {

  }

private:
  Distributor<Dim> distributor_;
  Camera camera_;
  sf::Window window_;
};

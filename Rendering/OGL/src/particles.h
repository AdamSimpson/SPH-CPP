#pragma once

#include "world.h"
#include "dimension.h"
#include "vec.h"
#include <vector>

class Particles {
public:
  Particles(const World& world);
  ~Particles();
  void draw(float particle_radius);
  void clear();

  template<typename Real>
  void add_particles(const std::vector< Vec<Real,two_dimensional> >& positions) {
      for(auto point : positions) {
        points_.push_back(point.x);
        points_.push_back(point.y);
        points_.push_back(0.0);

        colors_.push_back(0.0);
        colors_.push_back(0.0);
        colors_.push_back(1.0);
      }
  }

  template<typename Real>
  void add_particles(const std::vector< Vec<Real,three_dimensional> >& positions) {
      for(auto point : positions) {
        points_.push_back(point.x);
        points_.push_back(point.y);
        points_.push_back(point.z);

        colors_.push_back(0.0);
        colors_.push_back(0.0);
        colors_.push_back(1.0);
      }
  }

  std::vector<GLfloat>& points() {
    return points_;
  }

private:
  const World& world_;

  GLuint program_;

  GLint position_location_;
  GLint color_location_;
  GLint sphere_radius_location_;

  GLint view_matrices_index_;
  GLint light_index_;

  GLuint vbo_points_;
  GLuint vbo_colors_;
  GLuint vao_;

  std::vector<GLfloat> points_;
  std::vector<GLfloat> colors_;

  void create_buffers();
  void create_program();
  void destroy_buffers();
  void destroy_program();
};

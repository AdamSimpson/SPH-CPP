#pragma once

#include "drawable.h"
#include "dimension.h"
#include "GL/glew.h"
#include "vec.h"
#include <vector>

class Particles : public Drawable {
public:
  Particles();
  ~Particles();
  void draw() const;
  void clear();

  template<typename Real, Dimension Dim>
  void set_particles(const std::vector< Vec<Real,Dim> >& positions, float radius) {
    this->clear();
    this->add_particles(positions);
    radius_ = radius;
  }

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

  float radius_;

  void create_buffers();
  void create_program();
  void destroy_buffers();
  void destroy_program();
};

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

#include "drawable.h"
#include "utility_math.h"
#include "dimension.h"
#include "GL/glew.h"
#include "vec.h"
#include "ogl_utils.h"
#include <vector>

namespace sim {

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

  template<typename Real, Dimension Dim>
  void set_particles(const std::vector< Vec<Real,Dim> >& positions,
                     const std::vector<std::size_t>& counts, float radius) {
    this->clear();
    this->add_particles(positions, counts);
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

  template<typename Real>
  void add_particles(const std::vector< Vec<Real,three_dimensional> >& positions,
                     const std::vector<std::size_t>& particle_counts) {
      // Create color array with colors equally spaced around HSV space
      std::vector< Vec<float, three_dimensional> > colors_by_partition;
      float angle_spacing = 0.5f / particle_counts.size();
      Vec<float, three_dimensional> HSV;
      for(int i=0; i<particle_counts.size(); i++) {
        if(i%2)
          HSV.h = angle_spacing * i;
        else
          HSV.h = angle_spacing * i + 0.5f;
        HSV.s = 1.0f;
        HSV.v = 0.8f;
        colors_by_partition.push_back(Utility::hsv_to_rgb(HSV));
      }

      // Add particle positions and colors
      int current_color_index = 0;
      int current_color_count = 0;
      for(int i = 0; i<positions.size(); i++) {
        if(current_color_count == particle_counts[current_color_index]) {
          current_color_index++;
          current_color_count = 0;
        }

        points_.push_back(positions[i].x);
        points_.push_back(positions[i].y);
        points_.push_back(positions[i].z);

        colors_.push_back(colors_by_partition[current_color_index].r);
        colors_.push_back(colors_by_partition[current_color_index].g);
        colors_.push_back(colors_by_partition[current_color_index].b);

        current_color_count++;
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
}

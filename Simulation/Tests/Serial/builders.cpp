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

#include "vec.h"
#include <vector>
#include "builders.h"

/**
  Construct x_dim by y_dim grid of points spaced h apart, centered in cell
**/
 std::vector< Vec<float,2> > construct_points(int x_dim, int y_dim, float spacing) {
  std::vector< Vec<float,2> > points;

  for(int j=0; j < y_dim; j++) {
    for(int i=0; i < x_dim; i++) {
      float x = spacing * i + spacing/2.0;
      float y = spacing * j + spacing/2.0;
      points.push_back( Vec<float,2>(x,y) );
    }
  }
  return points;
}

/**
  Construct x_dim by y_dim grid of points spaced h apart, centered in cell
**/
 std::vector< Vec<float,3> > construct_points(int x_dim, int y_dim, int z_dim, float spacing) {
  std::vector< Vec<float,3> > points;

  for(int k=0; k < z_dim; k++) {
    for(int j=0; j < y_dim; j++) {
      for(int i=0; i < x_dim; i++) {
        float x = spacing * i + spacing/2.0;
        float y = spacing * j + spacing/2.0;
        float z = spacing * k + spacing/2.0;

        points.push_back( Vec<float,3>(x,y,z) );
      }
    }
  }
  return points;
}

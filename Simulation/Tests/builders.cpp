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

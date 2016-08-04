#pragma once

/**
  Construct x_dim by y_dim grid of points spaced h apart, centered in cell
**/
 std::vector< Vec<float,2> > construct_points(int x_dim, int y_dim, float spacing);

/**
  Construct x_dim by y_dim by z_dim grid of points spaced h apart
**/
 std::vector< Vec<float,3> > construct_points(int x_dim, int y_dim, int z_dim, float spacing);

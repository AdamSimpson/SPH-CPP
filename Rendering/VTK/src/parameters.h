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

#ifndef VTK_SRC_PARAMETERS_H_
#define VTK_SRC_PARAMETERS_H_

#include <string>

struct double3 {
  double x;
  double y;
  double z;
};
double3 ToDouble3(const std::string input_string);

struct Camera {
  double3 view_up;
  double3 position;
  double3 focal_point;
};

struct Boundary {
  double3 min_coord;
  double3 max_coord;
};

class Parameters {
  public:
    Parameters(const std::string ini_name) : ini_file_name{ini_name} {};
    void ReadParameters();
    Camera camera;
    Boundary boundary;
    std::string bp_file_name;

  private:
    std::string ini_file_name;
};

#endif

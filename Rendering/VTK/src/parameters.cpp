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

#include "parameters.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>

void Parameters::ReadParameters() {
  boost::property_tree::ptree property_tree;

  try {
    boost::property_tree::ini_parser::read_ini(this->ini_file_name, property_tree);

    // Camera Parameters
    this->camera.view_up     = ToDouble3(property_tree.get<std::string>("Camera.view_up"));
    this->camera.position    = ToDouble3(property_tree.get<std::string>("Camera.position"));
    this->camera.focal_point = ToDouble3(property_tree.get<std::string>("Camera.focal_point"));

    // Boundary Parameters
    this->boundary.min_coord = ToDouble3(property_tree.get<std::string>("Boundary.min_coord"));
    this->boundary.max_coord = ToDouble3(property_tree.get<std::string>("Boundary.max_coord"));

    this->bp_file_name = property_tree.get<std::string>("Input.bp_file_name");

  } catch(std::exception const& exception) {
      std::cout << "Aborting: " << exception.what() << std::endl;
      exit(-1);
  }
}

// Returns a double3 from comma separated input string
double3 ToDouble3(const std::string input_string) {
  double3 result;
  std::stringstream ss(input_string);
  std::string item;
  std::getline(ss, item, ',');
  boost::algorithm::trim(item);
  result.x = (boost::lexical_cast<double>(item));
  std::getline(ss, item, ',');
  boost::algorithm::trim(item);
  result.y = (boost::lexical_cast<double>(item));
  std::getline(ss, item);
  boost::algorithm::trim(item);
  result.z = (boost::lexical_cast<double>(item));

  return result;
}

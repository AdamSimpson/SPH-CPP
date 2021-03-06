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

#include "../../../src/vec.h"

#include "adios_reader.h"
#include <stdexcept>
#include <vector>
#include <string>
#include <boost/mpi.hpp>

extern "C" {
  #include "adios_read.h"
}

AdiosReader::AdiosReader(const std::string& name, const boost::mpi::communicator& comm): file_name{name}, communicator{comm} {

  adios_read_init_method(ADIOS_READ_METHOD_BP, static_cast<MPI_Comm>(communicator), "verbose=2");
  adios_file = adios_read_open_file(file_name.c_str(), ADIOS_READ_METHOD_BP, comm);

  if(adios_errno)
    throw std::runtime_error{adios_errmsg()};
}

AdiosReader::~AdiosReader() {
  adios_selection_delete(adios_selection);
  adios_read_finalize_method(ADIOS_READ_METHOD_BP);
  adios_read_close(adios_file);
}

template<typename T>
std::vector<T> AdiosReader::FetchValue(const std::string& value_name, std::size_t step) {
  ADIOS_VARINFO *var_info = adios_inq_var(adios_file, value_name.c_str());

  // Calculate element count of value variable
  int num_dims = var_info->ndim;
  size_t value_count = 1; // If num_dims == 0 the value is a scalar...this seems silly
  if(num_dims > 0) {
    value_count = var_info->dims[0];
    for(int i=1; i<num_dims; i++)
      value_count *= var_info->dims[i];
  }

  // hacked to work only for bytes of vec<float,3>
  value_count /= sizeof(Vec<float,3>);

  std::vector<T> value_data;
  value_data.resize(value_count);

  adios_schedule_read(adios_file, adios_selection, value_name.c_str(), step, 1, value_data.data());
  adios_perform_reads(adios_file, 1);

  adios_free_varinfo(var_info);

  return value_data;
}

// Explicitly instantiate byte reader
template std::vector<Vec<float,2>> AdiosReader::FetchValue<Vec<float,2>>(const std::string&, std::size_t step);
template std::vector<Vec<float,3>> AdiosReader::FetchValue<Vec<float,3>>(const std::string&, std::size_t step);


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

#ifndef VTK_SRC_ADIOS_READER_H_
#define VTK_SRC_ADIOS_READER_H_

#include <vector>
#include <string>
#include <boost/mpi.hpp>

extern "C" {
  #include "adios_read.h"
}

class AdiosReader {
  public:
    AdiosReader(const std::string& name, const boost::mpi::communicator& comm);
    ~AdiosReader();

    template<typename T>
    std::vector<T> FetchValue(const std::string& value_name, std::size_t step);

    AdiosReader(const AdiosReader&)            =delete;
    AdiosReader& operator=(const AdiosReader&) =delete;
    AdiosReader(AdiosReader&&) noexcept        =delete;
    AdiosReader& operator=(AdiosReader&&)      =delete;

  private:
    std::string file_name;
    ADIOS_FILE *adios_file;
    ADIOS_SELECTION *adios_selection = NULL;
    boost::mpi::communicator communicator;
};

#endif

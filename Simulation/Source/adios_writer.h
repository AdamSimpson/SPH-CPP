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

#include  <stdexcept>
#include "dimension.h"
#include "distributor.h"
#include "particles.h"
#include "mpi++.h"

extern "C" {
  #include "adios.h"
  #include "adios_error.h"
}

namespace sim {
/////////////////////////////////////////////////
// Adios file/stream writer class
/////////////////////////////////////////////////

  template<typename Real, Dimension Dim>
  class AdiosWriter {
  public:
    /*! Constructor: initilize adios environment and required members
       @param adios_writer_xml string to adios writer .xml file
       @param distributor reference
       @param particles reference
       @param parameters reference
    **/
    AdiosWriter(const std::string &adios_writer_xml,
                const Distributor<Real, Dim> &distributor): distributor_{distributor} {
      int err = adios_init(adios_writer_xml.c_str(), (MPI_Comm)distributor_.comm_compute().MPI_comm());
      if (err) {
        // Not all ranks return err on failure and so
        // MPI_Finalize will deadlock when distributor is destructed
        // So we let adios print the error message and then exit instead of
        // throwing exception
        exit(1);
      }
    }

    /*! Destructor: finalize adios environment
    **/
    ~AdiosWriter() {
      adios_finalize(distributor_.compute_rank());
    }

    AdiosWriter(const AdiosWriter &) = delete;
    AdiosWriter &operator=(const AdiosWriter &) = delete;
    AdiosWriter(AdiosWriter &&) noexcept = delete;
    AdiosWriter &operator=(AdiosWriter &&)      = delete;

    /*! Write particle coordinates as described in adios_writer_xml
       Note: custom types are no supported by Adios so vectors are written as raw bytes
    **/
    void write_particles(const Particles<Real, Dim> &particles) {
      int err;
      err = adios_open(&adios_handle_, "particles", "sim-output.bp", "a", distributor_.comm_compute().MPI_comm());
      if (err)
        throw std::runtime_error(adios_get_last_errmsg());

      // Set ADIOS particle "group" size
      uint64_t group_bytes = 3 * sizeof(int64_t)                                  // global, local, offset count
                             + distributor_.resident_count() * sizeof(Vec<Real, Dim>);  // (x, y {,z})
      uint64_t total_bytes;
      err = adios_group_size(adios_handle_, group_bytes, &total_bytes);
      if (err)
        throw std::runtime_error(adios_get_last_errmsg());

      // Compute offset in global output for current rank(sum of ranks to the "left")
      std::size_t local_bytes = distributor_.resident_count() * sizeof(Vec<Real, Dim>);
      std::size_t offset_bytes = distributor_.comm_compute().scan_sum(local_bytes);
      offset_bytes -= local_bytes;

      uint64_t global_bytes = distributor_.global_resident_count() * sizeof(Vec<Real, Dim>);

      err = adios_write(adios_handle_, "global_bytes", &global_bytes);
      err |= adios_write(adios_handle_, "local_bytes", &local_bytes);
      err |= adios_write(adios_handle_, "offset_bytes", &offset_bytes);
      // adios_write takes a non-const pointer so we unsafely cast it away
      void *positions_ptr = static_cast<void *>(const_cast<Vec<Real, Dim> *>
      (particles.positions().data()));

      err |= adios_write(adios_handle_, "positions", positions_ptr);

      if (err)
        throw std::runtime_error(adios_get_last_errmsg());

      err = adios_close(adios_handle_);
      if (err)
        throw std::runtime_error(adios_get_last_errmsg());
    }

  private:
    int64_t adios_handle_;
    const Distributor<Real,Dim>& distributor_;
  };
}

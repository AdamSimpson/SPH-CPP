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

#include <cmath>
#include "managed_allocation.h"
#include "dimension.h"
#include "array.h"
#include "vec.h"
#include "parameters.h"
#include "device.h"
#include "sim_algorithms.h"
#include "iostream"

namespace sim {

#define MAX_NEIGHBORS 60

  struct NeighborList {
    std::size_t neighbor_indices[MAX_NEIGHBORS];
    int count;
  }; /**< Neighbor bucket */

// NVCC C++14 workaround for missing constexpr functionality
#define neighbor_count() Dim == 2 ? 9 : 27

  /*! Begin iterator for range based for loops over neighbor indices
   * @param Neighbor list to provide iterator for
   * @return Iterator pointing to beginning of the last neighbor indice
   */
  const std::size_t *begin(const NeighborList &list);

  /*! End iterator for range based for loops over neighbor indices
   * @param Neighbor list to provide iterator for
   * @return Iterator pointing to one past the last neighbor indice
   */
  const std::size_t *end(const NeighborList &list);

  template<typename Real, Dimension Dim>
  class Neighbors : public ManagedAllocation {
  public:
    /*! Constructor
     * @param parameters Populated simulation parameters
     */
    Neighbors(const Parameters<Real, Dim> &parameters) : parameters_{parameters},
                                                         bin_spacing_{parameters_.neighbor_bin_spacing()},
                                                         bin_dimensions_{static_cast<Vec<std::size_t, Dim>>(ceil(
                                                             (parameters.boundary().extent())
                                                             / bin_spacing_) + static_cast<Real>(2))},
                                                         begin_indices_{product(bin_dimensions_)},
                                                         end_indices_{product(bin_dimensions_)},
                                                         bin_ids_{parameters.max_particles_local()},
                                                         particle_ids_{parameters.max_particles_local()},
                                                         neighbor_lists_{parameters.max_particles_local()} {};

    /*! Neighbor bins subscript operator
     */
    DEVICE_CALLABLE
    const NeighborList &operator[](const std::size_t index) const {
      return neighbor_lists_[index];
    }

   /*! Calculate 2D neighbor grid bin ID
    * Return linearized bin value of Vec<Real,2>
    * Each point is shifted by bin_spacing in each direction
    * To account for a 1 block grid boundary. This boundary allows neighbors to easily
    * be searched for without worrying about boundary conditions.
    * bins works with [,) semantics
    * @param point to calculate bin ID for
    */
    DEVICE_CALLABLE
    std::size_t calculate_bin_id(const Vec<Real, 2> &point) const {
      const auto point_shifted = point + bin_spacing_;
      const auto bin_location = static_cast< Vec<std::size_t, two_dimensional> >(floor(point_shifted / bin_spacing_));
      return (bin_location.y * bin_dimensions_.x + bin_location.x);
    }

    /*! Calculate 3D neighbor grid bin ID
     * Return linearized bin value of Vec<Real,2>
     * Each point is shifted by bin_spacing in each direction
     * To account for a 1 block grid boundary. This boundary allows neighbors to easily
     * be searched for without worrying about boundary conditions.
     * bins works with [,) semantics
     * @param point to calculate bin ID for
     */
    DEVICE_CALLABLE
    std::size_t calculate_bin_id(const Vec<Real, 3> &point) const {
      const auto point_shifted = point + bin_spacing_;
      const auto bin_location = static_cast< Vec<std::size_t, three_dimensional> >(floor(point_shifted / bin_spacing_));
      return (bin_dimensions_.x * bin_dimensions_.y * bin_location.z) +
             (bin_location.y * bin_dimensions_.x + bin_location.x);
    }

    /*! Calclate bin value for each particle
     * @param span Particle indices in which calculate bin ID's for
     * @param position_stars values to use for bin ID's
     */
    void calculate_bins(const IndexSpan &span, const Vec<Real, Dim> *position_stars) {
      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t i) {
        bin_ids_[i] = this->calculate_bin_id(position_stars[i]);
        particle_ids_[i] = i;
      });
    }

    /*! Sort particle_ids array into bin order
     * @param particle_count number of bin IDs to sort
     */
    void sort_bins(std::size_t particle_count) {
      sim::algorithms::sort_by_key(bin_ids_.data(), bin_ids_.data() + particle_count, particle_ids_.data());
    }

    /*! Find begin/end bounds of bins
     * @param particle_count number of bin ID's to find begin/end bounds for
     */
    void find_bin_bounds(std::size_t particle_count) {
      IndexSpan search_span{0, product(bin_dimensions_)};

      sim::algorithms::lower_bound(bin_ids_.data(), bin_ids_.data() + particle_count,
                                   search_span,
                                   begin_indices_.data());

      sim::algorithms::upper_bound(bin_ids_.data(), bin_ids_.data() + particle_count,
                                   search_span,
                                   end_indices_.data());

    }

    /*! Calculate 2D neighbor bin indices based upon particle coordinate
     * @param coord 2D coordinate
     * @param neighbor_indices pointer to array capable of containing neighbor indices
     */
    DEVICE_CALLABLE
    void calculate_neighbor_indices(const Vec<Real, 2> &coord, std::size_t *neighbor_indices) {
      int index = 0;
      for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
          const Vec<Real, 2> neighbor_coord{coord.x + i * bin_spacing_,
                                            coord.y + j * bin_spacing_};
          neighbor_indices[index] = calculate_bin_id(neighbor_coord);
          ++index;
        }
      }
    }

    /*! Calculate 3D neighbor bin indices based upon particle coordinate
     * @param coord 3D coordinate
     * @param neighbor_indices pointer to array capable of containing neighbor indices
     */
    DEVICE_CALLABLE
    void calculate_neighbor_indices(const Vec<Real, 3> &coord, std::size_t *neighbor_indices) {
      int index = 0;
      for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
          for (int k = -1; k < 2; k++) {
            const Vec<Real, 3> neighbor_coord{coord.x + i * bin_spacing_,
                                              coord.y + j * bin_spacing_,
                                              coord.z + k * bin_spacing_};
            neighbor_indices[index] = calculate_bin_id(neighbor_coord);
            ++index;
          }
        }
      }
    }

    // Requires C++14
    /*
    constexpr std::size_t neighbor_count() const {
      return static_cast<std::size_t>(pow(3, Dim));
    }*/

    /*! Fill the neighbor bins in the specified particle span
     * @param span           particle indices in which to fill neighbors for
     * @param position_stars positions to used to calculate neighbors
     */
    void fill_neighbors(IndexSpan span, const Vec<Real, Dim> *position_stars) {
      const Real valid_radius_squared = bin_spacing_ * bin_spacing_;

      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t particle_index) {
        const auto position_star = position_stars[particle_index];
        auto &list = neighbor_lists_[particle_index];

        // Zero out neighbor count for current list
        list.count = 0;

        // @todo NVCC doesn't like this constexpr
        // std::size_t neighbor_bin_indices[neighbor_count()];
        std::size_t neighbor_bin_indices[neighbor_count()];

        calculate_neighbor_indices(position_stars[particle_index], neighbor_bin_indices);
        for (auto neighbor_bin_index : neighbor_bin_indices) {
          const auto begin_index = begin_indices_[neighbor_bin_index];
          const auto end_index = end_indices_[neighbor_bin_index];

          for (auto j = begin_index; j < end_index; ++j) {
            const std::size_t neighbor_particle_index = particle_ids_[j];
            if (particle_index == neighbor_particle_index)
              continue;

            const auto neighbor_position_star = position_stars[neighbor_particle_index];
            const Real distance_squared = magnitude_squared(position_star - neighbor_position_star);
            if (distance_squared < valid_radius_squared && list.count < MAX_NEIGHBORS) {
              list.neighbor_indices[list.count] = neighbor_particle_index;
              ++list.count;
            }
          }
        }
      });
    }

    /*! Find all particle neighbor
     * particles_to_bin_span is the span of particles to bin/sort
     * particles_to_fill_span is the span of particles to fill in neighbors for
     * This is seperate as we need to bin/sort resident + halo(total local) but
     * Only need to fill neighbors for resident
     * @param particles_to_bin_span  Span defining the particles to place in neighbor bins
     * @param particles_to_fill_span Span defining the particles in which need a neighbor list(usually excludes halo)
     * @param coords Particle coordinates to use with particle_to_bin_span and particle_to_fill_span
     */
    void find(const IndexSpan &particles_to_bin_span, const IndexSpan &particles_to_fill_span,
              const Vec<Real, Dim> *coords) {
      const auto particles_to_bin_count = particles_to_bin_span.end - particles_to_bin_span.begin;
      this->calculate_bins(particles_to_bin_span, coords);
      this->sort_bins(particles_to_bin_count);
      this->find_bin_bounds(particles_to_bin_count);
      this->fill_neighbors(particles_to_fill_span, coords);
    }

    /*! Neighbor grid bin dimensions
     * @return Vector describing the number of neighbor bins in the neighbor grid
     */
    Vec<std::size_t, Dim> bin_dimensions() const {
      return bin_dimensions_;
    }

    /*! Default destructor
     */
    ~Neighbors() = default;
    /*! Default copy constructor
     */
    Neighbors(const Neighbors &) = default;
    /*! Default copy assignment operator
     */
    Neighbors &operator=(const Neighbors &) = default;
    /*! Default move constructor
     */
    Neighbors(Neighbors &&) noexcept  = default;
    /*! Default move assignment operator
     */
    Neighbors &operator=(Neighbors &&) = default;

  private:
    const Parameters<Real, Dim> &parameters_; /**< Reference to simulation global parameters */

    Real bin_spacing_;                        /**< Neighbor grid bin spacing */
    Vec<std::size_t, Dim> bin_dimensions_;    /**< Vector describing the number of neighbor bins in the neighbor grid */

    sim::Array<std::size_t> begin_indices_;   /**< Begin indices for bin ids */
    sim::Array<std::size_t> end_indices_;     /**< End indices for bin ids */
    sim::Array<std::size_t> bin_ids_;         /**< Array of bin ids */
    sim::Array<std::size_t> particle_ids_;    /**< Array of particle ids */

    sim::Array<NeighborList> neighbor_lists_; /**< Array of neighbor lists */
  };
}
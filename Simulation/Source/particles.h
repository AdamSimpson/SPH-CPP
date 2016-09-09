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

#include "managed_allocation.h"
#include "dimension.h"
#include "vec.h"
#include "array.h"
#include "parameters.h"
#include "neighbors.h"
#include "kernels.h"
#include "device.h"
#include "sim_algorithms.h"
#include <limits>

/*! Class to handle 2D and 3D PBD fluid particle physics
 */
namespace sim {
  template<typename Real, Dimension Dim>
  class Particles : public ManagedAllocation {
  public:
    /*! Constructor: allocates maximum particle storage
     * @param Parameters Reference to global simulation parameters
     */
    Particles(const Parameters<Real, Dim> &parameters) :
        parameters_{parameters},
        max_local_count_{parameters.max_particles_local()},
        neighbors_{parameters},
        positions_{max_local_count_},
        position_stars_{max_local_count_},
        velocities_{max_local_count_},
        densities_{max_local_count_},
        lambdas_{max_local_count_},
        scratch_{max_local_count_},
        scratch_scalar_{max_local_count_} {};

    /*! Default destructor
     */
    ~Particles() = default;

    Particles() = delete;

    Particles(const Particles &) = delete;

    Particles &operator=(const Particles &) = delete;

    Particles(Particles &&) noexcept = delete;

    Particles &operator=(Particles &&)      = delete;

    /*! Local particle count getter
      @return Number of process local particles currently in use
     */
    std::size_t local_count() const { return positions_.size(); }

    /*! Available particle count gtter
       @return Number of particles available
     */
    std::size_t available() const { return positions_.available(); }

    /*! Maximum allowed particles getter getter
       @return Maximum number of node/process local particles available
     */
    std::size_t max_local_count() const { return max_local_count_; }

    /*! Positions getter
       @return Reference to positions array
     */
    sim::Array<Vec<Real, Dim>> &positions() { return positions_; }

    /*! Position Stars getter
       @return Reference to position stars array
     */
    sim::Array<Vec<Real, Dim>> &position_stars() { return position_stars_; }

    /*! Velocities getter
       @return Reference to velocities array
     */
    sim::Array<Vec<Real, Dim>> &velocities() { return velocities_; }

    /*! Densities getter
       @return Reference to densities array
     */
    sim::Array<Real> &densities() { return densities_; }

    /*! Lambdas getter
       @return Reference to lambdas array
     */
    sim::Array<Real> &lambdas() { return lambdas_; }

    /*! Scratch getter
       @return Reference to scratch array
     */
    sim::Array<Vec<Real, Dim>> &scratch() { return scratch_; }

    /*! Positions getter
       @return Reference to positions array
     */
    const sim::Array<Vec<Real, Dim>> &positions() const { return positions_; }

    /*! Position Stars getter
       @return Reference to position stars array
     */
    const sim::Array<Vec<Real, Dim>> &position_stars() const { return position_stars_; }

    /*! Velocities getter
       @return Reference to velocities array
     */
    const sim::Array<Vec<Real, Dim>> &velocities() const { return velocities_; }

    /*! Densities getter
       @return Reference to densities array
     */
    const sim::Array<Real> &densities() const { return densities_; }

    /*! Scratch getter
       @return Reference to scratch array
     */
    const sim::Array<Vec<Real, Dim>> &scratch() const { return scratch_; }

    /*! Remove particles from end of array
     * @param count Number of particles to remove from end of array
     */
    void remove(std::size_t count) {
      positions_.pop_back(count);
      position_stars_.pop_back(count);
      velocities_.pop_back(count);
      densities_.pop_back(count);
      lambdas_.pop_back(count);
      scratch_.pop_back(count);
      scratch_scalar_.pop_back(count);
    }

    /*! Add particle to end of array
     * @param positions      Reference to position to add
     * @param position_stars Reference to position_star to add
     * @param velocities     Reference to velocity to add
     */
    void add(const Vec<Real, Dim> &position,
             const Vec<Real, Dim> &position_star,
             const Vec<Real, Dim> &velocity) {

      // @todo: Should assert all are same size
      // @todo: Should assert there is enough space

      positions_.push_back(position);
      position_stars_.push_back(position_star);
      velocities_.push_back(velocity);

      densities_.push_back((Real) 0.0);
      lambdas_.push_back((Real) 0.0);
      scratch_.push_back(Vec<Real, Dim>{0.0});
      scratch_scalar_.push_back((Real) 0.0);
    }

    /*! Add array of particles to end of array
     * @param positions      Pointer to beginning of positions to add
     * @param position_stars Pointer to beginning of position_stars to add
     * @param velocities     Pointer to beginning of velocities to add
     * @param count          Number of particles to add
     */
    void add(const Vec<Real, Dim> *positions,
             const Vec<Real, Dim> *position_stars,
             const Vec<Real, Dim> *velocities,
             std::size_t count) {

      // @todo: Should assert there is enough space

      positions_.push_back(positions, count);
      position_stars_.push_back(position_stars, count);
      velocities_.push_back(velocities, count);

      densities_.push_back((Real) 0.0, count);
      lambdas_.push_back((Real) 0.0, count);
      scratch_.push_back(Vec<Real, Dim>{0.0}, count);
      scratch_scalar_.push_back((Real) 0.0, count);
    }

    /*! Construct fluid volume, filling 2D aabb
     * @param aabb     Axis aligned bounding box area to fill with particles
     * @param velocity Initial particle velocity
     * @return         Number of particles added
     */
    std::size_t construct_fluid(const AABB<Real, 2> &aabb,
                                const Vec<Real, Dim> velocity = Vec<Real, Dim>{(Real) 0.0}) {
      // |--|--|--|
      // -o--o--o-
      Vec<std::size_t, Dim> particle_counts = bin_count_in_volume(aabb, parameters_.particle_rest_spacing());

      for (std::size_t y = 0; y < particle_counts[1]; ++y) {
        for (std::size_t x = 0; x < particle_counts[0]; ++x) {
          Vec<std::size_t, 2> int_coord{x, y};
          Vec<Real, Dim> coord = static_cast< Vec<Real, Dim> >(int_coord) *
                                 parameters_.particle_rest_spacing() +
                                 aabb.min;
          coord += parameters_.particle_rest_spacing() / static_cast<Real>(2.0);
          this->add(coord, coord, velocity);
        }  // x
      } // y

      return product(particle_counts);
    }

    /*! Construct fluid volume, filling 3D aabb
     * @param aabb     Axis aligned bounding box volume to fill with particles
     * @param velocity Initial particle velocity
     * @return         Number of particles added
     */
    std::size_t construct_fluid(const AABB<Real, 3> &aabb,
                                const Vec<Real, Dim> velocity = Vec<Real, Dim>{(Real) 0.0}) {
      // |--|--|--|
      // -o--o--o-
      Vec<std::size_t, Dim> particle_counts = bin_count_in_volume(aabb, parameters_.particle_rest_spacing());

      for (std::size_t z = 0; z < particle_counts[2]; ++z) {
        for (std::size_t y = 0; y < particle_counts[1]; ++y) {
          for (std::size_t x = 0; x < particle_counts[0]; ++x) {
            Vec<std::size_t, 3> int_coord{x, y, z};
            Vec<Real, Dim> coord = static_cast< Vec<Real, Dim> >(int_coord) *
                                   parameters_.particle_rest_spacing() +
                                   aabb.min;
            coord += parameters_.particle_rest_spacing() / static_cast<Real>(2.0);
            this->add(coord, coord, velocity);
          }  // x
        } // y
      } // z
      return product(particle_counts);
    }

    /*! Find particle neighbors
     * @param to_bin_span  Span defining the particles to place in neighbor bins
     * @param to_fill_span Span defining the particles in which need a neighbor list(usually excludes halo)
     */
    void find_neighbors(IndexSpan to_bin_span, IndexSpan to_fill_span) {
      neighbors_.find(to_bin_span, to_fill_span, position_stars_.data());
    }

    /*! Apply external forces to particles
     * @param span Particle over which to apply external forces
     */
    void apply_external_forces(IndexSpan span) {
      const Real g = parameters_.gravity();
      const Real dt = parameters_.time_step();

      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        velocities_[p].y += g * dt;
      });
    }

    /*! Predict particle positions and apply boundary conditions
     * @param span Particles over which to predict the positions
     */
    void predict_positions(IndexSpan span) {
      const Real dt = parameters_.time_step();

      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        auto position_p = positions_[p];
        auto velocity = velocities_[p];
        auto position_star_p = position_p + (velocity * dt);

        apply_boundary_conditions(position_star_p,
                                  parameters_);
        position_stars_[p] = position_star_p;
      });
    }

    /*! Compute particle densities
     * @param span Particles over which to compute densities for
     */
    void compute_densities(IndexSpan span) {
      const Poly6<Real, Dim> W{parameters_.smoothing_radius()};
      const Real W_0 = W(static_cast<Real>(0.0));

      const Real mass = parameters_.rest_mass();
      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        // Own contribution to density
        Real density = mass * W_0;

        for (const std::size_t q : neighbors_[p]) {

          // @todo get rid of this hack!
          if (magnitude(position_stars_[p] - position_stars_[q]) < 0.00000001)
            position_stars_[p] -= velocities_[p] * parameters_.time_step() / (Real) 50.0;

          const Real r_mag = magnitude(position_stars_[p] - position_stars_[q]);

          density += mass * W(r_mag);
        }
        densities_[p] = density;
      });
    }

    /*! Compute pressure lambdas
     * @param span Span over which to calculate lambas for
     */
    void compute_pressure_lambdas(IndexSpan span) {
      const Del_Spikey<Real, Dim> Del_W{parameters_.smoothing_radius()};

      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        // pressure constraint
        const Real constraint = densities_[p] / parameters_.rest_density() - static_cast<Real>(1.0);
        // Clamp constraint to be positive
        const Real C_p = (constraint < 0.0 ? 0.0 : constraint);

        Real sum_C = 0.0;
        Vec<Real, Dim> sum_gradient{0.0};
        for (const std::size_t q : neighbors_[p]) {
          const Real density_0 = parameters_.rest_density();
          // Can pull density_0 down below so it's not in inner loop
          const Vec<Real, Dim> gradient = (Real) -1.0 / density_0 * Del_W(position_stars_[p], position_stars_[q]);
          sum_gradient -= gradient;
          // Add k = j contribution
          sum_C += magnitude_squared(gradient);
        }
        // k = i contribution
        sum_C += magnitude_squared(sum_gradient);

        lambdas_[p] = -C_p / (sum_C + parameters_.lambda_epsilon());
      });
    }

    /*! Compute pressure delta positions
     * @param span    Particles in which to compute delta positions for
     * @param substep Solver substep number
     */
    void compute_pressure_dps(IndexSpan span, const int substep) {
      const Del_Spikey<Real, Dim> Del_W{parameters_.smoothing_radius()};

      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        Vec<Real, Dim> dp{0.0};
        for (const std::size_t q : neighbors_[p]) {
          dp += (lambdas_[p] + lambdas_[q]) * Del_W(position_stars_[p], position_stars_[q]);
        }
        scratch_[p] = (Real) 1.0 / parameters_.rest_density() * dp;
      });
    };

    /*! Update particle position stars
     * @param span Particles over which to update position stars for
     */
    void update_position_stars(IndexSpan span) {
      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        // scratch contains delta positions
        auto position_star_p_new = position_stars_[p] + scratch_[p];
        apply_boundary_conditions(position_star_p_new, parameters_);
        position_stars_[p] = position_star_p_new;
      });
    };

    /*! Update particle velocities
     * @param span Particles over which to update velocities for
     */
    void update_velocities(IndexSpan span) {
      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {

        Vec<Real, Dim> velocity{(position_stars_[p] - positions_[p]) / parameters_.time_step()};
        // Clamp to 0 if velocity under threshold
        if (magnitude_squared(velocity) < 0.000001 * parameters_.max_speed())
          velocity = Vec<Real, Dim>{0.0};

        velocities_[p] = velocity;
      });
    }

    /*! Update particle positions based upon position_stars
     * @param span Particles over which to update positions for
     */
    void update_positions(IndexSpan span) {
      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        positions_[p] = position_stars_[p];
      });
    }

    /*! Apply surface tension
     * @param color_field_span
     * @param surface_tension_span
     */
    void apply_surface_tension(IndexSpan color_field_span, IndexSpan surface_tension_span) {
      const Del_Spikey<Real, Dim> Del_W{parameters_.smoothing_radius()};
      const C_Spline<Real, Dim> C{parameters_.smoothing_radius()};

      // Compute gradient of color field
      sim::algorithms::for_each_index(color_field_span, [=] DEVICE_CALLABLE(std::size_t p) {
        Vec<Real, Dim> color{0.0};
        for (const std::size_t q : neighbors_[p]) {
          color += Del_W(position_stars_[p], position_stars_[q]) / densities_[q];
        }
        scratch_[p] = parameters_.smoothing_radius() * color;
      });

      sim::algorithms::for_each_index(surface_tension_span, [=] DEVICE_CALLABLE(std::size_t p) {
        Vec<Real, Dim> surface_tension_force{0.0};

        for (const std::size_t q : neighbors_[p]) {
          const Vec<Real, Dim> r = position_stars_[p] - position_stars_[q];

          Real r_mag = magnitude(r);
          if (r_mag < parameters_.smoothing_radius() * 0.000001)
            r_mag = parameters_.smoothing_radius() * 0.000001;

          const Vec<Real, Dim> cohesion_force{-parameters_.gamma() * C(r_mag) * r / r_mag};
          const Vec<Real, Dim> curvature_force{-parameters_.gamma() * (scratch_[p] - scratch_[q])};
          const Real K = 2.0 * parameters_.rest_density() / (densities_[p] + densities_[q]);
          surface_tension_force += K * (cohesion_force + curvature_force);
        }

        velocities_[p] += surface_tension_force / densities_[p] * parameters_.time_step();
      });
    }

    /*! Apply viscosity
     * @param span Particles over which to apply viscosity
     */
    void apply_viscosity(IndexSpan span) {
      const Poly6<Real, Dim> W{parameters_.smoothing_radius()};

      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        Vec<Real, Dim> dv{0.0};
        for (const std::size_t q : neighbors_[p]) {
          const Real r_mag = magnitude(position_stars_[p] - position_stars_[q]);
          dv += (velocities_[q] - velocities_[p]) * W(r_mag) / densities_[q];
        }
        velocities_[p] += parameters_.visc_c() * dv;
      });
    }

    /*! Compute vorticity and store in scratch array
     * @param span Partilces over which to apply vorticity
     */
    void compute_vorticity(IndexSpan span) {
      const Del_Spikey<Real, Dim> Del_W{parameters_.smoothing_radius()};

      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        Vec<Real, Dim> vorticity{0.0};

        for (const std::size_t q : neighbors_[p]) {
          const auto del = Del_W(position_stars_[p], position_stars_[q]);
          const auto v_diff = velocities_[q] - velocities_[p];
          vorticity += cross(v_diff, del);
        }

        scratch_[p] = vorticity;
      });
    }

    /*! Apply vorticity from scratch array
     * @param span Span over which to apply vorticity
     */
    void apply_vorticity(IndexSpan span) {
      const Del_Spikey<Real, Dim> Del_W{parameters_.smoothing_radius()};
      // Scratch contains vorticity
      sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE(std::size_t p) {
        Vec<Real, Dim> eta{0.0};

        for (const std::size_t q : neighbors_[p]) {
          const auto del = Del_W(position_stars_[p], position_stars_[q]);
          const Real vorticity_magnitude = magnitude(scratch_[q]);
          eta += vorticity_magnitude * del;
        }

        const auto N = eta / (magnitude(eta) + std::numeric_limits<Real>::epsilon());
        velocities_[p] += cross(N, scratch_[p]) * parameters_.vorticity_coef() * parameters_.time_step();
      });

    }

    /*! @todo DEVICE_CALLABLE lambdas can't currently have private members */
    //private:
    const Parameters<Real, Dim> &parameters_;    /*<< Reference to simulation parameters */
    const std::size_t max_local_count_;          /*<< Maximum number of particles allowed per process */
    Neighbors<Real, Dim> neighbors_;             /*<< Particle neighbors */
    sim::Array<Vec<Real, Dim> > positions_;      /*<< Particle positions */
    sim::Array<Vec<Real, Dim> > position_stars_; /*<< Particle position stars */
    sim::Array<Vec<Real, Dim> > velocities_;     /*<< Particle velocities */
    sim::Array<Real> densities_;                 /*<< Particle densities */
    sim::Array<Real> lambdas_;                   /*<< Particle lambads */

    // Scratch values are used for delta_positions, vorticities, and color field
    // Don't overwrite data you depend on
    sim::Array<Vec<Real, Dim> > scratch_;        /*<< Scratch vec array */
    sim::Array<Real> scratch_scalar_;            /*<<  Scratch scalar array */
  };

  /*! Apply boundary conditions
   * @param position   Reference to positions to apply boundary conditions to
   * @param parameters Global simulation parameters
   */
  template<typename Real, Dimension Dim>
  DEVICE_CALLABLE
  void apply_boundary_conditions(Vec<Real, Dim> &position,
                                 const Parameters<Real, Dim> &parameters) {
    // Push outside of mover sphere
    const float mover_radius = 0.2;
    const auto boundary = parameters.boundary();
    const Vec<Real, Dim> mover_center = parameters.mover_center();
    Real dr_squared = magnitude_squared(position - mover_center);
    if (dr_squared < mover_radius * mover_radius) {
      Real dr = sqrt(dr_squared);
      position += (mover_radius - dr) * (position - mover_center) / dr;
    }

    // Clamp inside boundary
    clamp_in_place(position, boundary.min, boundary.max);
  }

}

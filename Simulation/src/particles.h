#pragma once

#include "managed_allocation.h"
#include "dimension.h"
#include "vec.h"
#include "array.h"
#include "parameters.h"
#include "neighbors.h"
#include "kernels.h"
#include "device.h"
#include <limits>

/**
  Class to handle 2D and 3D SPH particle physics
**/
namespace sim {
template<typename Real, Dimension Dim>
class Particles: public ManagedAllocation {
public:
  /**
    @brief Constructor: allocates maximum particle storage
  **/
  Particles(const Parameters<Real, Dim>& parameters):
                               parameters_{parameters},
                               max_local_count_{parameters.max_particles_local()},
                               neighbors_{parameters},
                               positions_{max_local_count_},
                               positions_previous_{max_local_count_},
                               position_stars_{max_local_count_},
                               velocities_{max_local_count_},
                               velocities_previous_{max_local_count_},
                               densities_{max_local_count_},
                               lambdas_{max_local_count_},
                               scratch_{max_local_count_},
                               scratch_scalar_{max_local_count_} {};

  Particles()                            = delete;
  ~Particles()                           = default;
  Particles(const Particles&)            = default;
  Particles& operator=(const Particles&) = default;
  Particles(Particles&&) noexcept        = default;
  Particles& operator=(Particles&&)      = default;

  /**
    @return number of node/process local particles currently in use
  **/
  std::size_t local_count() const { return positions_.size(); }

  /**
     @return number of particles available
   **/
  std::size_t available() const { return positions_.available(); }

  /**
    @return maximum number of node/process local particles available
  **/
  std::size_t max_local_count() const { return max_local_count_; }

  /**
    @return reference to particle sim::arrays
  **/
  sim::Array<Vec<Real,Dim>>& positions() { return positions_; }
  sim::Array<Vec<Real,Dim>>& position_stars() { return position_stars_; }
  sim::Array<Vec<Real,Dim>>& velocities() { return velocities_; }
  sim::Array<Real>& densities() { return densities_; }
  sim::Array<Real>& lambdas() { return lambdas_; }
  sim::Array<Vec<Real,Dim>>& scratch() { return scratch_; }

  const sim::Array<Vec<Real,Dim>>& positions() const { return positions_; }
  const sim::Array<Vec<Real,Dim>>& position_stars() const { return position_stars_; }
  const sim::Array<Vec<Real,Dim>>& velocities() const { return velocities_; }
  const sim::Array<Vec<Real,Dim>>& scratch() const { return scratch_; }

  /**
    Remove particles from end of array
  **/
  void remove(std::size_t count) {
    positions_.pop_back(count);
    positions_previous_.pop_back(count);
    position_stars_.pop_back(count);
    velocities_.pop_back(count);
    velocities_previous_.pop_back(count);
    densities_.pop_back(count);
    lambdas_.pop_back(count);
    scratch_.pop_back(count);
    scratch_scalar_.pop_back(count);
  }

  /**
     Add particle to end of array
  **/
  void add(const Vec<Real,Dim>& positions,
           const Vec<Real,Dim>& position_stars,
           const Vec<Real,Dim>& velocities) {

    // @todo: Should assert all are same size
    // @todo: Should assert there is enough space

    positions_.push_back(positions);
    positions_previous_.push_back(positions);
    position_stars_.push_back(position_stars);
    velocities_.push_back(velocities);
    velocities_previous_.push_back(velocities);

    densities_.push_back((Real)0.0);
    lambdas_.push_back((Real)0.0);
    scratch_.push_back(Vec<Real,Dim>{0.0});
    scratch_scalar_.push_back((Real)0.0);
  }

  /**
     Add array of particles to end of array
   **/
  void add(const Vec<Real,Dim>* positions,
           const Vec<Real,Dim>* position_stars,
           const Vec<Real,Dim>* velocities,
           std::size_t count) {

    // @todo: Should assert there is enough space

    positions_.push_back(positions, count);
    positions_previous_.push_back(positions, count);
    position_stars_.push_back(position_stars, count);
    velocities_.push_back(velocities, count);
    velocities_previous_.push_back(velocities, count);

    densities_.push_back((Real)0.0, count);
    lambdas_.push_back((Real)0.0, count);
    scratch_.push_back(Vec<Real,Dim>{0.0}, count);
    scratch_scalar_.push_back((Real)0.0, count);
  }

  /**
    @brief Set initial fluid particles to current node, filling 2D aabb
  **/
  std::size_t construct_fluid(const AABB<Real,2>& aabb,
                              const Vec<Real,Dim> velocity = Vec<Real,Dim>{(Real)0.0}) {
    // |--|--|--|
    // -o--o--o-
    Vec<std::size_t,Dim> particle_counts = bin_count_in_volume(aabb, parameters_.particle_rest_spacing());

    for(std::size_t y=0; y < particle_counts[1]; ++y) {
      for(std::size_t x=0; x < particle_counts[0]; ++x) {
        Vec<std::size_t,2> int_coord{x,y};
        Vec<Real,Dim> coord = static_cast< Vec<Real,Dim> >(int_coord) *
                               parameters_.particle_rest_spacing()    +
                               aabb.min;
        coord += parameters_.particle_rest_spacing()/static_cast<Real>(2.0);
        this->add(coord, coord, velocity);
      }  // x
    } // y

    return product(particle_counts);
  }

  /**
    @brief Set initial fluid particles to current node, filling 3D aabb
  **/
  std::size_t construct_fluid(const AABB<Real,3>& aabb,
                              const Vec<Real,Dim> velocity = Vec<Real,Dim>{(Real)0.0}) {
    // |--|--|--|
    // -o--o--o-
    Vec<std::size_t,Dim> particle_counts = bin_count_in_volume(aabb, parameters_.particle_rest_spacing());

    std::cout<< particle_counts.x<<","<<particle_counts.y<<","<<particle_counts.z<<std::endl;

    for(std::size_t z=0; z < particle_counts[2]; ++z) {
      for(std::size_t y=0; y < particle_counts[1]; ++y) {
        for(std::size_t x=0; x < particle_counts[0]; ++x) {
          Vec<std::size_t,3> int_coord{x,y,z};
          Vec<Real,Dim> coord = static_cast< Vec<Real,Dim> >(int_coord) *
                                 parameters_.particle_rest_spacing()    +
                                 aabb.min;
          coord += parameters_.particle_rest_spacing()/static_cast<Real>(2.0);
          this->add(coord, coord, velocity);
        }  // x
      } // y
    } // z
    return product(particle_counts);
  }

  void find_neighbors(IndexSpan to_bin_count, IndexSpan to_fill_count) {
    neighbors_.find(to_bin_count, to_fill_count, position_stars_.data());
  }

  /**
    @brief Apply external forces to particles
  **/
  void apply_external_forces(IndexSpan span) {
    const Real g = parameters_.gravity();
    const Real dt = parameters_.time_step();

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
      velocities_[p].y += g*dt;
    });
  }

  /**
    @brief predict particle positions and apply boundary conditions
  **/
  void predict_positions(IndexSpan span) {
    const Real dt = parameters_.time_step();

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
      auto position_p = positions_[p];
      auto velocity = velocities_[p];
      auto position_star_p = position_p + (velocity * dt);
//      auto position_star_p = static_cast<Real>(4.0/3.0) * position_p - static_cast<Real>(1.0/3.0) * positions_previous_[p]
//                             + static_cast<Real>(8.0/9.0) * dt * velocities_[p] - static_cast<Real>(2.0/9.0) * dt * velocities_previous_[p]
//                             + Vec<Real,Dim>(0.0, -static_cast<Real>(4.0/9.0)*dt*dt * static_cast<Real>(9.8), 0.0);

      apply_boundary_conditions(position_star_p,
                                parameters_);
      position_stars_[p] = position_star_p;
    });
  }

  void compute_densities(IndexSpan span) {
    const Poly6<Real,Dim> W{parameters_.smoothing_radius()};
    const Real W_0 = W(static_cast<Real>(0.0));

    const Real mass = parameters_.rest_mass();
    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        // Own contribution to density
        Real density = mass * W_0;

        for(const std::size_t q : neighbors_[p]) {

          // @todo get rid of this hack!
          if(magnitude(position_stars_[p] - position_stars_[q]) < 0.00000001)
            position_stars_[p] -= velocities_[p] * parameters_.time_step() / (Real)50.0;

          const Real r_mag = magnitude(position_stars_[p] - position_stars_[q]);

          density += mass * W(r_mag);
        }
        densities_[p] = density;
    });
  }

  void compute_pressure_lambdas(IndexSpan span) {
    const Del_Spikey<Real,Dim> Del_W{parameters_.smoothing_radius()};

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
      // pressure constraint
      const Real constraint = densities_[p]/parameters_.rest_density() - static_cast<Real>(1.0);
      // Clamp constraint to be positive
      const Real C_p = (constraint < 0.0 ? 0.0 : constraint);

      Real sum_C = 0.0;
      Vec<Real,Dim> sum_gradient{0.0};
      for(const std::size_t q : neighbors_[p]) {
        const Real density_0 = parameters_.rest_density();
        // Can pull density_0 down below so it's not in inner loop
        const Vec<Real,Dim> gradient = (Real)-1.0 / density_0 * Del_W(position_stars_[p], position_stars_[q]);
        sum_gradient -= gradient;
        // Add k = j contribution
        sum_C += magnitude_squared(gradient);
      }
      // k = i contribution
      sum_C += magnitude_squared(sum_gradient);

      lambdas_[p] = -C_p/(sum_C + parameters_.lambda_epsilon());
      });
  }
/*
  void compute_surface_lambdas(IndexSpan span) {
    const Del_Spikey<Real,Dim> Del_W{parameters_.smoothing_radius()};

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
      // surface tension constraint
      Vec<Real,Dim> color{0.0};
      for(const std::size_t q : neighbors_[p]) {
        color += Del_W(position_stars_[p],  position_stars_[q]) / densities_[q];
      }
      if(magnitude(color) < 0.0001)
        color = Vec<Real,Dim>{0.0};
      else
        color /= magnitude(color);

      Real constraint_tension = 0.0;
      for(const std::size_t q : neighbors_[p]) {
        const auto r = (position_stars_[p] - position_stars_[q]);
        const Real r_mag = magnitude(r);

        if(r_mag < 0.0001*parameters_.smoothing_radius());
          continue;

        const auto r_norm = r/r_mag;

        constraint_tension += dot(r_norm, color);
      }
      scratch_scalar_[p] = constraint_tension;
    });

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
      // Calculate gradient of constraint
      Real sum_C = 0.0;
      Vec<Real,Dim> sum_gradient{0.0};

      for(const std::size_t q : neighbors_[p]) {
        const Vec<Real,Dim> gradient = scratch_scalar_[q] / densities_[q] * Del_W(position_stars_[p], position_stars_[q]);
        sum_gradient += gradient;
        // Add k = j contribution
        sum_C += magnitude_squared(gradient);
      }
      // k = i contribution
      sum_C += magnitude_squared(sum_gradient);

      // surface tension contribution
      lambdas_[p] = -scratch_scalar_[p]/(sum_C + parameters_.lambda_epsilon());
      });
  }

  void compute_surface_dps(IndexSpan span, const int substep){
    const Del_Spikey<Real,Dim> Del_W{parameters_.smoothing_radius()};

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        const Real stiffness = 1.0 - pow(static_cast<Real>(1.0) - parameters_.k_stiff(),
                                         static_cast<Real>(1.0)/static_cast<Real>(substep));

        Vec<Real,Dim> dp{0.0};
        for(const std::size_t q : neighbors_[p]) {
          dp += scratch_scalar_[q] / densities_[q] * (lambdas_[p] + lambdas_[q]) * Del_W(position_stars_[p],
                                                             position_stars_[q]);
        }

        scratch_[p] = stiffness * (float)-0.005 * dp;
      });
  };
*/
  void compute_pressure_dps(IndexSpan span, const int substep){
    const Del_Spikey<Real,Dim> Del_W{parameters_.smoothing_radius()};

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        Vec<Real,Dim> dp{0.0};
        for(const std::size_t q : neighbors_[p]) {
          dp += (lambdas_[p] + lambdas_[q]) * Del_W(position_stars_[p], position_stars_[q]);
        }
        scratch_[p] = (Real)1.0/parameters_.rest_density() * dp;
      });
  };

  void update_position_stars(IndexSpan span){
    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        // scratch contains delta positions
        const auto position_star_p_old = position_stars_[p];
        auto position_star_p_new = position_stars_[p] + scratch_[p];
        apply_boundary_conditions(position_star_p_new, parameters_);
        position_stars_[p] = position_star_p_new;
      });
  };

  void update_velocities(IndexSpan span){
    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
          velocities_previous_[p] = velocities_[p];
//          Vec<Real,Dim> velocity{(static_cast<Real>(1.5)*position_stars_[p] - static_cast<Real>(2.0)*positions_[p] + static_cast<Real>(0.5)*positions_previous_[p] ) / parameters_.time_step()};
        Vec<Real,Dim> velocity{(position_stars_[p] - positions_[p])/parameters_.time_step()};
        // Clamp to maximum velocity
//        clamp_in_place(velocity, static_cast<Real>(-1.0)*parameters_.max_speed(), parameters_.max_speed());
        // Clamp to 0 if velocity under threshold
        if(magnitude_squared(velocity) < 0.000001 * parameters_.max_speed())
            velocity = Vec<Real,Dim>{0.0};

        velocities_[p] = velocity;
    });
  }

  void update_positions(IndexSpan span){
    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        positions_previous_[p]  = positions_[p];
        positions_[p] = position_stars_[p];
      });
  }

  void apply_surface_tension(IndexSpan color_field_span, IndexSpan surface_tension_span) {
    const Del_Spikey<Real,Dim> Del_W{parameters_.smoothing_radius()};
    const C_Spline<Real,Dim> C{parameters_.smoothing_radius()};

    // Compute gradient of color field
    sim::algorithms::for_each_index(color_field_span, [=] DEVICE_CALLABLE (std::size_t p) {
        Vec<Real,Dim> color{0.0};
        for(const std::size_t q : neighbors_[p]) {
          color += Del_W(position_stars_[p],  position_stars_[q]) / densities_[q];
        }
        scratch_[p] = parameters_.smoothing_radius() * color;
      });

    sim::algorithms::for_each_index(surface_tension_span, [=] DEVICE_CALLABLE (std::size_t p) {
        Vec<Real,Dim> surface_tension_force{0.0};

        for(const std::size_t q : neighbors_[p]) {
          const Vec<Real,Dim> r = position_stars_[p] - position_stars_[q];

          Real r_mag = magnitude(r);
          if(r_mag < parameters_.smoothing_radius()*0.000001)
            r_mag = parameters_.smoothing_radius()*0.000001;

          const Vec<Real,Dim> cohesion_force{-parameters_.gamma() * C(r_mag) * r/r_mag};
          const Vec<Real,Dim> curvature_force{-parameters_.gamma() * (scratch_[p] - scratch_[q])};
          const Real K = 2.0*parameters_.rest_density() / (densities_[p] + densities_[q]);
          surface_tension_force += K * (cohesion_force + curvature_force);
        }

        velocities_[p] += surface_tension_force / densities_[p] * parameters_.time_step();
//          position_stars_[p] += surface_tension_force / densities_[p] * parameters_.time_step()/3.0 * parameters_.time_step();
      });
  }

  void apply_viscosity(IndexSpan span) {
    const Poly6<Real,Dim> W{parameters_.smoothing_radius()};

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        Vec<Real,Dim> dv{0.0};
        for(const std::size_t q : neighbors_[p]) {
          const Real r_mag = magnitude(position_stars_[p] - position_stars_[q]);
          dv += (velocities_[q] - velocities_[p]) * W(r_mag) / densities_[q];
        }
        velocities_[p] += parameters_.visc_c() * dv;
      });
  }

  void compute_vorticity(IndexSpan span) {
    const Del_Spikey<Real,Dim> Del_W{parameters_.smoothing_radius()};

    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        Vec<Real,Dim> vorticity{0.0};

        for(const std::size_t q : neighbors_[p]) {
          const auto del = Del_W(position_stars_[p], position_stars_[q]);
          const auto v_diff = velocities_[q] - velocities_[p];
          vorticity += cross(v_diff, del);
        }

        scratch_[p] = vorticity;
      });
  }

  void apply_vorticity(IndexSpan span) {
    const Del_Spikey<Real,Dim> Del_W{parameters_.smoothing_radius()};
    // Scratch contains vorticity
    sim::algorithms::for_each_index(span, [=] DEVICE_CALLABLE (std::size_t p) {
        Vec<Real,Dim> eta{0.0};

        for(const std::size_t q : neighbors_[p]) {
          const auto del = Del_W(position_stars_[p], position_stars_[q]);
          const Real vorticity_magnitude = magnitude(scratch_[q]);
          eta += vorticity_magnitude * del;
        }

        const auto N = eta / (magnitude(eta) + std::numeric_limits<Real>::epsilon());
        velocities_[p] += cross(N, scratch_[p]) * parameters_.vorticity_coef() * parameters_.time_step();
      });

  }

// DEVICE_CALLABLE lambdas can't have private members
//private:
  const Parameters<Real,Dim>& parameters_;
  const std::size_t max_local_count_;
  Neighbors<Real,Dim> neighbors_;
  sim::Array< Vec<Real,Dim> > positions_;
  sim::Array< Vec<Real,Dim> > positions_previous_;
  sim::Array< Vec<Real,Dim> > position_stars_;
  sim::Array< Vec<Real,Dim> > velocities_;
  sim::Array< Vec<Real,Dim> > velocities_previous_;
  sim::Array< Real > densities_;
  sim::Array< Real > lambdas_;

  // Scratch values are used for delta_positions, vorticities, and color field
  // Don't overwrite data you depend on
  sim::Array< Vec<Real,Dim> > scratch_;
  sim::Array< Real > scratch_scalar_;
};

template<typename Real, Dimension Dim>
DEVICE_CALLABLE
void apply_boundary_conditions(Vec<Real,Dim>& position,
                               const Parameters<Real,Dim>& parameters) {
  // Push outside of mover sphere
  const float mover_radius = 0.2;
  const auto boundary = parameters.boundary();
  const Vec<Real,Dim> mover_center = parameters.mover_center();
  Real dr_squared = magnitude_squared(position - mover_center);
  if(dr_squared < mover_radius * mover_radius) {
    Real dr = sqrt(dr_squared);
    position += (mover_radius - dr) * (position - mover_center) / dr;
  }

  // Clamp inside boundary
  clamp_in_place(position, boundary.min, boundary.max);
}
}

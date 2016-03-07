//
//  Boost Parameters<Real,Dim> serilization and optimization code
//

#pragma once

#include "boost_aabb_optimizations.h"

/**
  boost Parameters<Real,Dim> serilizer
**/
namespace boost { namespace serialization {
template<class Archive, typename Real, Dimension Dim>
void serialize(Archive & ar, Parameters<Real,Dim> & params, const unsigned int version)
{
  ar & params.max_particles_local_;
  ar & params.initial_global_particle_count_;
  ar & params.solve_step_count_;
  ar & params.particle_rest_spacing_;
  ar & params.particle_radius_;
  ar & params.smoothing_radius_;
  ar & params.rest_density_;
  ar & params.rest_mass_;
  ar & params.gravity_;
  ar & params.gamma_;
  ar & params.lambda_epsilon_;
  ar & params.k_stiff_;
  ar & params.visc_c_;
  ar & params.time_step_;
  ar & params.max_speed_;
  ar & params.boundary_;
  ar & params.initial_fluid_;
  ar & params.simulation_mode_;
} }}

/**
  BOOST_IS_MPI_DATATYPE(Parameters<Real,Dim>)
**/
namespace boost { namespace mpi {
  template <typename Real, Dimension Dim>
  struct is_mpi_datatype< Parameters<Real,Dim> > : mpl::true_ { };
}}

/**
  BOOST_CLASS_TRACKING(Parameters<Real,Dim>,track_never)
**/
namespace boost { namespace serialization {
template <typename Real, Dimension Dim>
struct tracking_level< Parameters<Real,Dim> >
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<track_never> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = tracking_level::type::value
    );
}; }}

/**
  BOOST_CLASS_IMPLEMENTATION(Parameters<Real,Dim>, object_serializable)
**/
namespace boost { namespace serialization {
template <typename Real, Dimension Dim>
struct implementation_level< Parameters<Real,Dim> >
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<object_serializable> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = implementation_level::type::value
    );
}; }}

/**
BOOST_IS_BITWISE_SERIALIZABLE(Parameters<Real,Dim>)
**/
namespace boost { namespace serialization {
    template <typename Real, Dimension Dim>
    struct is_bitwise_serializable< Parameters<Real,Dim> > : mpl::true_ {};
} }

//
//  Boost AABB<Real,Dim> serilization and optimization code
//

#pragma once

#include "boost_vec_optimizations.h"

/**
  boost AABB<Real,Dim> serilizer
**/
namespace boost { namespace serialization {
template<class Archive, typename Real, Dimension Dim>
void serialize(Archive & ar, AABB<Real,Dim> & aabb, const unsigned int version)
{
  ar & aabb.min;
  ar & aabb.max;
} }}

/**
  BOOST_IS_MPI_DATATYPE(AABB<Real,Dim>)
**/
namespace boost { namespace mpi {
  template <typename Real, Dimension Dim>
  struct is_mpi_datatype< AABB<Real,Dim> > : mpl::true_ { };
}}

/**
  BOOST_CLASS_TRACKING(AABB<Real,Dim>,track_never)
**/
namespace boost { namespace serialization {
template <typename Real, Dimension Dim>
struct tracking_level< AABB<Real,Dim> >
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<track_never> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = tracking_level::type::value
    );
}; }}

/**
  BOOST_CLASS_IMPLEMENTATION(AABB<Real,Dim>, object_serializable)
**/
namespace boost { namespace serialization {
template <typename Real, Dimension Dim>
struct implementation_level< AABB<Real,Dim> >
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<object_serializable> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = implementation_level::type::value
    );
}; }}

/**
BOOST_IS_BITWISE_SERIALIZABLE(AABB<Real,Dim>)
**/
namespace boost { namespace serialization {
    template <typename Real, Dimension Dim>
    struct is_bitwise_serializable< AABB<Real,Dim> > : mpl::true_ {};
} }

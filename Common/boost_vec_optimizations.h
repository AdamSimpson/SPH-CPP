//
//  Boost Vec<Real,Dim> serilization and optimization code
//

#pragma once

/**
  boost Vec<Real,Dim> serilizer
**/
namespace boost { namespace serialization {
template<class Archive, typename Real, Dimension Dim>
void serialize(Archive & ar, Vec<Real,Dim> & vec, const unsigned int version)
{
    for(int i=0; i<Dim; ++i) {
      ar & vec[i];
    }
} }}


/**
  BOOST_IS_MPI_DATATYPE(Vec<Real,Dim>)
**/
namespace boost { namespace mpi {
  template <typename Real, Dimension Dim>
  struct is_mpi_datatype< Vec<Real,Dim> > : mpl::true_ { };
}}

/**
  BOOST_CLASS_TRACKING(Vec<Real,Dim>,track_never)
**/
namespace boost { namespace serialization {
template <typename Real, Dimension Dim>
struct tracking_level< Vec<Real,Dim> >
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<track_never> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = tracking_level::type::value
    );
}; }}

/**
  BOOST_CLASS_IMPLEMENTATION(Vec<Real,Dim>, object_serializable)
**/
namespace boost { namespace serialization {
template <typename Real, Dimension Dim>
struct implementation_level< Vec<Real,Dim> >
{
    typedef mpl::integral_c_tag tag;
    typedef mpl::int_<object_serializable> type;
    BOOST_STATIC_CONSTANT(
        int,
        value = implementation_level::type::value
    );
}; }}

/**
BOOST_IS_BITWISE_SERIALIZABLE(Vec<Real,Dim>)
**/
namespace boost { namespace serialization {
    template <typename Real, Dimension Dim>
    struct is_bitwise_serializable< Vec<Real,Dim> > : mpl::true_ {};
} }

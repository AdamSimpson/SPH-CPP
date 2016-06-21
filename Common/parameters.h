#pragma once

#include "managed_allocation.h"
#include "execution_mode.h"
#include "dimension.h"
#include "vec.h"
#include "aabb.h"
#include "device.h"

#include <cmath>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace sim {
// Forward declaration
template<typename Real, Dimension Dim>
Vec<Real,Dim> to_real_vec(const std::string& input_string);

/**
  @brief Simulation wide tunable parameters
**/
template<typename Real, Dimension Dim>
class Parameters: public ManagedAllocation {
public:

  enum Mode {
    EDIT_VIEW      = (1 << 0),
    EMITTER_ACTIVE = (1 << 1),
    EDIT_EMITTER   = (1 << 2),
    EDIT_MOVER     = (1 << 3),
    PAUSE_COMPUTE  = (1 << 4),
    EXIT           = (1 << 5),
  };

  /**
    @brief  Construct initial parameters from file_name .INI file
  **/
  Parameters(const std::string& file_name): simulation_mode_{Mode::EDIT_VIEW},
                                            execution_mode_{ExecutionMode::GPU} {
    this->read_INI(file_name);
    this->derive_from_input();
  };

  Parameters()                             = default;
  ~Parameters()                            = default;
  Parameters(const Parameters&)            = default;
  Parameters& operator=(const Parameters&) = default;
  Parameters(Parameters&&) noexcept        = default;
  Parameters& operator=(Parameters&&)      = default;

  /**
    @brief Read file_name .INI containing parameters
  **/
  void read_INI(const std::string& file_name) {
    boost::property_tree::ptree property_tree;
    boost::property_tree::ini_parser::read_ini(file_name, property_tree);

    solve_step_count_ = property_tree.get<std::size_t>("SimParameters.number_solve_steps");
    time_step_ = property_tree.get<Real>("SimParameters.time_step");
    initial_global_particle_count_ = property_tree.get<std::size_t>("SimParameters.global_particle_count");
    max_particles_local_ = property_tree.get<std::size_t>("SimParameters.max_particles_local");
    gravity_ = property_tree.get<Real>("PhysicalParameters.g");
    gamma_ = property_tree.get<Real>("PhysicalParameters.gamma");
    visc_c_ = property_tree.get<Real>("PhysicalParameters.visc_c");
    lambda_epsilon_ = property_tree.get<Real>("PhysicalParameters.lambda_epsilon");
    k_stiff_ = property_tree.get<Real>("PhysicalParameters.k_stiff");
    rest_density_ = property_tree.get<Real>("PhysicalParameters.density");
    vorticity_coef_ = property_tree.get<Real>("PhysicalParameters.vorticity_coef");

    boundary_.min = to_real_vec<Real,Dim>(property_tree.get<std::string>("Boundary.min"));
    boundary_.max = to_real_vec<Real,Dim>(property_tree.get<std::string>("Boundary.max"));

    initial_fluid_.min = to_real_vec<Real,Dim>(property_tree.get<std::string>("InitialFluid.min"));
    initial_fluid_.max = to_real_vec<Real,Dim>(property_tree.get<std::string>("InitialFluid.max"));

    mover_center_ = to_real_vec<Real,Dim>(property_tree.get<std::string>("Mover.center"));
  }

  /**
    @brief Derive additional parameters from .INI parameters required for simulation
  **/
  void derive_from_input() {
    particle_rest_spacing_ = pow(initial_fluid_.volume() / initial_global_particle_count_, 1.0/Dim);
    particle_radius_ = particle_rest_spacing_/2.0;
    smoothing_radius_ = 1.8*particle_rest_spacing_;

    Vec<std::size_t,Dim> particle_counts = bin_count_in_volume(initial_fluid_, particle_rest_spacing_);
    std::size_t particle_count = product(particle_counts);
    Real mass_fudge = 1.0;
    rest_mass_ = 1.0; //mass_fudge * initial_fluid_.volume() * rest_density_ / particle_count;
    Real particle_volume = /*4.0/3.0 * M_PI **/ pow(particle_rest_spacing_ , 3.0);
    rest_density_ = rest_mass_ / particle_volume;

    emitter_center_ = boundary_.center();
    emitter_velocity_ = Vec<Real,Dim>{1.0, 0.0, 0.0};

    std::cout<<"emitter_center: "<<emitter_center_<<std::endl;

    std::cout<<"Max speed must be reset if smoothing radius changes\n";
    max_speed_ = 0.5*smoothing_radius_*solve_step_count_ / time_step_;

    // @todo params print function to print them all
    std::cout<<"rest_spacing: "<<particle_rest_spacing_<<std::endl;
    std::cout<<"smootihng length: "<<smoothing_radius_<<std::endl;
    std::cout<<"particle count: "<<particle_count<<std::endl;
    std::cout<<"rest mass: "<<rest_mass_<<std::endl;
    std::cout<<"density: "<<rest_density_<<std::endl;
    std::cout<<"max speed: "<<max_speed_<<std::endl;
    std::cout<<"time step: "<<time_step_<<std::endl;
  }

  /**
    @return maximum node level particle count
  **/
  DEVICE_CALLABLE
  std::size_t max_particles_local() const {
    return max_particles_local_;
  }

  /**
    Set Initial requested global particle count
  **/
//  void set_initial_global_particle_count(std::size_t global_count) {
//    initial_global_particle_count_ = global_count;
//  }

  /**
    Get Initial requested global particle count
  **/
  DEVICE_CALLABLE
  std::size_t initial_global_particle_count() const {
    return initial_global_particle_count_;
  }

  /**
    @return Initial fluid configuration as AABB
  **/
  DEVICE_CALLABLE
  const AABB<Real,Dim>& initial_fluid() const {
    return initial_fluid_;
  }

  /**
    @return Global boundary as AABB
  **/
  DEVICE_CALLABLE
  const AABB<Real,Dim>& boundary() const {
    return boundary_;
  }

  /**
    @return Rest spacing of particles
  **/
  DEVICE_CALLABLE
  Real particle_rest_spacing() const {
    return particle_rest_spacing_;
  }

  /**
    @return smoothing radius for particles
  **/
  DEVICE_CALLABLE
  Real smoothing_radius() const {
    return smoothing_radius_;
  }

  DEVICE_CALLABLE
  void increase_smoothing_radius() {
    smoothing_radius_ += 0.1;
  }

  DEVICE_CALLABLE
  void decrease_smoothing_radius() {
    smoothing_radius_ -= 0.1;
  }

  /**
    @return Gravitational acceleration magnitude
  **/
  DEVICE_CALLABLE
  Real gravity() const {
    return gravity_;
  }

  DEVICE_CALLABLE
  void increase_gravity() {
    gravity_ += 0.5;
  }

  DEVICE_CALLABLE
  void decrease_gravity() {
    gravity_ -= 0.5;
  }

  /**
    @return particle radius
  **/
  DEVICE_CALLABLE
  Real particle_radius() const {
    return particle_radius_;
  }

  /**
    @return time step size
  **/
  DEVICE_CALLABLE
  Real time_step() const {
    return time_step_;
  }

  /**
     @return solve step count
   **/
  DEVICE_CALLABLE
  std::size_t solve_step_count() const {
    return solve_step_count_;
  }

  DEVICE_CALLABLE
  Real rest_mass() const {
    return rest_mass_;
  }

  DEVICE_CALLABLE
  Real rest_density() const {
    return rest_density_;
  }

  DEVICE_CALLABLE
  void increase_rest_density() {
    rest_density_ += 50.0;
  }

  DEVICE_CALLABLE
  void decrease_rest_density() {
    rest_density_ -=50.0;
  }

  DEVICE_CALLABLE
  Real lambda_epsilon() const {
    return lambda_epsilon_;
  }

  DEVICE_CALLABLE
  Real k_stiff() const {
    return k_stiff_;
  }

  DEVICE_CALLABLE
  Real max_speed() const {
    return max_speed_;
  }

  DEVICE_CALLABLE
  Real gamma() const {
    return gamma_;
  }

  DEVICE_CALLABLE
  void increase_gamma() {
    gamma_ += 100.0;
  }

  DEVICE_CALLABLE
  void decrease_gamma() {
    gamma_ -= 100.0;
  }

  DEVICE_CALLABLE
  Real visc_c() const {
    return visc_c_;
  }

  DEVICE_CALLABLE
  void increase_visc_c() {
    visc_c_ += 0.01;
  }

  DEVICE_CALLABLE
  void decrease_visc_c() {
    visc_c_ -= 0.01;
  }

  DEVICE_CALLABLE
  Real vorticity_coef() const {
    return vorticity_coef_;
  }

  DEVICE_CALLABLE
  bool simulation_active() const {
    return !(simulation_mode_ & Mode::EXIT);
  }

  DEVICE_CALLABLE
  void exit_simulation() {
    simulation_mode_ = Mode::EXIT;
  }

  DEVICE_CALLABLE
  bool compute_active() const {
    return !(simulation_mode_ & Mode::PAUSE_COMPUTE) && !(simulation_mode_ & Mode::EXIT);
  }

  DEVICE_CALLABLE
  ExecutionMode execution_mode() const {
    return execution_mode_;
  }

  DEVICE_CALLABLE
  void toggle_computation_active() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::PAUSE_COMPUTE);
  }

  DEVICE_CALLABLE
  void enable_gpu_execution_mode() {
    execution_mode_ = ExecutionMode::GPU;
  }

  DEVICE_CALLABLE
  void enable_cpu_execution_mode() {
    execution_mode_ = ExecutionMode::CPU;
  }

  DEVICE_CALLABLE
  void toggle_emitter_active() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EMITTER_ACTIVE);
  }

  DEVICE_CALLABLE
  void toggle_view_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EDIT_VIEW);
  }

  DEVICE_CALLABLE
  void disable_view_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ & ~Mode::EDIT_VIEW);
  }

  DEVICE_CALLABLE
  void enable_view_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ | Mode::EDIT_VIEW);
  }

  DEVICE_CALLABLE
  bool view_edit() const {
    return simulation_mode_ & Mode::EDIT_VIEW;
  }

  DEVICE_CALLABLE
  bool emitter_active() const {
    return simulation_mode_ & Mode::EMITTER_ACTIVE;
  }

  DEVICE_CALLABLE
  bool edit_emitter() const {
    return simulation_mode_ & Mode::EDIT_EMITTER;
  }

  DEVICE_CALLABLE
  void toggle_emitter_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EDIT_EMITTER);
  }

  DEVICE_CALLABLE
  void disable_emitter_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ & ~Mode::EDIT_EMITTER);
  }

  DEVICE_CALLABLE
  void enable_emitter_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ | Mode::EDIT_EMITTER);
  }

  DEVICE_CALLABLE
  const Vec<Real,Dim>& emitter_center() const {
    return emitter_center_;
  }

  DEVICE_CALLABLE
  const Vec<Real,Dim>& emitter_velocity() const {
    return emitter_velocity_;
  }

  DEVICE_CALLABLE
  const Vec<Real,Dim>& mover_center() const {
    return mover_center_;
  }

  DEVICE_CALLABLE
  void toggle_mover_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EDIT_MOVER);
  }

  DEVICE_CALLABLE
  bool edit_mover() const {
    return simulation_mode_ & Mode::EDIT_MOVER;
  }

//private:
  // Not private so we boost mpi helpers work
  // @todo is this monster parameters struct reasonable?
  std::size_t max_particles_local_;
  std::size_t initial_global_particle_count_;
  std::size_t solve_step_count_;
  Real particle_rest_spacing_;
  Real particle_radius_;
  Real smoothing_radius_;
  Real rest_density_;
  Real rest_mass_;
  Real gravity_;
  Real gamma_;
  Real lambda_epsilon_;
  Real k_stiff_;
  Real visc_c_;
  Real time_step_;
  Real max_speed_;
  Real vorticity_coef_;
  AABB<Real,Dim> boundary_;
  AABB<Real,Dim> initial_fluid_;
  Mode simulation_mode_;
  ExecutionMode execution_mode_;
  Vec<Real,Dim> emitter_center_;
  Vec<Real,Dim> emitter_velocity_;
  Vec<Real,Dim> mover_center_;
};

/**
  @brief Returns a Vec<> from comma seperated input string
**/
template<typename Real, Dimension Dim>
Vec<Real,Dim> to_real_vec(const std::string& input_string) {
  Vec<Real,Dim> result;
  std::stringstream ss(input_string);
  std::string item;

  for(int i=0; i<Dim; ++i) {
    std::getline(ss, item, ',');
    boost::algorithm::trim(item);
    result[i] = (boost::lexical_cast<Real>(item));
  }

  return result;
}
}

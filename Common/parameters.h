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

/*! Simulation wide tunable parameters
 */
template<typename Real, Dimension Dim>
class Parameters: public ManagedAllocation {
public:

  enum Mode {
    EDIT_VIEW      = (1 << 0), /**< User input modifies visulization view **/
    EMITTER_ACTIVE = (1 << 1), /**< Fluid emitter is active(emitting fluid **/
    EDIT_EMITTER   = (1 << 2), /**< User input modifies emitter **/
    EDIT_MOVER     = (1 << 3), /**< User input modifies mover **/
    PAUSE_COMPUTE  = (1 << 4), /**< Computation is paused **/
    EXIT           = (1 << 5), /**< Simulation will exit cleanly **/
  }; /**< enum Mode to describe the state of the application, mostly used for interactive rendering **/

  /*! Construct initial parameters from file_name .INI file
   * @param file_name the .ini parameters file
   */
  Parameters(const std::string& file_name): simulation_mode_{Mode::EDIT_VIEW},
                                            execution_mode_{ExecutionMode::GPU} {
    this->read_INI(file_name);
    this->derive_from_input();
  };

  /*! Default constructor
   */
  Parameters()                             = default;
  /*! Default destructor
   */
  ~Parameters()                            = default;
  /*! Default copy constructor
   */
  Parameters(const Parameters&)            = default;
  /*! Default copy assignment operator
   */
  Parameters& operator=(const Parameters&) = default;
  /*! Default move constructor
   */
  Parameters(Parameters&&) noexcept        = default;
  /*! Default move assignment operator
   */
  Parameters& operator=(Parameters&&)      = default;

  /*! Read file_name .INI containing parameters
   * @param filename the .ini paramaters file to read
   */
  void read_INI(const std::string& file_name) {
    boost::property_tree::ptree property_tree;
    boost::property_tree::ini_parser::read_ini(file_name, property_tree);

    solve_step_count_ = property_tree.get<std::size_t>("SimParameters.number_solve_steps", -1);
    time_step_ = property_tree.get<Real>("SimParameters.time_step", -1.0);
    initial_global_particle_count_ = property_tree.get<std::size_t>("SimParameters.global_particle_count", -1);
    max_particles_local_ = property_tree.get<std::size_t>("SimParameters.max_particles_local", -1);
    neighbor_bin_spacing_ = property_tree.get<Real>("SimParameters.neighbor_bin_spacing", -1.0);

    gravity_ = property_tree.get<Real>("PhysicalParameters.g", -1.0);
    gamma_ = property_tree.get<Real>("PhysicalParameters.gamma", -1.0);
    visc_c_ = property_tree.get<Real>("PhysicalParameters.visc_c", -1.0);
    lambda_epsilon_ = property_tree.get<Real>("PhysicalParameters.lambda_epsilon", -1.0);
    k_stiff_ = property_tree.get<Real>("PhysicalParameters.k_stiff", -1.0);
    rest_density_ = property_tree.get<Real>("PhysicalParameters.density", -1.0);
    vorticity_coef_ = property_tree.get<Real>("PhysicalParameters.vorticity_coef", -1.0);
    smoothing_radius_ = property_tree.get<Real>("PhysicalParameters.smoothing_radius", -1.0);
    particle_rest_spacing_ = property_tree.get<Real>("PhysicalParameters.rest_spacing", -1.0);

    boundary_.min = to_real_vec<Real,Dim>(property_tree.get<std::string>("Boundary.min", "0.0, 0.0, 0.0"));
    boundary_.max = to_real_vec<Real,Dim>(property_tree.get<std::string>("Boundary.max", "0.0, 0.0, 0.0"));

    initial_fluid_.min = to_real_vec<Real,Dim>(property_tree.get<std::string>("InitialFluid.min", "0.0, 0.0, 0.0"));
    initial_fluid_.max = to_real_vec<Real,Dim>(property_tree.get<std::string>("InitialFluid.max", "0.0, 0.0, 0.0"));

    mover_center_ = to_real_vec<Real,Dim>(property_tree.get<std::string>("Mover.center", "0.0, 0.0, 0.0"));
  }

  /*! Derive additional parameters from .INI parameters required for simulation
   */
  void derive_from_input() {
    if(particle_rest_spacing_ <= 0.0)
      particle_rest_spacing_ = pow(initial_fluid_.volume() / initial_global_particle_count_, 1.0 / Dim);

    particle_radius_ = particle_rest_spacing_ / 2.0;

    if(smoothing_radius_ <= 0.0)
      smoothing_radius_ = 1.8*particle_rest_spacing_;

    if(neighbor_bin_spacing_ <= 0.0)
      neighbor_bin_spacing_ = 1.2*smoothing_radius_;

//    Vec<std::size_t,Dim> particle_counts = bin_count_in_volume(initial_fluid_, particle_rest_spacing_);
//    std::size_t particle_count = product(particle_counts);
    rest_mass_ = 1.0; //mass_fudge * initial_fluid_.volume() * rest_density_ / particle_count;
    Real particle_volume = /*4.0/3.0 * M_PI **/ std::pow(particle_rest_spacing_ , (Real)Dim);
    rest_density_ = rest_mass_ / particle_volume;

    emitter_center_ = boundary_.center();
    emitter_velocity_ = Vec<Real,Dim>{0.0};

    // Max speed must be reset if smoothing radius changes
    max_speed_ = 0.5*smoothing_radius_*solve_step_count_ / time_step_;

    // @todo params print function to print them all
  }

  /*! Maximum local particles getter
    @return maximum node level particle count
   */
  DEVICE_CALLABLE
  std::size_t max_particles_local() const {
    return max_particles_local_;
  }

  /*! Initially requested global particle count getter
    @return the initial global particle count, which may differ from the constructed particle count
   */
  DEVICE_CALLABLE
  std::size_t initial_global_particle_count() const {
    return initial_global_particle_count_;
  }

  /*! Initial fluid configuration AABB getter
    @return Initial fluid configuration axis aligned bounding box
   */
  DEVICE_CALLABLE
  const AABB<Real,Dim>& initial_fluid() const {
    return initial_fluid_;
  }

  /*! Global boundary AABB getter
    @return Global boundary as AABB
   */
  DEVICE_CALLABLE
  const AABB<Real,Dim>& boundary() const {
    return boundary_;
  }

  /*! Particle rest spacing getter
    @return Rest spacing of particles
   */
  DEVICE_CALLABLE
  Real particle_rest_spacing() const {
    return particle_rest_spacing_;
  }

  /*! Particle smoothing radius getter
    @return smoothing radius for particles
   */
  DEVICE_CALLABLE
  Real smoothing_radius() const {
    return smoothing_radius_;
  }

  /*! Neighbor grid bin spacing getter
    @return neighbor bin grid spacing(bin heigth/width)
   */
  DEVICE_CALLABLE
  Real neighbor_bin_spacing() const {
    return neighbor_bin_spacing_;
  }

  /*! Increase particle smoothing radius
   */
  DEVICE_CALLABLE
  void increase_smoothing_radius() {
    smoothing_radius_ += 0.1;
  }

  /*! Decrease particle smoothing radius
   */
  DEVICE_CALLABLE
  void decrease_smoothing_radius() {
    smoothing_radius_ -= 0.1;
  }

  /*! Gravitational acceleration getter
    @return Gravitational acceleration magnitude
   */
  DEVICE_CALLABLE
  Real gravity() const {
    return gravity_;
  }

  /*! Increase gravitational acceleration magnitude
   */
  DEVICE_CALLABLE
  void increase_gravity() {
    gravity_ += 0.5;
  }

  /*! Decrease gravitational acceleration magnitude
   */
  DEVICE_CALLABLE
  void decrease_gravity() {
    gravity_ -= 0.5;
  }

  /*! Particle radius getter
    @return particle radius
   */
  DEVICE_CALLABLE
  Real particle_radius() const {
    return particle_radius_;
  }

  /*! Time step getter
    @return simulation time step
  **/
  DEVICE_CALLABLE
  Real time_step() const {
    return time_step_;
  }

  /*! PBD solver step count getter
     @return PBD solver step count
   **/
  DEVICE_CALLABLE
  std::size_t solve_step_count() const {
    return solve_step_count_;
  }

  /*! Particle target rest mass getter
     @return Particle target rest mass
   */
  DEVICE_CALLABLE
  Real rest_mass() const {
    return rest_mass_;
  }

  /*! Particle target rest density getter
     @return Particle target rest density
   */
  DEVICE_CALLABLE
  Real rest_density() const {
    return rest_density_;
  }

  /*! Increase target rest density
   */
  DEVICE_CALLABLE
  void increase_rest_density() {
    rest_density_ += 50.0;
  }

  /*! Decrease target rest density
   */
  DEVICE_CALLABLE
  void decrease_rest_density() {
    rest_density_ -= 50.0;
  }

  /*! Lambda epsilon getter
   * @return the CFM epsilon value used in lambda calculation
   */
  DEVICE_CALLABLE
  Real lambda_epsilon() const {
    return lambda_epsilon_;
  }

  /*! K stiffness getter
   * @return K stiffness value
   */
  DEVICE_CALLABLE
  Real k_stiff() const {
    return k_stiff_;
  }

  /*! Maximum particle speed getter
   * Simulation wide maximum speed, ~CFL
   */
  DEVICE_CALLABLE
  Real max_speed() const {
    return max_speed_;
  }

  /*! Surface tension gamma getter
   * @return surface tension gamma value
   */
  DEVICE_CALLABLE
  Real gamma() const {
    return gamma_;
  }

  /*! Increase surface tension gamma
   */
  DEVICE_CALLABLE
  void increase_gamma() {
    gamma_ += 100.0;
  }

  /*! Decrease surface tension gamma
   */
  DEVICE_CALLABLE
  void decrease_gamma() {
    gamma_ -= 100.0;
  }

  /*! Viscosity coefficient getter
   * @return viscosity coefficient
   */
  DEVICE_CALLABLE
  Real visc_c() const {
    return visc_c_;
  }

  /*! Increase viscosity coefficient
   */
  DEVICE_CALLABLE
  void increase_visc_c() {
    visc_c_ += 0.01;
  }

  /*! Decrease viscosity coefficient
   */
  DEVICE_CALLABLE
  void decrease_visc_c() {
    visc_c_ -= 0.01;
  }

  /*! Vorticity coefficient getter
   */
  DEVICE_CALLABLE
  Real vorticity_coef() const {
    return vorticity_coef_;
  }

  /*! Is the application active(not exiting)?
   * @return true if the simulation is not requesting to exit
   */
  DEVICE_CALLABLE
  bool simulation_active() const {
    return !(simulation_mode_ & Mode::EXIT);
  }

  /*! Is the application requesting to exit?
   * @return true if simulation has requested to exit
   */
  DEVICE_CALLABLE
  void exit_simulation() {
    simulation_mode_ = Mode::EXIT;
  }

  /*! Is the compute portion of the simulation actively running?
   * @return true if the simulation isn't paused or requesting exit
   */
  DEVICE_CALLABLE
  bool compute_active() const {
    return !(simulation_mode_ & Mode::PAUSE_COMPUTE) && !(simulation_mode_ & Mode::EXIT);
  }

  /*! Pause the compute portion of the simulation
   */
  DEVICE_CALLABLE
  void pause_compute() {
    simulation_mode_ = (Mode) (simulation_mode_ | Mode::PAUSE_COMPUTE);
  }

  /*! Activate the compute portion of the simulation
   */
  DEVICE_CALLABLE
  void activate_compute() {
    simulation_mode_ = (Mode) (simulation_mode_ & ~Mode::PAUSE_COMPUTE);
  }

  /*! Toggle the paused state of the simulation
   */
  DEVICE_CALLABLE
  void toggle_compute_paused() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::PAUSE_COMPUTE);
  }

  /*! Simulation execution mode getter
   * @return The execution mode of the application
   */
  DEVICE_CALLABLE
  ExecutionMode execution_mode() const {
    return execution_mode_;
  }

  /*! Enable GPU backend execution of sim_algorithms
   */
  DEVICE_CALLABLE
  void enable_gpu_execution_mode() {
    execution_mode_ = ExecutionMode::GPU;
  }

  /*! Enable CPU backend execution of sim_algorithms
   */
  DEVICE_CALLABLE
  void enable_cpu_execution_mode() {
    execution_mode_ = ExecutionMode::CPU;
  }

  /*! Toggle active fluid emitter
   */
  DEVICE_CALLABLE
  void toggle_emitter_active() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EMITTER_ACTIVE);
  }

  /*! Is the emitter active?
   * @return true if fluid particle emitter active
   */
  DEVICE_CALLABLE
  bool emitter_active() const {
    return simulation_mode_ & Mode::EMITTER_ACTIVE;
  }

  /*! Is the emitter state editable?
   * @return true if the fluid emitter is currently editable
   */
  DEVICE_CALLABLE
  bool edit_emitter() const {
    return simulation_mode_ & Mode::EDIT_EMITTER;
  }

  /*! Toggle the edit state of emitter
   */
  DEVICE_CALLABLE
  void toggle_edit_emitter() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EDIT_EMITTER);
  }

  /*! Disable the emitter edit state
   */
  DEVICE_CALLABLE
  void disable_edit_emitter() {
    simulation_mode_ = (Mode) (simulation_mode_ & ~Mode::EDIT_EMITTER);
  }

  /*! Enable the emitter edit state
   */
  DEVICE_CALLABLE
  void enable_edit_emitter() {
    simulation_mode_ = (Mode) (simulation_mode_ | Mode::EDIT_EMITTER);
  }

  /*! Toggle if the render view is editable
   */
  DEVICE_CALLABLE
  void toggle_edit_view() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EDIT_VIEW);
  }

  /*! Disable the render view being editable
   */
  DEVICE_CALLABLE
  void disable_edit_view() {
    simulation_mode_ = (Mode) (simulation_mode_ & ~Mode::EDIT_VIEW);
  }

  /*! Enable the render view being editable
   */
  DEVICE_CALLABLE
  void enable_edit_view() {
    simulation_mode_ = (Mode) (simulation_mode_ | Mode::EDIT_VIEW);
  }

  /*! Is the render view currently editable?
   * @return true if the render view is currently editable
   */
  DEVICE_CALLABLE
  bool edit_view() const {
    return simulation_mode_ & Mode::EDIT_VIEW;
  }

  /*! The emitter center point
   * @return a point describing the center of the fluid emitter source
   */
  DEVICE_CALLABLE
  const Vec<Real,Dim>& emitter_center() const {
    return emitter_center_;
  }

  /*! Emited particle velocity
   * @return a vector describing the emitted particles initial velocity
   */
  DEVICE_CALLABLE
  const Vec<Real,Dim>& emitter_velocity() const {
    return emitter_velocity_;
  }

  /*! Particle mover center
   * @return a point describing the center of the mover object
   */
  DEVICE_CALLABLE
  const Vec<Real,Dim>& mover_center() const {
    return mover_center_;
  }

  /*! Toggle the mover object being editable
   */
  DEVICE_CALLABLE
  void toggle_mover_edit() {
    simulation_mode_ = (Mode) (simulation_mode_ ^ Mode::EDIT_MOVER);
  }

  /*! Is the mover editable?
   * @return true if the mover is editable
   */
  DEVICE_CALLABLE
  bool edit_mover() const {
    return simulation_mode_ & Mode::EDIT_MOVER;
  }

  std::size_t max_particles_local_;           /**< Maximum particle count per process **/
  std::size_t initial_global_particle_count_; /**< Initially requested global particle count**/
  std::size_t solve_step_count_;              /**<  PBD solver steps per time step **/
  Real particle_rest_spacing_;                /**<  Particle rest spacing **/
  Real particle_radius_;                      /**<  Particle rest radius **/
  Real smoothing_radius_;                     /**<  SPH particle smoothing radius **/
  Real neighbor_bin_spacing_;                 /**<  Neighbor grid bin dimension **/
  Real rest_density_;                         /**<  Particle rest density **/
  Real rest_mass_;                            /**<  Particle rest mass **/
  Real gravity_;                              /**<  Gravity magnitude **/
  Real gamma_;                                /**<  Surface tension gamma coefficient **/
  Real lambda_epsilon_;                       /**<  Constraint force mixing epsillion for PDB lambdas **/
  Real k_stiff_;                              /**<  K_stiff **/
  Real visc_c_;                               /**<  Viscoscity coefficient **/
  Real time_step_;                            /**<  Simulation time step **/
  Real max_speed_;                            /**<  Maximum particle speed, based upon CFL **/
  Real vorticity_coef_;                       /**<  Vorticity coefficient **/
  AABB<Real,Dim> boundary_;                   /**<  Global boundary AABB **/
  AABB<Real,Dim> initial_fluid_;              /**<  Initial fluid AABB **/
  Mode simulation_mode_;                      /**<  Application mode **/
  ExecutionMode execution_mode_;              /**<  Simulation compute mode **/
  Vec<Real,Dim> emitter_center_;              /**<  Fluid emitter center **/
  Vec<Real,Dim> emitter_velocity_;            /**<  Fluid emitter particle velocity **/
  Vec<Real,Dim> mover_center_;                /**<  Mover ball center **/
};

/*!  fill a Vec<> from comma seperated input string
 * @return a Vec with the values specified by the input string
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

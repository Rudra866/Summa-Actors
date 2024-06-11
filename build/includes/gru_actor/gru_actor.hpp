#pragma once
#include "caf/all.hpp"
#include "fortran_data_types.hpp"
#include "settings_functions.hpp"
// #include "hru_actor.hpp"
#include <vector>

extern "C" {
  void getNumHRU(int& index_gru, int& num_hru);

  void initGRU_fortran(int& index_gru, void* gru_data, int& err, void* message);
  void setupGRU_fortran(int& index_gru, void* gru_data, int& err, 
                        void* message);
  void readGRURestart_fortran(int& index_gru, void* gru_data, int& err, 
                              void* message);
  void setTimeZoneOffsetGRU_fortran(int& iFile, void* gru_data, int& err, 
                                    void* message);
  void readGRUForcing_fortran(int& index_gru, int& iStep, int& iRead, 
                              int& iFile, void* gru_data, int& err, 
                              void* message);
  void runGRU_fortran(int& index_gru, int& timestep, void* gru_data, 
                      int& dt_init_factor, int& err, void* message);

  void writeGRUOutput_fortran(int& index_gru, int& timestep, int& output_step, 
                              void* gru_data, int& err, void* message);

}


class GruActor {
  caf::event_based_actor* self_;
  int netcdf_index_;
  int job_index_;
  HRUActorSettings hru_actor_settings_;
  caf::actor file_access_actor_;
  caf::actor parent_;

  int num_hrus_;
  std::vector<void*> hrus_;
  void* bvar_stat_ = new_handle_var_dlength();
  void* bvar_struct_ = new_handle_var_dlength();

  void* gru_data_;

  double dt_init_ = 0.0;
  int dt_init_factor_ = 1;
  int num_steps_until_write_;
  int num_steps_ = 0;                    // number of time steps
  int timestep_ = 1;	                   // Current Timestep of HRU simulation
  int iFile_ = 1;
  int stepsInCurrentFFile_;             // number of time steps in current forcing file
  int forcingStep_ = 1;                 // index of current time step in current forcing file
  int output_structure_step_index_ = 1; // index of current time step in output structure

  public:
    GruActor(caf::event_based_actor* self, int netcdf_index, int job_index, 
             int num_steps, HRUActorSettings hru_actor_settings,
             caf::actor file_access_actor, caf::actor parent) 
             : self_(self), netcdf_index_(netcdf_index), job_index_(job_index), 
               num_steps_(num_steps), hru_actor_settings_(hru_actor_settings),
               file_access_actor_(file_access_actor), parent_(parent) {};

    caf::behavior make_behavior();

};


// struct gru_actor_state {
//   int netcdf_index;
//   int gru_job_index;
//   HRU_Actor_Settings hru_actor_settings;
//   caf::actor file_access_actor;
//   caf::actor parent;

//   int num_hrus;
//   std::vector<void*> hrus;
//   void* bvar_stat = new_handle_var_dlength();
//   void* bvar_struct = new_handle_var_dlength();

//   void* gru_data;

//   double dt_init = 0.0;
//   int dt_init_factor = 1;
//   int num_steps_until_write;
//   int num_steps = 0;                    // number of time steps
//   int timestep = 1;	                   // Current Timestep of HRU simulation
//   int iFile = 1;
// 	int stepsInCurrentFFile;             // number of time steps in current forcing file
//   int forcingStep = 1;                 // index of current time step in current forcing file
//   int output_structure_step_index = 1; // index of current time step in output structure

// };

// caf::behavior gru_actor(caf::stateful_actor<gru_actor_state>* self, 
//                         int netcdf_index, int gru_job_index, int num_steps,
//                         HRU_Actor_Settings hru_actor_settings,
//                         caf::actor file_access_actor, caf::actor parent);
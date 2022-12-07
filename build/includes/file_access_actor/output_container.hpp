#pragma once

#include "caf/actor.hpp"
#include <optional>
#include <cmath>
#include "fortran_data_types.hpp"
#include <vector>
#include <iostream>



struct hru_output_handles {
    // Statistic Structures
    void* handle_forc_stat        = new_handle_var_dlength();
    void* handle_prog_stat        = new_handle_var_dlength();
    void* handle_diag_stat        = new_handle_var_dlength();
    void* handle_flux_stat        = new_handle_var_dlength();
    void* handle_indx_stat        = new_handle_var_dlength();
    void* handle_bvar_stat        = new_handle_var_dlength();
    // primary data structures (scalars)
    void* handle_time_struct      = new_handle_var_i();
    void* handle_forc_struct      = new_handle_var_d();
    void* handle_attr_struct      = new_handle_var_d();
    void* handle_type_struct      = new_handle_var_i();
    void* handle_id_struct        = new_handle_var_i8();
    // primary data structures (variable length vectors)
    void* handle_indx_struct      = new_handle_var_ilength();
    void* handle_mpar_struct      = new_handle_var_dlength();
    void* handle_prog_struct      = new_handle_var_dlength();
    void* handle_diag_struct      = new_handle_var_dlength();
    void* handle_flux_struct      = new_handle_var_dlength();
    // basin-average structures
    void* handle_bpar_struct      = new_handle_var_d();
    void* handle_bvar_struct      = new_handle_var_dlength();
    // ancillary data structures
    void* handle_dpar_struct      = new_handle_var_d();
    void* handle_finalize_stats   = new_handle_var_i();
    void* handle_output_timestep  = new_handle_var_i();

    ~hru_output_handles() {
        // statistics structures
        delete_handle_var_dlength(handle_forc_stat);
        delete_handle_var_dlength(handle_prog_stat);
        delete_handle_var_dlength(handle_diag_stat);
        delete_handle_var_dlength(handle_flux_stat);
        delete_handle_var_dlength(handle_indx_stat);
        delete_handle_var_dlength(handle_bvar_stat);
        // primary data structures (scalars)
        delete_handle_var_i(handle_time_struct);
        delete_handle_var_d(handle_forc_struct);
        delete_handle_var_d(handle_attr_struct);
        delete_handle_var_i(handle_type_struct);
        delete_handle_var_i8(handle_id_struct);
        // primary data structures (variable length vectors)
        delete_handle_var_ilength(handle_indx_struct);
        delete_handle_var_dlength(handle_mpar_struct);
        delete_handle_var_dlength(handle_prog_struct);
        delete_handle_var_dlength(handle_diag_struct);
        delete_handle_var_dlength(handle_flux_struct);
        // basin-average structures
        delete_handle_var_d(handle_bpar_struct);
        delete_handle_var_dlength(handle_bvar_struct);
        // ancillary data structures
        delete_handle_var_d(handle_dpar_struct);
        // counter variables
        delete_handle_var_i(handle_output_timestep);
        delete_handle_flagVec(handle_finalize_stats);
    }
};

struct hru_output_info {
    caf::actor hru_actor;
    int index_hru;
    int index_gru;
    bool ready_to_write;
};


struct output_partition {
    int start_gru;
    int num_gru;            
    int num_timesteps;     
    int simulation_timesteps_remaining;
    int grus_ready_to_write;
    // 2D matrix of output handles
    std::vector<std::shared_ptr<hru_output_info>> hru_info_and_data;
};



// Take an unintialized vector of output partitions and initialize it
void initArrayOfOuputPartitions(std::vector<std::shared_ptr<output_partition>>& output_partitions, 
    int num_partitions, int num_gru, int num_timesteps,  int simulation_timesteps_remaining);

// Add a HRU that is ready to write to the output structure
std::optional<int> addReadyToWriteHRU(std::vector<std::shared_ptr<output_partition>>& output_partitions, 
    caf::actor hru_actor, int gru_index, int hru_index);


// find which partition the HRU belongs to
int findPatritionIndex(int grus_per_partition, int gru_index, int num_partitions);

// After writing to a file, update the number of timesteps remaining in the simulation
void updateSimulationTimestepsRemaining(std::shared_ptr<output_partition>& output_partition);

// After writing to a file, check if we need to send the hru a modified timestep value b/c we have less simulation timesteps remaining than 
// the number of timesteps in the output file
void updateNumTimeForPartition(std::shared_ptr<output_partition> &output_partition);

void resetReadyToWrite(std::shared_ptr<output_partition> &output_partition);










// This class holds the output for the HRUs as a buffer so 
// we can write more data at once
class Output_Container {
    private:
        // Matrix charactieristics
        int max_steps; // maximum number of steps we can hold for an HRU before writing
        int max_hrus; // maximum number of hrus we can hold for the structure

        std::vector<std::vector<hru_output_handles>> hru_output_handles_vector; // Pointers to HRU output data

    public:
        Output_Container(int max_hrus, int max_steps);
        ~Output_Container();

        // insertes output from an HRU into hru_output_handles
        void insertOutput(int hru_index, hru_output_handles hru_output);

        bool isFull(int hru_index);

        // returns the matrix of hru_outputs for writing
        std::vector<std::vector<hru_output_handles>> getAllHRUOutput();

        void clearAll();


};
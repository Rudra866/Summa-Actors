#pragma once
#include "caf/all.hpp"
#include <string>
#include <iostream>

extern "C" {

  void f_readDimension(int& start_gru, int& num_gru, int& file_gru, 
                       int& file_hru, int& err, void* message);
  void f_setHruCount(int& iGRU, int& sGRU);
  void f_setIndexMap();
  void f_getNumHru(int& num_hru);

  void f_readIcondNlayers(int& num_gru, int& err, void* message);

  void f_getNumHruPerGru(int& arr_size, int& num_hru_per_gru_array);

  void f_deallocateGruStruc();
}

/** Determine the state of the GRU */
enum class gru_state { running, failed, succeeded };

/** Gru Information (meant to mimic gru_struc)*/
class GRU {
  private:
    int index_netcdf_;       // The index of the GRU in the netcdf file
    int index_job_;          // The index of the GRU within this job
    caf::actor actor_ref_;   // The actor for the GRU

    int num_hrus_;           // The number of HRUs in the GRU

    // Modifyable Parameters
    int dt_init_factor_;     // The initial dt for the GRU
    double rel_tol_;         // The relative tolerance for the GRU
    double abs_tol_;         // The absolute tolerance for the GRU
    double rel_tol_temp_cas_; // The relative tolerance for the temperature of the cas
    double rel_tol_temp_veg_; // The relative tolerance for the temperature of the veg
    double rel_tol_wat_veg_;  // The relative tolerance for the water content of the veg
    double rel_tol_temp_soil_snow_; // The relative tolerance for the temperature of the soil snow
    double rel_tol_wat_snow_; // The relative tolerance for the water content of the snow
    double rel_tol_matric_;   // The relative tolerance for the matric potential
    double rel_tol_aquifr_;   // The relative tolerance for the aquifer
    double abs_tol_temp_cas_; // The absolute tolerance for the temperature of the cas
    double abs_tol_temp_veg_; // The absolute tolerance for the temperature of the veg
    double abs_tol_wat_veg_;  // The absolute tolerance for the water content of the veg
    double abs_tol_temp_soil_snow_; // The absolute tolerance for the temperature of the soil snow
    double abs_tol_wat_snow_; // The absolute tolerance for the water content of the snow
    double abs_tol_matric_;   // The absolute tolerance for the matric potential
    double abs_tol_aquifr_;   // The absolute tolerance for the aquifer

    // Status Information
    int attempts_left_;      // The number of attempts left for the GRU to succeed
    gru_state state_;        // The state of the GRU

    // Timing Information
    double run_time_ = 0.0;  // The total time to run the GRU

    
  public:
    // Constructor
    GRU(int index_netcdf, int index_job, caf::actor actor_ref, 
        int dt_init_factor, double rel_tol, double abs_tol, double rel_tol_temp_cas,
        double rel_tol_temp_veg, double rel_tol_temp_soil_snow, double rel_tol_wat_veg,
        double rel_tol_wat_snow, double rel_tol_matric, double rel_tol_aquifr,int max_attempts) 
        : index_netcdf_(index_netcdf), index_job_(index_job), 
          actor_ref_(actor_ref), dt_init_factor_(dt_init_factor),
          rel_tol_(rel_tol), abs_tol_(abs_tol), rel_tol_temp_cas_(rel_tol_temp_cas),
          rel_tol_temp_veg_(rel_tol_temp_veg), rel_tol_temp_soil_snow_(rel_tol_soil_snow),
          rel_tol_wat_veg_(rel_tol_wat_veg), rel_tol_wat_snow_(rel_tol_wat_snow),
          rel_tol_matric_(rel_tol_matric), rel_tol_aquifr_(rel_tol_aquifr),attempts_left_(max_attempts),
          state_(gru_state::running) {};

    // Deconstructor
    ~GRU() {};

    // Getters
    inline int getIndexNetcdf() const { return index_netcdf_; }
    inline int getIndexJob() const { return index_job_; }
    inline caf::actor getActorRef() const { return actor_ref_; }
    inline double getRunTime() const { return run_time_; }
    inline double getRelTol() const { return rel_tol_; }
    inline double getAbsTol() const { return abs_tol_; }
    inline double getRelTolTempCas() const { return rel_tol_temp_cas; }
    inline double getRelTolTempVeg() const { return rel_tol_temp_veg;}
    inline double getRelTolWatVeg() const { return rel_tol_wat_veg;}
    inline double getRelTolTempSoilSnow() const { return rel_tol_temp_soil_snow;}
    inline double getRelTolWatSnow() const { return rel_tol_wat_snow;}
    inline double getRelTolMatric() const { return rel_tol_matric;}
    inline double getRelTolAquifr() const { return rel_tol_aquifr;}
    inline double getAbsTolTempCas() const { return abs_tol_temp_cas;}
    inline double getAbsTolTempVeg() const { return abs_tol_temp_veg;}
    inline double getAbsTolWatVeg() const { return abs_tol_wat_veg;}
    inline double getAbsTolTempSoilSnow() const { return abs_tol_temp_soil_snow;}
    inline double getAbsTolWatSnow() const { return abs_tol_wat_snow;}
    inline double getAbsTolMatric() const { return abs_tol_matric;}
    inline double getAbsTolAquifr() const { return abs_tol_aquifr;}
    inline int getAttemptsLeft() const { return attempts_left_; }
    inline gru_state getStatus() const { return state_; }

    // Setters
    inline void setRunTime(double run_time) { run_time_ = run_time; }
    //Setting rel_tol will set all rel_tol_* to the same value
    inline void setRelTol(double rel_tol){
      rel_tol_ = rel_tol;
      rel_tol_temp_cas = rel_tol;
      rel_tol_temp_veg = rel_tol;
      rel_tol_temp_soil_snow = rel_tol;
      rel_tol_wat_veg = rel_tol;
      rel_tol_wat_snow = rel_tol;
      rel_tol_matric = rel_tol;
      rel_tol_aquifr = rel_tol;
       }
    inline void setAbsTol(double abs_tol) { 
      abs_tol_ = abs_tol;
      abs_tol_temp_cas = abs_tol;
      abs_tol_temp_veg = abs_tol;
      abs_tol_temp_soil_snow = abs_tol;
      abs_tol_wat_veg = abs_tol;
      abs_tol_wat_snow = abs_tol;
      abs_tol_matric = abs_tol;
      abs_tol_aquifr = abs_tol;
        }
    inline void setSuccess() { state_ = gru_state::succeeded; }
    inline void setFailed() { state_ = gru_state::failed; }
    inline void setRunning() { state_ = gru_state::running; }

    // Methods
    inline bool isFailed() const { return state_ == gru_state::failed; }
    inline void decrementAttemptsLeft() { attempts_left_--; }
    inline void setActorRef(caf::actor gru_actor) { actor_ref_ = gru_actor; }
};


// Class to track Gru Information For Nodes in data assimilation
struct NodeGruInfo {
  int node_start_gru_;  // Start Gru for the node
  int start_gru_sim_;   // Start Gru provided by the user (global)
  int node_num_gru_;    // Number of GRUs for the node
  int num_gru_sim_;     // Number of GRUs provided by the user (global)
  int file_gru_;

  NodeGruInfo(int node_start_gru = -1, int start_gru_sim = -1, 
              int node_num_gru = -1, int num_gru_sim = -1, int file_gru = -1) 
              : node_start_gru_(node_start_gru), start_gru_sim_(start_gru_sim),
                node_num_gru_(node_num_gru), num_gru_sim_(num_gru_sim), 
                file_gru_(file_gru) {};
};
template <class Inspector>
bool inspect(Inspector& f, NodeGruInfo& x) {
  return f.object(x).fields(f.field("node_start_gru", x.node_start_gru_),
                            f.field("start_gru_sim", x.start_gru_sim_),
                            f.field("node_num_gru", x.node_num_gru_),
                            f.field("num_gru_sim", x.num_gru_sim_),
                            f.field("file_gru", x.file_gru_));
}

class GruStruc {
  private:
    // Inital Information about the GRUs
    int start_gru_;
    int num_gru_;
    int num_hru_;
    int file_gru_;
    int file_hru_;
    
    // GRU specific Information
    std::vector<std::unique_ptr<GRU>> gru_info_;
    std::vector<NodeGruInfo> node_gru_info_;

  
    // Runtime status of the GRUs
    int num_gru_done_ = 0;
    int num_gru_failed_ = 0;
    int num_retry_attempts_left_ = 0;
    int attempt_ = 1;

    // todo: check if this is necessary
    std::vector<int> num_hru_per_gru_;
  public:
    GruStruc(int start_gru, int num_gru, int num_retry_attempts);
    ~GruStruc(){f_deallocateGruStruc();};
    int readDimension();
    int readIcondNlayers();

    // Set the gru information for each node participating in data assimilation
    int setNodeGruInfo(int num_nodes);
    std::string getNodeGruInfoString();
    inline NodeGruInfo getNodeGruInfo(int index) {
      return node_gru_info_[index];
    }

    inline std::vector<std::unique_ptr<GRU>>& getGruInfo() { return gru_info_; }
    inline int getStartGru() const { return start_gru_; }
    inline int getNumGru() const { return num_gru_; }
    inline int getFileGru() const { return file_gru_; }
    inline int getNumHru() const { return num_hru_; }
    inline int getGruInfoSize() const { return gru_info_.size(); }
    inline int getNumGruDone() const { return num_gru_done_; }
    inline int getNumGruFailed() const { return num_gru_failed_; }

    inline void addGRU(std::unique_ptr<GRU> gru) {
      gru_info_.push_back(std::move(gru));
    }

    inline void incrementNumGruDone() { num_gru_done_++; }
    inline void incrementNumGruFailed() { num_gru_failed_++; num_gru_done_++;}
    inline void decrementRetryAttempts() { num_retry_attempts_left_--; }
    inline void decrementNumGruFailed() { num_gru_failed_--; num_gru_done_--;}
    inline GRU* getGRU(int index) { return gru_info_[index-1].get(); }

    inline bool isDone() { return num_gru_done_ >= num_gru_; }
    inline bool hasFailures() { return num_gru_failed_ > 0; }
    inline bool shouldRetry() { return num_retry_attempts_left_ > 0; }

    int getFailedIndex(); 
    void getNumHrusPerGru();

    // todo: check if this is necessary
    inline int getNumHruPerGru(int index) { return num_hru_per_gru_[index]; }
};
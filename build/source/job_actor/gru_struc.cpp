#include "gru_struc.hpp"
#include "message_atoms.hpp"
#include <iostream>
#include <memory>
#include <execution>
#include <algorithm>
using chrono_time = std::chrono::time_point<std::chrono::high_resolution_clock>;

GruStruc::GruStruc(int start_gru, int num_gru, int num_retry_attempts) {
  start_gru_ = start_gru;
  num_gru_ = num_gru;
  num_retry_attempts_left_ = num_retry_attempts;
}

int GruStruc::readDimension() {
  chrono_time start = std::chrono::high_resolution_clock::now();
  // gru_struc is set up in fortran here
  int err = 0; int file_gru, file_hru;
  std::unique_ptr<char[]> err_msg(new char[256]);
  f_readDimension(start_gru_, num_gru_, file_gru, file_hru, err, 
                  &err_msg);
  if (err != 0) { 
    std::cout << "ERROR: GruStruc - ReadDimension()\n";
    std::cout << err_msg.get() << "\n";
  }
  file_gru_ = file_gru;
  file_hru_ = file_hru;

  std::vector<int> indicies(num_gru_);
  std::iota(indicies.begin(), indicies.end(), start_gru_);
  std::for_each(std::execution::par, indicies.begin(), indicies.end(), 
    [=](int i) { f_setHruCount(i, start_gru_); 
  });
  f_setIndexMap();
  f_getNumHru(num_hru_);


  chrono_time end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "Time taken for ReadDimension: " << elapsed_seconds.count() 
            << "s\n";
  return err;
}

int GruStruc::readIcondNlayers() {
  chrono_time start = std::chrono::high_resolution_clock::now();

  int err = 0;
  std::unique_ptr<char[]> err_msg(new char[256]);
  read_icond_nlayers_fortran(num_gru_, err, &err_msg);
  if (err != 0) { 
    std::cout << "ERROR: GruStruc - ReadIcondNlayers\n";
  }

  chrono_time end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "Time taken for ReadIcondNlayers: " << elapsed_seconds.count() 
            << "s\n";
  return 0;
}

int GruStruc::getFailedIndex() {
  for (int i = 0; i < gru_info_.size(); i++) {
    if (gru_info_[i]->getStatus() == gru_state::failed) {
      return gru_info_[i]->getIndexJob();
    }
  }
  return -1;
}

void GruStruc::getNumHrusPerGru() {
  num_hru_per_gru_.resize(num_gru_, 0);
  get_num_hru_per_gru_fortran(num_gru_, num_hru_per_gru_[0]);
}

int GruStruc::setNodeGruInfo(int num_nodes) {
  int gru_per_node = (num_gru_ + num_nodes - 1) / num_nodes;
  int remaining = num_gru_;
  for (int i = 0; i < num_nodes; i++) {
    int node_start_gru = i * gru_per_node + start_gru_;
    int node_num_gru = gru_per_node;
    if (i == num_nodes - 1) {
      node_num_gru = remaining;
    }
    remaining -= node_num_gru;
    node_gru_info_.push_back(NodeGruInfo(
        node_start_gru, start_gru_, node_num_gru, num_gru_, file_gru_));
  }

  return 0;
}

std::string GruStruc::getNodeGruInfoString() {
  std::string str = "Gru Per Node Information\n";
  for (int i = 0; i < node_gru_info_.size(); i++) {
    str += "------------------------------------------\n";
    str += "Node " + std::to_string(i) + "\n";
    str += "Start_Gru = " + std::to_string(node_gru_info_[i].node_start_gru_);
    str += " : Num_Gru = " + std::to_string(node_gru_info_[i].node_num_gru_);
    str += "\n";
  }
  return str;
}


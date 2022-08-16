#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include <string>
#include "batch_manager.hpp"
#include "summa_server.hpp"
#include "message_atoms.hpp"
#include "global.hpp"
#include <optional>


namespace caf {

behavior summa_server(stateful_actor<summa_server_state>* self, std::string config_path) {
    aout(self) << "Summa Server has Started \n";
    self->state.config_path = config_path;
    // std::string returnType;
    // getSettingsTest(std::vector<std::string> {"test", "test2"} ,returnType);

    // --------------------------Initalize Settings --------------------------
    self->state.total_hru_count = getSettings(self->state.config_path, "SimulationSettings", "total_hru_count", 
		self->state.total_hru_count).value_or(-1);
    if (self->state.total_hru_count == -1) {
        aout(self) << "ERROR: With total_hru_count - CHECK Summa_Actors_Settings.json\n";
    }
    self->state.num_hru_per_batch = getSettings(self->state.config_path, "SimulationSettings", "num_hru_per_batch", 
		self->state.num_hru_per_batch).value_or(-1);
    if (self->state.num_hru_per_batch == -1) {
        aout(self) << "ERROR: With num_hru_per_batch - CHECK Summa_Actors_Settings.json\n";
    }
    // --------------------------Initalize Settings --------------------------

    // -------------------------- Initalize CSV ------------------------------
    std::ofstream csv_output;
    self->state.csv_output_name = "Batch_Results.csv";
    csv_output.open(self->state.csv_output_name, std::ios_base::out);
    csv_output << 
        "Batch_ID,"  <<
        "Start_HRU," <<
        "Num_HRU,"   << 
        "Hostname,"  <<
        "Run_Time,"  <<
        "Read_Time," <<
        "Write_Time,"<<
        "Status\n";
    csv_output.close();
    // -------------------------- Initalize CSV ------------------------------

    // -------------------------- Assemble Batches ---------------------------
    aout(self) << "Assembling HRUs into Batches\n";
    if (assembleBatches(self) == -1) {
        aout(self) << "ERROR: assembleBatches\n";
    } else {
        aout(self) << "HRU Batches Assembled, Ready For Clients to Connect \n";

        for (std::vector<int>::size_type i = 0; i < self->state.batch_list.size(); i++) {
            self->state.batch_list[i].printBatchInfo();
        }
    }
    // -------------------------- Assemble Batches ---------------------------

    return {
        [=](connect_to_server, actor client, std::string hostname) {
            // Client is connecting - Add it to our client list and assign it a batch
            aout(self) << "Actor trying to connect with hostname " << hostname << "\n";
            int client_id = self->state.client_list.size(); // So we can lookup the client in O(1) time 
            self->state.client_list.push_back(Client(client_id, client, hostname));

            std::optional<int> batch_id = getUnsolvedBatchID(self);
            if (batch_id.has_value()) {
                // update the batch in the batch list with the host and actor_ref
                self->state.batch_list[batch_id.value()].assignedBatch(self->state.client_list[client_id].getHostname(), client);
                
                int start_hru = self->state.batch_list[batch_id.value()].getStartHRU();
                int num_hru = self->state.batch_list[batch_id.value()].getNumHRU();

                self->send(client, 
                    batch_v, 
                    client_id, 
                    batch_id.value(), 
                    start_hru, 
                    num_hru, 
                    self->state.config_path);

            } else {

                aout(self) << "We Are Done - Telling Clients to exit \n";
                for (std::vector<int>::size_type i = 0; i < self->state.client_list.size(); i++) {
                    self->send(self->state.client_list[i].getActor(), time_to_exit_v);
                }
                self->quit();
                return;
            }
        },

        [=](done_batch, actor client, int client_id, int batch_id, double total_duration, 
            double total_read_duration, double total_write_duration) {
            
            self->state.batch_list[batch_id].solvedBatch(total_duration, total_read_duration, total_write_duration);
            self->state.batch_list[batch_id].writeBatchToFile(self->state.csv_output_name);
            self->state.batches_solved++;
            self->state.batches_remaining = self->state.batch_list.size() - self->state.batches_solved;

            aout(self) << "\n****************************************\n";
            aout(self) << "Client finished batch: " << batch_id << "\n";
            aout(self) << "Client hostname = " << self->state.client_list[client_id].getHostname() << "\n";
            aout(self) << "Total Batch Duration = " << total_duration << "\n";
            aout(self) << "Total Batch Read Duration = " << total_read_duration << "\n";
            aout(self) << "Total Batch Write Duration = " << total_write_duration << "\n";
            aout(self) << "Batches Solved = " << self->state.batches_solved << "\n";
            aout(self) << "Batches Remaining = " << self->state.batches_remaining << "\n";
            aout(self) << "****************************************\n";

            // Find a new batch
            std::optional<int> new_batch_id = getUnsolvedBatchID(self);
             if (new_batch_id.has_value()) {
                // update the batch in the batch list with the host and actor_ref
                self->state.batch_list[new_batch_id.value()].assignedBatch(self->state.client_list[client_id].getHostname(), client);
            
                int start_hru = self->state.batch_list[new_batch_id.value()].getStartHRU();
                int num_hru = self->state.batch_list[new_batch_id.value()].getNumHRU();

                self->send(client, 
                    batch_v, 
                    client_id, 
                    new_batch_id.value(), 
                    start_hru, 
                    num_hru, 
                    self->state.config_path);

            } else {
                // We found no batch this means all batches are assigned
                if (self->state.batches_remaining == 0) {
                    aout(self) << "All Batches Solved -- Telling Clients To Exit\n";
                    for (std::vector<int>::size_type i = 0; i < self->state.client_list.size(); i++) {
                        self->send(self->state.client_list[i].getActor(), time_to_exit_v);
                    }
                    aout(self) << "\nSUMMA_SERVER -- EXITING\n";
                    self->quit();
                } else {
                    aout(self) << "No Batches left to compute -- letting client stay connected in case batch fails\n";
                }
            }
        }
    };
}


int assembleBatches(stateful_actor<summa_server_state>* self) {
    int remaining_hru_to_batch = self->state.total_hru_count;
    int count_index = 0; // this is like the offset for slurm bash scripts
    int start_hru = 1;

    while(remaining_hru_to_batch > 0) {
        if (self->state.num_hru_per_batch > remaining_hru_to_batch) {
            self->state.batch_list.push_back(Batch(count_index, start_hru, 
                remaining_hru_to_batch));
            remaining_hru_to_batch = 0;
        } else {
            self->state.batch_list.push_back(Batch(count_index, start_hru, 
                self->state.num_hru_per_batch));
            
            remaining_hru_to_batch -= self->state.num_hru_per_batch;
            start_hru += self->state.num_hru_per_batch;
            count_index += 1;
        }
    }
    return 0;
}

std::optional<int> getUnsolvedBatchID(stateful_actor<summa_server_state>* self) {
    // Find the first unassigned batch
    for (std::vector<int>::size_type i = 0; i < self->state.batch_list.size(); i++) {
        if (self->state.batch_list[i].getBatchStatus() == unassigned) {
            return i;
        }
    }
    return {};
}

} // end namespace

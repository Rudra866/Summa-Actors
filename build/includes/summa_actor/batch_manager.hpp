#pragma once
#include "caf/all.hpp"
#include <vector>
#include <string>

class Batch;

enum batch_status {
    unassigned,
    assigned,
    solved,
    failed
};

class Batch{
    private:
        int batch_id;
        int start_hru;
        int num_hru;

        bool assigned_to_actor;

     public:
        Batch(int batch_id = -1, int start_hru = -1, int num_hru = -1);

        int getBatchID();

        bool getBatchStatus();


        template <class Inspector>
        friend bool inspect(Inspector& inspector, Batch& batch) {
            return inspector.object(batch).fields(
                        inspector.field("batch_id", batch.batch_id), 
                        inspector.field("start_hru", batch.start_hru),
                        inspector.field("num_hru", batch.num_hru),
                        inspector.field("status", batch.assigned_to_actor));
        }
};




// class Batch {
//     private:
//         int batch_id;
//         int start_hru;
//         int num_hru;

        // double run_time = -1;
        // double read_time = -1;
        // double write_time = -1;
        
//         std::string assigned_host;
//         caf::actor assigned_actor;
//         // batch_status status;


//     public:
//         Batch(int batch_id = -1, int start_hru = -1, int num_hru = -1);

//         // void printBatchInfo();

//         // batch_status getBatchStatus();

//         // int getBatchID();

//         // int getStartHRU();

//         // int getNumHRU();

//         // void solvedBatch(double run_time, double read_time, double write_time);

//         // void assignedBatch(std::string hostname, caf::actor actor_ref);

//         // void updateRunTime(double run_time);

//         // void writeBatchToFile(std::string file_name);

//         template <class Inspector>
//         friend bool inspect(Inspector& inspector, Batch& batch) {
//             return inspector.object(batch).fields(
//                         inspector.field("batch_id", batch.batch_id), 
//                         inspector.field("start_hru", batch.start_hru),
//                         inspector.field("num_hru", batch.num_hru),
//                         inspector.field("run_time", batch.run_time),
//                         inspector.field("read_time", batch.read_time),
//                         inspector.field("write_time", batch.write_time),
//                         inspector.field("assigned_host", batch.assigned_host),
//                         inspector.field("assigned_actor", batch.assigned_actor),
//                         inspector.field("status", batch.status));
//         }
// };


class Batch_Container {
    private:
        int total_hru_count;
        int num_hru_per_batch;


        std::vector<Batch> batch_list;
        std::vector<Batch> solved_batches;
        std::vector<Batch> failed_batches;        
    
    
    public:
        /**
         * @brief Construct a new Batch_Container object
         * Creating the batch_manager will also create the batches
         * with the two parameters that are passed in. 
         */
        Batch_Container(int total_hru_count, int num_hru_per_batch);
        
        /**
         * Assign a batch to be solved by a client.
         * The hostname and the actor_ref of the client solving this batch
         * are added to the client for the servers awareness
         * The batch is then returned by this method and sent to the respective client
         */
        Batch assignBatch(std::string hostname, caf::actor actor_ref);

        /**
         * On a successful batch we take the batch given to us by the client 
         * and add it to our solved_batches list.
         *  
         * We can then remove the batch from the global batch list.
         */
        void updateBatch_success(Batch successful_batch, std::string output_csv);

        /**
         * A batch failure is returned to us by the client
         * This is for when a client failed to solve the batch.
         */
        void updateBatch_failure(Batch failed_batch);

        /**
         * A client has found to be disconnected. Unassign all batches
         * that were assigned to the disconnected client. The client id 
         * is passed in as a parameter
         */
        void updatedBatch_disconnectedClient(int client_id);

        /**
         * Create the csv file for the completed batches.
         */
        void inititalizeCSVOutput(std::string csv_output_name);
    
    private:
        /**
         * Assemble the total number of HRUs given by the user into batches.
         * The total number of hrus and the sizes of the batches are parameters
         * provided by the user.
         */
        void assembleBatches(int total_hru_count, int num_hru_per_batch);



};
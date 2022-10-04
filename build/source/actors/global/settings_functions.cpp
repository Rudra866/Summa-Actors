#include "settings_functions.hpp"
#include "global.hpp"

int read_settings_from_json(std::string json_settings_file,
        Distributed_Settings &distributed_settings, 
        Summa_Actor_Settings &summa_actor_settings,
        File_Access_Actor_Settings &file_access_actor_settings, 
        Job_Actor_Settings &job_actor_settings, 
        HRU_Actor_Settings &hru_actor_settings) {
    
    
    // read distributed settings
    std::string parent_key = "Distributed_Settings";
    distributed_settings.distributed_mode = getSettings(json_settings_file, parent_key, 
        "distributed_mode", distributed_settings.distributed_mode).value_or(false);

    distributed_settings.hostname = getSettings(json_settings_file, parent_key, 
        "hostname", distributed_settings.hostname).value_or("");

    distributed_settings.port = getSettings(json_settings_file, parent_key,
        "port", distributed_settings.port).value_or(-1);

    distributed_settings.total_hru_count = getSettings(json_settings_file, parent_key,
        "total_hru_count", distributed_settings.total_hru_count).value_or(-1);

    distributed_settings.num_hru_per_batch = getSettings(json_settings_file, parent_key,
        "num_hru_per_batch", distributed_settings.num_hru_per_batch).value_or(-1);

    
    // read settings for summa actor
    parent_key = "Summa_Actor";
    summa_actor_settings.output_structure_size = getSettings(json_settings_file, parent_key,
        "output_structure_size", summa_actor_settings.output_structure_size).value_or(250);
    
    summa_actor_settings.max_gru_per_job = getSettings(json_settings_file, parent_key,
        "max_gru_per_job", summa_actor_settings.max_gru_per_job).value_or(250);


    // read file access actor settings
    parent_key = "File_Access_Actor";
    file_access_actor_settings.num_vectors_in_output_manager = getSettings(json_settings_file, parent_key,
        "num_vectors_in_output_manager", file_access_actor_settings.num_vectors_in_output_manager).value_or(1);


    // read settings for job actor
    parent_key = "Job_Actor";
    job_actor_settings.file_manager_path = getSettings(json_settings_file, parent_key,
        "file_manager_path", job_actor_settings.file_manager_path).value_or("");

    job_actor_settings.output_csv = getSettings(json_settings_file, parent_key,
        "output_csv", job_actor_settings.output_csv).value_or(false);

    job_actor_settings.csv_path = getSettings(json_settings_file, parent_key, 
        "csv_path", job_actor_settings.csv_path).value_or("");


    // read settings for hru_actor
    parent_key = "HRU_Actor";
    hru_actor_settings.print_output = getSettings(json_settings_file, parent_key, 
        "print_output", hru_actor_settings.print_output).value_or(true);

    hru_actor_settings.output_frequency = getSettings(json_settings_file, parent_key, 
        "output_frequency", hru_actor_settings.output_frequency).value_or(250);

    return 0;
}


void check_settings_from_json(Distributed_Settings &distributed_settings, 
    Summa_Actor_Settings &summa_actor_settings, File_Access_Actor_Settings &file_access_actor_settings, 
    Job_Actor_Settings &job_actor_settings, HRU_Actor_Settings &hru_actor_settings) {

    std::cout << "************ DISTRIBUTED_SETTINGS ************\n";
    std::cout << distributed_settings.distributed_mode << "\n";
    std::cout << distributed_settings.hostname << "\n";
    std::cout << distributed_settings.port << "\n";
    std::cout << distributed_settings.total_hru_count << "\n";
    std::cout << distributed_settings.num_hru_per_batch << "\n\n\n";

    std::cout << "************ SUMMA_ACTOR_SETTINGS ************\n";
    std::cout << summa_actor_settings.output_structure_size << "\n";
    std::cout << summa_actor_settings.max_gru_per_job << "\n\n\n";

    std::cout << "************ FILE_ACCESS_ACTOR_SETTINGS ************\n";
    std::cout << file_access_actor_settings.num_vectors_in_output_manager << "\n\n\n";

    std::cout << "************ JOB_ACTOR_SETTINGS ************\n";
    std::cout << job_actor_settings.file_manager_path << "\n";
    std::cout << job_actor_settings.output_csv << "\n";
    std::cout << job_actor_settings.csv_path << "\n\n\n";

    std::cout << "************ HRU_ACTOR_SETTINGS ************\n";
    std::cout << hru_actor_settings.print_output << "\n";
    std::cout << hru_actor_settings.output_frequency << "\n\n\n"; 

}
#pragma once
#include <string>
#include <fstream>

class Logger {
  private:
    std::string log_file_;
    bool enable_logging_;
  public:
    Logger(const std::string log_file_name = "");
    ~Logger();
    void log(const std::string &message);

    template <class Inspector>
    friend bool inspect(Inspector& inspector, Logger& logger) {
        return inspector.object(logger).fields(
              inspector.field("log_file", logger.log_file_));
    }

};

class ErrorLogger {
  private:
    std::string log_file_;
    std::string log_dir_;
    int attempt_ = 1;
    bool enable_logging_;         
  public:
    ErrorLogger(const std::string error_log_file_name = "");
    ~ErrorLogger() {};
    void logError(int ref_gru, int indx_gru, int timestep, int be_steps, double rel_tol, 
                  double abs_tolWat, double abs_tolNrg, int err_code, const std::string &message);
    void nextAttempt();
};


class SuccessLogger {
  private:
    std::string log_file_;
    std::string log_dir_;
    int attempt_ = 1;
    bool enable_logging_;
  public:
    SuccessLogger(const std::string success_log_file_name = "");
    ~SuccessLogger() {};
    void logSuccess(int ref_gru, int indx_gru, int be_steps, double rel_tol, double abs_tolWat, double abs_tolNrg);
    void nextAttempt();
};
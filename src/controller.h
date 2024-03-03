#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <filesystem>

#include "config.h"

class controller {

public:
    controller();

    bool run(int argc, char *argv[]);
private:
    std::string _contract_dir;
    std::string _mir_file;
    std::string _hir_file;
    std::string _esbmc_path;
    config _globals{};

    static bool is_mir(const std::filesystem::path &path);
    static bool is_hir(const std::filesystem::path &path);
    static bool is_directory(const std::filesystem::path &path);

    static std::string get_datetime();

    static std::filesystem::path get_temp_dir();

    static std::string get_millis(std::chrono::time_point<std::chrono::high_resolution_clock> start, std::chrono::time_point<std::chrono::high_resolution_clock> end);
};



#endif //CONTROLLER_H

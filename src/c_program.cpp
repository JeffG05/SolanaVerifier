#include <sstream>
#include "c_program.h"

c_program::c_program(const std::filesystem::path &path) {
    _path = path;
}

c_program::c_program(const std::string &path) : c_program(std::filesystem::path(path)) {}

std::string c_program::get_path() const {
    return _path.string();
}

void c_program::verify() const {
    std::string esbmc_path = "esbmc";
    if (const char* ESBMC_PATH = std::getenv("SV_ESBMC_PATH")) {
        esbmc_path = ESBMC_PATH;
    }

    std::stringstream cmd;
    cmd << esbmc_path << " " << get_path();
    system(cmd.str().data());
}


#include "c_program.h"

c_program::c_program(const std::filesystem::path &path) {
    _path = path;
}

c_program::c_program(const std::string &path) : c_program(std::filesystem::path(path)) {}

std::string c_program::get_path() const {
    return _path.string();
}

#include "smt_formula.h"

smt_formula::smt_formula(const std::filesystem::path &path) {
    _path = path;
}

smt_formula::smt_formula(const std::string &path) : smt_formula(std::filesystem::path(path)) {}


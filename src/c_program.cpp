#include <sstream>
#include <fstream>
#include "c_program.h"

#include "utils.h"

c_program::c_program(const std::string &contract_name, const std::filesystem::path &path) {
    _contract_name = contract_name;
    _path = path;
}

c_program::c_program(const std::string &contract_name, const std::string &path) : c_program(contract_name, std::filesystem::path(path)) {}

std::string c_program::get_path() const {
    return _path.string();
}

smt_formula c_program::build_smt(const std::filesystem::path& target) const {
    const std::filesystem::path out = target / (_contract_name + ".smt2");

    std::stringstream cmd;
    cmd << "esbmc " << get_path() << "--smtlib --smt-formula-only --output " << out;
    system(cmd.str().data());

    return smt_formula(out);
}


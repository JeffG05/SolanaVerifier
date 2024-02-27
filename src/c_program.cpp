#include <sstream>
#include <fstream>
#include "c_program.h"

c_program::c_program(const std::string &contract_name, const std::filesystem::path &path) {
    _contract_name = contract_name;
    _path = path;
}

c_program::c_program(const std::string &contract_name, const std::string &path) : c_program(contract_name, std::filesystem::path(path)) {}

std::string c_program::get_path() const {
    return _path.string();
}

verification_result c_program::verify_boolector(const std::filesystem::path& target, const std::filesystem::path& esbmc_path) const {
    return verify(target, esbmc_path, "boolector");
}

verification_result c_program::verify_z3(const std::filesystem::path& target, const std::filesystem::path& esbmc_path) const {
    return verify(target, esbmc_path, "z3");
}

verification_result c_program::verify(const std::filesystem::path& target, const std::filesystem::path& esbmc_path, const std::string& smt_solver) const {
    const std::filesystem::path log_out = target / (_contract_name + "_" + smt_solver + "_log.txt");

    std::stringstream cmd;
    cmd << esbmc_path << " " << get_path() << " --" << smt_solver << " --file-output " << log_out << " > /dev/null 2>&1";
    system(cmd.str().data());

    return verification_result(log_out);
}



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

smt_formula c_program::build_smt(const std::filesystem::path& target, const std::filesystem::path& esbmc_path) const {
    const std::filesystem::path smt_out = target / (_contract_name + ".smt2");
    const std::filesystem::path log_out = target / (_contract_name + "_smt_log.txt");

    std::stringstream cmd;
    cmd << esbmc_path << " " << get_path() << " --smtlib --smt-formula-only --output " << smt_out << " --file-output " << log_out << " > /dev/null 2>&1";
    system(cmd.str().data());

    return smt_formula(smt_out);
}

verification_result c_program::verify_z3(const std::filesystem::path& target, const std::filesystem::path& esbmc_path) const {
    const std::filesystem::path log_out = target / (_contract_name + "_z3_log.txt");

    std::stringstream cmd;
    cmd << esbmc_path << " " << get_path() << " --z3 --file-output " << log_out << " > /dev/null 2>&1";
    system(cmd.str().data());

    return verification_result(verifier::z3, log_out);
}



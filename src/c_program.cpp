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

std::string c_program::get_dir() const {
    return _path.parent_path().string();
}

verification_result c_program::verify(const std::filesystem::path& target, const std::filesystem::path& esbmc_path, const std::string& smt_solver) const {
    const std::filesystem::path log_out = target / (_contract_name + "_" + smt_solver + "_log.txt");

    std::stringstream cmd;
    std::list<std::string> options = {
            "--" + smt_solver,
            "--file-output " + log_out.string(),
            "--incremental-bmc",
            "--multi-property",
            "--no-bounds-check",
            "--no-div-by-zero-check",
            "--no-pointer-check",
            "--no-align-check",
            "--no-unlimited-scanf-check",
            "--no-vla-size-check",
            "--force-malloc-success",
            "--64"
    };
    cmd << esbmc_path << " " << get_path() << " " << utils::join(options, " ") << " > /dev/null 2>&1";
    system(cmd.str().data());

    return verification_result(log_out);
}



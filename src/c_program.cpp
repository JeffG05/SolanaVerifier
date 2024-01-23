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
    const std::filesystem::path temp_out = target / (_contract_name + "_output.txt");
    const std::filesystem::path out_path = target / (_contract_name + ".smt2");

    std::stringstream cmd;
    cmd << "esbmc " << get_path() << " --smt-formula-only --file-output " << temp_out;
    system(cmd.str().data());

    std::fstream in;
    in.open(temp_out, std::ios::in);
    if (!in.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read SMT converter output"));
    }

    std::fstream out;
    out.open(out_path, std::ios::in);
    if (!out.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to write SMT output"));
    }

    std::string line;
    bool smt_started = false;
    while (getline(in, line)) {
        if (!smt_started && utils::trim(line).starts_with("(")) {
            smt_started = true;
        }

        if (smt_started) {
            out << line << std::endl;
        }
    }

    in.close();
    out.close();

    return smt_formula(out_path);
}


#include <fstream>
#include "verification_result.h"
#include "utils.h"

verification_result::verification_result(const verifier v, const std::filesystem::path &path) {
    _verifier = v;
    _log_path = path;

    auto [is_sat, vulnerability] = parse_log();
    _is_sat = is_sat;
    _vulnerability = vulnerability;
}

verification_result::verification_result(const verifier v, const std::string &path) : verification_result(v, std::filesystem::path(path)) {}

bool verification_result::get_is_sat() const {
    return _is_sat;
}

std::string verification_result::get_vulnerability() const {
    return _vulnerability;
}

result verification_result::parse_log() const {
    switch (_verifier) {
        case verifier::z3:
            return parse_z3();
    }

    std::throw_with_nested(std::runtime_error("Unknown verifier used"));
}

result verification_result::parse_z3() const {
    bool is_sat;
    std::string vulnerability;

    std::fstream file;
    file.open(_log_path, std::ios::in);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read verifier log"));
    }

    std::string line;
    while (getline(file, line)) {
        if (line == "VERIFICATION SUCCESSFUL") {
            is_sat = true;
        } else if (line == "VERIFICATION FAILED") {
            is_sat = false;
        }

        if (utils::trim(line).starts_with("Vulnerability Found: ")) {
            vulnerability = utils::trim(line).substr(21);
        }
    }

    return std::make_tuple(is_sat, vulnerability);
}




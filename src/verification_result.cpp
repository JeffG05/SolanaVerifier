#include <fstream>
#include "verification_result.h"

verification_result::verification_result(const verifier v, const std::filesystem::path &path) {
    _verifier = v;
    _log_path = path;

    auto [is_sat] = parse_log();
    _is_sat = is_sat;
}

verification_result::verification_result(const verifier v, const std::string &path) : verification_result(v, std::filesystem::path(path)) {}

bool verification_result::get_is_sat() const {
    return _is_sat;
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
    }

    return std::make_tuple(is_sat);
}




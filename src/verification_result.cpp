#include <fstream>
#include "verification_result.h"
#include "utils.h"

verification_result::verification_result(const std::filesystem::path &path) {
    _log_path = path;

    auto [is_sat, vulnerability, error] = parse_log();
    _is_sat = is_sat;
    _vulnerability = vulnerability;
    _error = error;
}

verification_result::verification_result(const std::string &path) : verification_result(std::filesystem::path(path)) {}

bool verification_result::get_is_sat() const {
    return _is_sat;
}

std::optional<vulnerability> verification_result::get_vulnerability() const {
    return _vulnerability;
}

std::optional<std::string> verification_result::get_error() const {
    return _error;
}

result verification_result::parse_log() const {
    bool is_sat;
    std::optional<vulnerability> vulnerability_found = std::nullopt;
    std::optional<std::string> error_found = std::nullopt;

    std::fstream file;
    file.open(_log_path, std::ios::in);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read verifier log"));
    }

    std::string line;
    while (getline(file, line)) {
        if (utils::trim(line).starts_with("ERROR: ")) {
            error_found = utils::trim(line).substr(7);
        }

        if (line == "VERIFICATION SUCCESSFUL") {
            is_sat = true;
        } else if (line == "VERIFICATION FAILED") {
            is_sat = false;
        }

        if (utils::trim(line).starts_with("Vulnerability Found: ")) {
            std::string vulnerability_id = utils::trim(line).substr(21);
            if (std::ranges::all_of(vulnerability_id, ::isdigit)) {
                int vulnerability_enum = std::stoi(vulnerability_id);
                const auto type = static_cast<vulnerability_type>(vulnerability_enum);
                vulnerability_found = vulnerability(type);
            } else {
                vulnerability_found = std::nullopt;
            }
        }
    }

    return std::make_tuple(is_sat, vulnerability_found, error_found);
}




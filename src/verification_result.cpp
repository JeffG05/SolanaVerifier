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
            continue;
        }
        if (line == "VERIFICATION FAILED") {
            is_sat = false;
            continue;
        }

        if (utils::trim(line).starts_with("Vulnerability Found: ")) {
            std::list<std::string> result_infos = utils::split(utils::trim(line), "; ");
            auto result_itr = result_infos.begin();

            std::optional<vulnerability_type> vulnerability_type_found;
            std::optional<std::string> vulnerability_reason_found;

            std::string vulnerability_id = result_itr->substr(21);
            if (std::ranges::all_of(vulnerability_id, ::isdigit)) {
                int vulnerability_enum = std::stoi(vulnerability_id);
                vulnerability_type_found = static_cast<vulnerability_type>(vulnerability_enum);
            }
            if (!vulnerability_type_found.has_value()) {
                vulnerability_found = std::nullopt;
            }

            if (++result_itr == result_infos.end()) {
                vulnerability_found = vulnerability(vulnerability_type_found.value(), std::nullopt);
                continue;
            }

            vulnerability_reason_found = result_itr->substr(8);
            vulnerability_found = vulnerability(vulnerability_type_found.value(), vulnerability_reason_found);
        }
    }

    return std::make_tuple(is_sat, vulnerability_found, error_found);
}




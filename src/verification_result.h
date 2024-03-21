#ifndef VERIFICATION_RESULT_H
#define VERIFICATION_RESULT_H

#include <filesystem>
#include <list>
#include <optional>

#include "vulnerability.h"

typedef std::tuple<bool, std::list<vulnerability>, std::optional<std::string>> result;

class verification_result {
public:
    explicit verification_result(const std::filesystem::path &path);
    explicit verification_result(const std::string &path);

    [[nodiscard]] bool get_is_sat() const;
    [[nodiscard]] std::list<vulnerability> get_vulnerabilities() const;
    [[nodiscard]] std::optional<std::string> get_error() const;

    [[nodiscard]] result parse_log() const;
private:
    std::filesystem::path _log_path;

    bool _is_sat;
    std::list<vulnerability> _vulnerabilities;
    std::optional<std::string> _error;
};



#endif //VERIFICATION_RESULT_H

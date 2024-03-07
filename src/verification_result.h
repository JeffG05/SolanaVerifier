#ifndef VERIFICATION_RESULT_H
#define VERIFICATION_RESULT_H

#include <filesystem>
#include <optional>

#include "vulnerability.h"

typedef std::tuple<bool, std::optional<vulnerability>, std::optional<std::string>> result;

class verification_result {
public:
    explicit verification_result(const std::filesystem::path &path);
    explicit verification_result(const std::string &path);

    [[nodiscard]] bool get_is_sat() const;
    [[nodiscard]] std::optional<vulnerability> get_vulnerability() const;
    [[nodiscard]] std::optional<std::string> get_error() const;

    [[nodiscard]] result parse_log() const;
private:
    std::filesystem::path _log_path;

    bool _is_sat;
    std::optional<vulnerability> _vulnerability;
    std::optional<std::string> _error;
};



#endif //VERIFICATION_RESULT_H

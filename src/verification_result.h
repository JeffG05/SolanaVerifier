#ifndef VERIFICATION_RESULT_H
#define VERIFICATION_RESULT_H

#include <filesystem>

typedef std::tuple<bool> result;

enum class verifier {
    z3
};

class verification_result {
public:
    explicit verification_result(verifier v, const std::filesystem::path &path);
    explicit verification_result(verifier v, const std::string &path);

    bool get_is_sat() const;

    [[nodiscard]] result parse_log() const;
    [[nodiscard]] result parse_z3() const;
private:
    std::filesystem::path _log_path;
    verifier _verifier;
    bool _is_sat;
};



#endif //VERIFICATION_RESULT_H

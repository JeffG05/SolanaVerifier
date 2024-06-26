#ifndef C_PROGRAM_H
#define C_PROGRAM_H

#include <filesystem>
#include "verification_result.h"

class c_program {
public:
    explicit c_program(const std::string &contract_name, const std::filesystem::path& path);
    explicit c_program(const std::string &contract_name, const std::string& path);

    [[nodiscard]] std::string get_path() const;
    [[nodiscard]] std::string get_dir() const;

    [[nodiscard]] verification_result verify(const std::filesystem::path& target, const std::filesystem::path& esbmc_path, const std::string& smt_solver) const;

private:
    std::string _contract_name;
    std::filesystem::path _path;
};

#endif //C_PROGRAM_H

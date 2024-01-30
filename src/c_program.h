#ifndef C_PROGRAM_H
#define C_PROGRAM_H

#include <filesystem>
#include "smt_formula.h"
#include "verification_result.h"

class c_program {
public:
    explicit c_program(const std::string &contract_name, const std::filesystem::path& path);
    explicit c_program(const std::string &contract_name, const std::string& path);

    [[nodiscard]] std::string get_path() const;

    [[nodiscard]] smt_formula build_smt(const std::filesystem::path& target, const std::filesystem::path& esbmc_path) const;
    [[nodiscard]] verification_result verify_z3(const std::filesystem::path& target, const std::filesystem::path& esbmc_path) const;

protected:
    std::string _contract_name;
    std::filesystem::path _path;
};

#endif //C_PROGRAM_H

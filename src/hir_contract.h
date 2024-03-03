#ifndef HIR_CONTRACT_H
#define HIR_CONTRACT_H
#include <filesystem>

#include "mir_statement.h"


class hir_contract {
public:
    explicit hir_contract(const std::filesystem::path& path);
    explicit hir_contract(const std::string& path);

    [[nodiscard]] std::string get_path() const;
    [[nodiscard]] mir_statements get_structs() const;
    [[nodiscard]] std::string get_target() const;
private:
    std::filesystem::path _path;
    mir_statements _structs;
    std::string _target;

    static std::string trim_line(const std::string& line);
    [[nodiscard]] mir_statements extract_structs() const;
    [[nodiscard]] std::string extract_target() const;
};

#endif //HIR_CONTRACT_H

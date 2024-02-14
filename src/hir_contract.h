#ifndef HIR_CONTRACT_H
#define HIR_CONTRACT_H
#include <filesystem>

#include "mir_statement.h"


class hir_contract {
public:
    explicit hir_contract(const std::filesystem::path& path);
    explicit hir_contract(const std::string& path);

    [[nodiscard]] std::string get_path() const;

    mir_statements extract_structs() const;
private:
    std::filesystem::path _path;
};

#endif //HIR_CONTRACT_H

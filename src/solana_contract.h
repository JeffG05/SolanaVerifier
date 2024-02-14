#ifndef SOLANA_CONTRACT_H
#define SOLANA_CONTRACT_H

#include <string>
#include <filesystem>

#include "hir_contract.h"
#include "mir_contract.h"

class solana_contract {
public:
    explicit solana_contract(const std::filesystem::path& path);
    explicit solana_contract(const std::string& path);

    [[nodiscard]] std::string get_path() const;
    [[nodiscard]] std::string get_manifest_path() const;
    [[nodiscard]] std::string get_name() const;

    [[nodiscard]] hir_contract convert_to_hir(const std::filesystem::path& target) const;
    [[nodiscard]] mir_contract convert_to_mir(const std::filesystem::path& target, const mir_statements& structs) const;

protected:
    std::filesystem::path _contract_dir;
};



#endif //SOLANA_CONTRACT_H

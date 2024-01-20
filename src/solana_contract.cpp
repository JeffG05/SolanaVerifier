#include <toml++/toml.hpp>
#include "solana_contract.h"

solana_contract::solana_contract(const std::filesystem::path &path) {
    _contract_dir = path;
}

solana_contract::solana_contract(const std::string &path) : solana_contract(std::filesystem::path(path)) {}


std::string solana_contract::get_path() const {
    return _contract_dir.string();
}

std::string solana_contract::get_manifest_path() const {
    return (_contract_dir / "Cargo.toml").string();
}

std::string solana_contract::get_name() const {
    try {
        toml::table tbl = toml::parse_file(get_manifest_path());
        return tbl["package"]["name"].value_or("");
    } catch (const toml::v3::ex::parse_error&) {
        return "";
    }
}

mir_contract solana_contract::convert_to_mir(const std::filesystem::path& target) const {
    // Copy contract code to target location
    copy(_contract_dir, target, std::filesystem::copy_options::recursive);

    // Run command to generate mir
    std::stringstream cmd;
    cmd << "cd " << target << " && cargo rustc -- -o result.mir --emit mir";
    system(cmd.str().data());

    // Return mir contract
    const std::string mir_path = (target / "result.mir").string();
    return {get_name(), mir_path};
}







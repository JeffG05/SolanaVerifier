#include <toml++/toml.hpp>
#include "solana_contract.h"

#include <iostream>
#include <regex>

solana_contract::solana_contract(const std::filesystem::path &path, const config globals) {
    _contract_dir = path;
    _globals = globals;
}

solana_contract::solana_contract(const std::string &path, const config globals) : solana_contract(std::filesystem::path(path), globals) {}


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

mir_contract solana_contract::convert_to_mir(const std::filesystem::path &target, const mir_statements& structs) const {
    // Copy contract code to target location
    if (is_empty(target)) {
        copy(_contract_dir, target, std::filesystem::copy_options::recursive);
        edit_rust_files(target);
    }

    // Run command to generate mir
    std::stringstream cmd;
    cmd << "cd " << target << " && cargo rustc -- -o result.mir --emit mir -C overflow-checks=false -C debug-assertions=false > /dev/null 2>&1";
    system(cmd.str().data());

    // Return mir contract
    const std::string mir_path = (target / "result.mir").string();
    return {get_name(), mir_path, structs, _globals};
}

hir_contract solana_contract::convert_to_hir(const std::filesystem::path& target) const {
    // Copy contract code to target location
    if (is_empty(target)) {
        copy(_contract_dir, target, std::filesystem::copy_options::recursive);
        edit_rust_files(target);
    }

    // Run command to generate hir
    std::stringstream cmd;
    cmd << "cd " << target << " && cargo rustc -- -o result.hir --Z unpretty=hir -C overflow-checks=false -C debug-assertions=false > /dev/null 2>&1";
    system(cmd.str().data());

    // Return mir contract
    const std::string hir_path = (target / "result.hir").string();
    return hir_contract(hir_path);
}

void solana_contract::edit_rust_files(const std::filesystem::path &target) {
    for (const auto& file : std::filesystem::recursive_directory_iterator(target)) {
        if (file.is_regular_file() && file.path().extension() == ".rs" && file.path().parent_path().stem() == "src") {
            edit_rust_file(file.path());
        }
    }
}

void solana_contract::edit_rust_file(const std::filesystem::path &file_path) {
    std::fstream file;
    file.open(file_path, std::ios::in);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read Rust file"));
    }

    std::string temp_file_path = file_path.string() + ".temp";
    std::fstream temp_file;
    temp_file.open(temp_file_path, std::ios::out);
    if (!temp_file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to write temp Rust file"));
    }

    std::string line;
    std::regex num_const_regex (R"((.*?)((?:[ui](?:8|16|32|64|size)|f(?:8|16|32|64)))::(MAX|MIN)(.*))");
    std::smatch match;
    while (getline(file, line)) {
        std::string updated_line;
        while (std::regex_match(line, match, num_const_regex)) {
            updated_line += match[1].str();
            if (match[3].str() == "MAX") {
                std::string t = match[2].str();
                if (t == "u8") {
                    updated_line += "255u8";
                } else if (t == "u16") {
                    updated_line += "65535u16";
                } else if (t == "u32") {
                    updated_line += "4294967295u32";
                } else if (t == "u64") {
                    updated_line += "18446744073709551615u64";
                } else if (t == "usize") {
                    updated_line += "18446744073709551615usize";
                } else if (t == "i8") {
                    updated_line += "127i8";
                } else if (t == "i16") {
                    updated_line += "32767i16";
                } else if (t == "i32") {
                    updated_line += "2147483647i32";
                } else if (t == "i64") {
                    updated_line += "9223372036854775807i64";
                } else if (t == "isize") {
                    updated_line += "9223372036854775807isize";
                } else if (t == "f64") {
                    updated_line += "1.7976931348623157E+308f64";
                } else if (t == "f32") {
                    updated_line += "3.40282347E+38f32";
                } else {
                    updated_line += match[2].str() + "::" + match[3].str();
                }
            } else {
                std::string t = match[2].str();
                if (t == "u8") {
                    updated_line += "0u8";
                } else if (t == "u16") {
                    updated_line += "0u16";
                } else if (t == "u32") {
                    updated_line += "0u32";
                } else if (t == "u64") {
                    updated_line += "0u64";
                } else if (t == "usize") {
                    updated_line += "0usize";
                } else if (t == "i8") {
                    updated_line += "-128i8";
                } else if (t == "i16") {
                    updated_line += "-32768i16";
                } else if (t == "i32") {
                    updated_line += "-2147483648i32";
                } else if (t == "i64") {
                    updated_line += "-9223372036854775808i64";
                } else if (t == "isize") {
                    updated_line += "-9223372036854775808isize";
                } else if (t == "f64") {
                    updated_line += "-1.7976931348623157E+308f64";
                } else if (t == "f32") {
                    updated_line += "-3.40282347E+38f32";
                } else {
                    updated_line += match[2].str() + "::" + match[3].str();
                }
            }
            line = match[4].str();
        }
        updated_line += line;
        temp_file << updated_line << std::endl;
    }

    file.close();
    temp_file.close();

    copy(temp_file_path, file_path, std::filesystem::copy_options::overwrite_existing);
    remove(temp_file_path.c_str());
}

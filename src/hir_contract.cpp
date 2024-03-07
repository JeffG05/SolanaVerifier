#include <fstream>
#include "hir_contract.h"

#include <iostream>
#include <regex>

#include "mir_contract.h"
#include "utils.h"

hir_contract::hir_contract(const std::filesystem::path &path) {
    _path = absolute(path);
    _structs = extract_structs();
    _target = extract_target();
}

hir_contract::hir_contract(const std::string &path) : hir_contract(std::filesystem::path(path)) {}

std::string hir_contract::get_path() const {
    return _path.string();
}

mir_statements hir_contract::get_structs() const {
    return _structs;
}

std::string hir_contract::get_target() const {
    return _target;
}

mir_statements hir_contract::extract_structs() const {
    std::fstream file;
    file.open(_path, std::ios::in);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read HIR file"));
    }

    mir_statements structs;

    std::string line;
    bool struct_active = false;
    unsigned int var_i = 0;
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (!struct_active) {
            if (trimmed_line.starts_with("struct ")) {
                std::regex struct_regex (R"(^struct (.+) \{$)");
                std::smatch match;
                if (std::regex_match(trimmed_line, match, struct_regex)) {
                    nlohmann::json data;
                    data["name"] = match[1].str();
                    auto root = mir_statement(statement_type::data_struct, data);
                    structs.push_back(root);
                    struct_active = true;
                }
            }
            continue;
        }

        if (trimmed_line.starts_with("//")) {
            continue;
        }

        if (trimmed_line == "}") {
            struct_active = false;
            var_i = 0;
            continue;
        }

        std::regex field_regex (R"(^(.+): (.+),$)");
        std::smatch match;
        if (std::regex_match(trimmed_line, match, field_regex)) {
            nlohmann::json data;
            data["variable"] = "get" + std::to_string(var_i);
            data["variable_type"] = mir_statement::convert_type(match[2].str());
            auto field = mir_statement(statement_type::variable, data);
            structs.back().add_child(field);
            var_i++;
        }
    }

    file.close();
    return structs;
}

std::string hir_contract::extract_target() const {
    std::fstream file;
    file.open(_path, std::ios::in);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read HIR file"));
    }

    std::string line;
    bool entrypoint_found = false;
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (trimmed_line == "unsafe extern \"C\" fn entrypoint(input: *mut u8)") {
            entrypoint_found = true;
        }

        const std::regex entrypoint_def (R"(^match (.+)\(.+, .+, .+\) \{$)");
        std::smatch match;
        if (entrypoint_found && std::regex_match(trimmed_line, match, entrypoint_def)) {
            return match[1].str();
        }
    }

    file.close();
    std::throw_with_nested(std::runtime_error("No entrypoint found"));
}

std::string hir_contract::trim_line(const std::string &line) {
    std::string trimmed = utils::trim(line);
    const std::regex comment_regex (R"(^(.*?[;{])?(?:\s*\/\/.*)$)");
    if (std::smatch match; std::regex_match(trimmed, match, comment_regex)) {
        return match[1].str();
    }
    return trimmed;
}


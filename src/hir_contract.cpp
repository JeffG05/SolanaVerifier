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
    std::optional<mir_statement> subenum;

    std::string line;
    bool struct_active = false;
    bool enum_active = false;
    unsigned int var_i = 0;
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (!struct_active && !enum_active) {
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
            } else if (trimmed_line.starts_with("enum ")) {
                std::regex enum_regex (R"(^enum (.+) \{$)");
                std::smatch match;
                if (std::regex_match(trimmed_line, match, enum_regex)) {
                    nlohmann::json data;
                    data["name"] = match[1].str();
                    auto root = mir_statement(statement_type::data_enum, data);
                    structs.push_back(root);
                    enum_active = true;
                }
            }
            continue;
        }

        if (trimmed_line.starts_with("//")) {
            continue;
        }

        if (trimmed_line == "}") {
            struct_active = false;
            enum_active = false;
            var_i = 0;
            continue;
        }

        if (struct_active) {
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
        } else if (subenum.has_value()) {
            std::regex field_regex (R"(^(.+): (.+),$)");
            std::smatch match;
            if (trimmed_line == "},") {
                structs.push_front(subenum.value());
                subenum = std::nullopt;
                var_i = 0;
            } else if (std::regex_match(trimmed_line, match, field_regex)) {
                nlohmann::json data;
                data["variable"] = "get" + std::to_string(var_i);
                data["variable_type"] = mir_statement::convert_type(match[2].str());
                auto field = mir_statement(statement_type::variable, data);
                subenum->add_child(field);
                var_i++;
            }
        } else {
            std::regex complex_value_regex (R"(^([^}\s]+) \{$)");
            std::smatch match;
            if (std::regex_match(trimmed_line, match, complex_value_regex)) {
                std::string subenum_name = structs.back().get_ast_data().at("name").get<std::string>() + "_" + match[1].str();

                nlohmann::json data;
                data["variable"] = "value_" + utils::to_lower(match[1].str());
                data["variable_type"] = subenum_name;
                auto field = mir_statement(statement_type::variable, data);
                structs.back().add_child(field);

                nlohmann::json subenum_data;
                subenum_data["name"] = subenum_name;
                auto subenum_field = mir_statement(statement_type::data_enum_struct, subenum_data);
                subenum = subenum_field;
            }
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
    bool target_line_found = false;
    std::string target_line;
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (trimmed_line == "unsafe extern \"C\" fn entrypoint(input: *mut u8)") {
            entrypoint_found = true;
        }

        if (entrypoint_found && (target_line_found || trimmed_line.starts_with("match "))) {
            target_line_found = true;
            target_line += trimmed_line;
            if (trimmed_line.ends_with(" {")) {
                const std::regex entrypoint_def (R"(^match (.+)\(.+,\s?.+,\s?.+\) \{$)");
                std::smatch match;
                if (std::regex_match(target_line, match, entrypoint_def)) {
                    return match[1].str();
                }
            }
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


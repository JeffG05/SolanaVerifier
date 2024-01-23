#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <list>
#include <regex>
#include <cmrc/cmrc.hpp>
#include "mir_contract.h"
#include "mir_statement.h"
#include "utils.h"

CMRC_DECLARE(SolanaVerifierAssets);

mir_contract::mir_contract(const std::string &contract_name, const std::filesystem::path &path) {
    _contract_name = contract_name;
    _path = path;
}

mir_contract::mir_contract(const std::string &contract_name, const std::string &path) : mir_contract(contract_name, std::filesystem::path(path)) {}


std::string mir_contract::get_path() const {
    return _path;
}

c_program mir_contract::convert_to_c(const std::filesystem::path& target, const std::string& target_funtion) const {
    // Open file in read mode
    std::fstream file;
    file.open(_path, std::ios::in);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read MIR file"));
    }

    // Create AST tree
    const mir_statement ast_tree = create_ast_tree(file);
    file.close();

    // Export AST tree as C program
    const std::filesystem::path c_file_path = export_c_program(target, ast_tree, target_funtion);

    // Export library file
    export_library_file(target);

    return c_program(_contract_name, c_file_path);

}

mir_statement mir_contract::create_ast_tree(std::istream& file) const {
    // Init statement tree
    auto root = mir_statement::create_root(_contract_name);

    std::string line;
    auto current_statement_lines = std::list<std::string>();
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (mir_statement::line_is_statement_start(trimmed_line)) {
            if (!current_statement_lines.empty()) {
                if (std::optional<mir_statement> statement = mir_statement::parse_lines(current_statement_lines); statement.has_value()) {
                    root.add_child(statement.value());
                }
                current_statement_lines.clear();
            }
        }

        current_statement_lines.push_back(trimmed_line);
    }
    if (!current_statement_lines.empty()) {
        if (const std::optional<mir_statement> statement = mir_statement::parse_lines(current_statement_lines);
            statement.has_value()) {
            root.add_child(statement.value());
        }
        current_statement_lines.clear();
    }

    return root;
}

std::filesystem::path mir_contract::export_c_program(const std::filesystem::path &target, mir_statement ast_tree, const std::string& target_function) {
    nlohmann::json contract_data = ast_tree.get_ast_data();
    const std::string contract_name = contract_data.at("contract").get<std::string>();

    const std::filesystem::path out = target / (contract_name + ".c");
    std::fstream file;
    file.open(out, std::ios::out);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to write C program"));
    }

    // Add import statements
    file << "#include <assert.h>" << std::endl;
    file << "#include <stdbool.h>" << std::endl;
    file << "#include \"solana.c\"" << std::endl;
    file << std::endl;

    std::list<mir_statement> functions = ast_tree.get_children();
    for (auto function_statement: functions) {
        nlohmann::json function_data = function_statement.get_ast_data();
        const std::string function_name = function_data.at("name").get<std::string>();
        const std::string function_return = function_data.at("return_type").get<std::string>();
        std::list<mir_statement> function_parameters = function_statement.get_children({ parameter });
        std::list<mir_statement> function_variables = function_statement.get_children({ variable });
        std::list<mir_statement> function_blocks = function_statement.get_children({block});
        std::list<mir_statement> function_states;
        std::ranges::copy(function_parameters, std::back_insert_iterator(function_states));
        std::ranges::copy(function_variables, std::back_insert_iterator(function_states));

        // Create function state struct
        file << "typedef struct " << function_name << "_state_struct {" << std::endl;
        for (const auto& parameter_statement: function_states) {
            nlohmann::json parameter_data = parameter_statement.get_ast_data();
            const std::string parameter_name = parameter_data.at("variable").get<std::string>();
            const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
            file << "\t" << ast_variable_to_c(parameter_type, parameter_name) << ";" << std::endl;
        }
        file << "} " << function_name << "_state;" << std::endl;
        file << std::endl;

        // Create block functions
        for (auto block_statement : std::ranges::reverse_view(function_blocks)) {
            nlohmann::json block_data = block_statement.get_ast_data();
            const auto block_name = block_data.at("name").get<std::string>();
            std::list<mir_statement> block_assignments = block_statement.get_children({assignment});

            file << function_name << "_state " << function_name << "_" << block_name << "(" << function_name << "_state state) {" << std::endl;

            // Add assignments
            for (auto assignment_statement : block_assignments) {
                nlohmann::json assignment_data = assignment_statement.get_ast_data();
                const std::string assignment_variable = assignment_data.at("variable").get<std::string>();
                const std::string assignment_value = assignment_data.at("value").get<std::string>();
                std::list<mir_statement> assignment_branches = assignment_statement.get_children({branch});

                file << "\tstate." << assignment_variable << " = " << assignment_value << ";" << std::endl;

                // Add branching logic
                auto branch_it = assignment_branches.begin();
                for (int i = 0; i < assignment_branches.size(); ++i) {
                    nlohmann::json branch_data = branch_it->get_ast_data();
                    const std::string branch_condition = branch_data.at("condition").get<std::string>();
                    const std::string branch_destination = branch_data.at("destination").get<std::string>();
                    file << "\tif (state." << assignment_variable << " == " << branch_condition << ") {" << std::endl;
                    file << "\t\treturn " << function_name << "_" << branch_destination << "(state);" << std::endl;
                    if (i == assignment_branches.size() - 1) {
                        file << "\t} else {" << std::endl;
                        file << "\t\tassert(false);" << std::endl;
                        file << "\t}" << std::endl;
                    } else {
                        file << "\t} else ";
                    }
                    ++branch_it;
                }
            }
            file << "\treturn state;" << std::endl;
            file << "}" << std::endl;
            file << std::endl;
        }

        // Create function header
        file << function_return << " " << function_name << "(";
        unsigned int i = 0;
        for (const auto& parameter_statement: function_parameters) {
            nlohmann::json parameter_data = parameter_statement.get_ast_data();
            const std::string parameter_name = parameter_data.at("variable").get<std::string>();
            const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
            file << ast_variable_to_c(parameter_type, parameter_name);
            if (i < function_parameters.size() - 1) {
                file << ", ";
            }
            i++;
        }
        file << ") {" << std::endl;

        // Init function state
        file << "\t" << function_name << "_state state;" << std::endl;
        for (const auto& parameter_statement: function_states) {
            nlohmann::json parameter_data = parameter_statement.get_ast_data();
            const std::string parameter_name = parameter_data.at("variable").get<std::string>();
            const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
            if (parameter_type.starts_with("array ")) {
                file << "\tunsigned int i" << parameter_name << " = nondet_u8();" << std::endl;
                file << "\tstate." << parameter_name << "[i" << parameter_name << "] = ";
                if (parameter_statement.get_type() == parameter) {
                    file << parameter_name << "[i" << parameter_name << "];" << std::endl;
                } else {
                    file << "nondet_" << ast_variable_to_c(parameter_type) << "();" << std::endl;
                }
            } else {
                file << "\tstate." << parameter_name << " = ";
                if (parameter_statement.get_type() == parameter) {
                    file << parameter_name << ";" << std::endl;
                } else {
                    file << "nondet_" << parameter_type << "();" << std::endl;
                }
            }
        }
        file << std::endl;

        // Call first block
        const std::string first_block = function_blocks.front().get_ast_data().at("name").get<std::string>();
        file << "\tstate = " << function_name << "_" << first_block << "(state);" << std::endl;
        file << std::endl;

        // Return value
        file << "\treturn state._0;" << std::endl;
        file << "}" << std::endl;
        file << std::endl;
    }

    // Get target function in AST
    auto function_it = functions.begin();
    while (function_it != functions.end() && function_it->get_ast_data().at("name").get<std::string>() != target_function) {
        ++function_it;
    }
    if (function_it == functions.end()) {
        std::throw_with_nested(std::runtime_error("Target function not found"));
    }
    mir_statement target_function_statement = *function_it;
    const std::string target_function_type = target_function_statement.get_ast_data().at("return_type").get<std::string>();

    // Create main function
    file << "int main() {" << std::endl;
    for (const auto& parameter_statement: target_function_statement.get_children({parameter})) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        file << "\t" << ast_variable_to_c(parameter_type, parameter_name) << ";" << std::endl;
    }
    file << std::endl;
    for (const auto& parameter_statement: target_function_statement.get_children({parameter})) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        if (parameter_type.starts_with("array ")) {
            file << "\t" << parameter_name << "[nondet_u8()] = nondet_" << ast_variable_to_c(parameter_type) << "();" << std::endl;
        } else {
            file << "\t" << parameter_name << " = nondet_" << parameter_type << "();" << std::endl;
        }
    }
    file << std::endl;


    file << "\t" << ast_variable_to_c(target_function_type, "result") << " = " << target_function << "(";

    unsigned int i = 0;
    std::list<mir_statement> target_parameters = target_function_statement.get_children({parameter});
    for (const auto& parameter_statement: target_parameters) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        file << parameter_name;
        if (i < target_parameters.size() - 1) {
            file << ", ";
        }
        i++;
    }
    file << ");" << std::endl;
    file << std::endl;
    file << "\treturn 0;" << std::endl;
    file << "}" << std::endl;

    file.close();
    return out;
}

std::string mir_contract::ast_variable_to_c(const std::string &type, const std::string &name) {
    if (type.starts_with("array ")) {
        const std::string subtype = type.substr(6);
        return subtype + " " + name + "[256]";
    }
    return type + " " + name;
}

std::string mir_contract::ast_variable_to_c(const std::string &type) {
    if (type.starts_with("array ")) {
        const std::string subtype = type.substr(6);
        return subtype;
    }
    return type;
}

std::filesystem::path mir_contract::export_library_file(const std::filesystem::path &target) {
    const std::filesystem::path out = target / "solana.c";
    std::fstream file;
    file.open(out, std::ios::out);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to write external library"));
    }

    const auto asset_fs = cmrc::SolanaVerifierAssets::get_filesystem();
    const cmrc::file lib_file = asset_fs.open("assets/solana.c");

    file << std::string(lib_file.begin(), lib_file.end()) << std::endl;
    file.close();
    return out;
}

std::string mir_contract::trim_line(const std::string &line) {
    std::string trimmed = utils::trim(line);
    const std::regex comment_regex (R"(^(.*?[;{])?(?:\s*\/\/.*)$)");
    if (std::smatch match; std::regex_match(trimmed, match, comment_regex)) {
        return match[1].str();
    }
    return trimmed;
}

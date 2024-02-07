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
    mir_statement ast_tree = create_ast_tree(file);
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

    mir_statements functions = ast_tree.get_children();
    for (auto function_statement: std::ranges::reverse_view(functions)) {
        nlohmann::json function_data = function_statement.get_ast_data();
        const std::string function_name = function_data.at("name").get<std::string>();
        const std::string function_return = function_data.at("return_type").get<std::string>();
        
        mir_statements function_parameters = function_statement.get_children({statement_type::parameter });
        mir_statements function_variables = function_statement.get_children({statement_type::variable });
        mir_statements function_blocks = function_statement.get_children({statement_type::block});

        mir_statements function_states;
        std::ranges::copy(function_parameters, std::back_insert_iterator(function_states));
        std::ranges::copy(function_variables, std::back_insert_iterator(function_states));

        reference_map references;

        generate_result_structs(&file, function_states, function_name);
        generate_tuple_structs(&file, function_states, function_name);
        generate_controlflow_structs(&file, function_states, function_name);
        generate_function_struct(&file, function_states, function_name);

        for (const auto& block_statement : std::ranges::reverse_view(function_blocks)) {
            generate_block_function(&file, block_statement, function_name, &references);
        }

        generate_function(&file, function_states, function_name, function_return);
    }

    generate_main_function(&file, functions, target_function);

    file.close();
    return out;
}

std::string mir_contract::get_c_type(const std::string &type, const std::string &name, const std::string &function_name) {
    if (type.starts_with("array<")) {
        return get_c_subtype(type) + " " + name + "[256]";
    }
    if (type.starts_with("tuple<")) {
        const std::string tuple_name = get_tuple_name(type, function_name);
        return tuple_name + " " + name;
    }
    if (type.starts_with("result<")) {
        const std::string result_name = get_result_name(type, function_name);
        return result_name + " " + name;
    }
    if (type.starts_with("controlflow<")) {
        const std::string controlflow_name = get_controlflow_name(type, function_name);
        return controlflow_name + " " + name;
    }
    return type + " " + name;
}

std::string mir_contract::get_return_c_type(const std::string &type, const std::string &name, const std::string &function_name) {
    const std::string c_type = get_c_type(type, "_0", function_name);
    if (!c_type.ends_with(" _0")) {
        std::throw_with_nested(std::runtime_error("Unsupported return type: " + type));
    }

    return c_type.substr(0, c_type.size() - 3);
}


std::string mir_contract::get_c_subtype(const std::string &type) {
    if (type.starts_with("array<")) {
        const std::string subtype = type.substr(6, type.size()-7);
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

void mir_contract::generate_tuple_structs(std::ostream *out, const mir_statements &state_statements, const std::string &function_name) {
    for (const auto& parameter_statement: state_statements) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        if (!parameter_type.starts_with("tuple<")) {
            continue;
        }

        std::string struct_name = get_tuple_name(parameter_type, function_name);
        std::string raw_value_types = parameter_type.substr(6, parameter_type.size() - 7);
        std::list<std::string> value_types = utils::split(raw_value_types, ", ");

        *out << "typedef struct " << struct_name << "_struct {" << std::endl;
        unsigned int tuple_i = 0;
        for (const auto& value_type: value_types) {
            *out << "\t" << value_type << " get" << tuple_i << ";" << std::endl;
            tuple_i++;
        }
        *out << "} " << struct_name << ";" << std::endl;

        *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
        *out << "\t" << struct_name << " t;" << std::endl;
        unsigned int tuple_i2 = 0;
        for (const auto& value_type: value_types) {
            *out << "\tt.get" << tuple_i2 << " = nondet_" << clean_type(value_type) << "();" << std::endl;
            tuple_i2++;
        }
        *out << "\treturn t;" << std::endl;
        *out << "}" << std::endl;

        *out << std::endl;
    }
}

void mir_contract::generate_result_structs(std::ostream *out, const mir_statements &state_statements, const std::string &function_name) {
    for (const auto& parameter_statement: state_statements) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        if (!parameter_type.starts_with("result<")) {
            continue;
        }

        std::string struct_name = get_result_name(parameter_type, function_name);
        std::string success_type = parameter_type.substr(7, parameter_type.size() - 8);

        *out << "typedef struct " << struct_name << "_struct {" << std::endl;
        *out << "\tbool is_success;" << std::endl;
        if (success_type != "void") {
            *out << "\t" << success_type << " value;" << std::endl;
        }
        *out << "} " << struct_name << ";" << std::endl;

        *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
        *out << "\t" << struct_name << " r;" << std::endl;
        *out << "\tr.is_success = nondet_bool();" << std::endl;
        if (success_type != "void") {
            *out << "\tr.value = nondet_" << success_type << "();" << std::endl;
        }
        *out << "\treturn r;" << std::endl;
        *out << "}" << std::endl;

        *out << std::endl;
    }
}

void mir_contract::generate_controlflow_structs(std::ostream *out, const mir_statements &state_statements, const std::string &function_name) {
    for (const auto& parameter_statement: state_statements) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        if (!parameter_type.starts_with("controlflow<")) {
            continue;
        }

        std::string struct_name = get_controlflow_name(parameter_type, function_name);
        std::string raw_controlflow_types = parameter_type.substr(12, parameter_type.size() - 13);
        std::pair<std::string, std::string> types = get_argument_pair(raw_controlflow_types);

        *out << "typedef struct " << struct_name << "_struct {" << std::endl;
        *out << "\tcontrolflow type;" << std::endl;
        if (types.first != "void") {
            *out << "\t" << get_c_type(types.first, "continue_value", function_name) << std::endl;
        }
        if (types.second != "void") {
            *out << "\t" << get_c_type(types.second, "break_value", function_name) << std::endl;
        }
        *out << "} " << struct_name << ";" << std::endl;

        *out << std::endl;
    }
}

void mir_contract::generate_function_struct(std::ostream *out, const mir_statements &state_statements, const std::string &function_name) {
    *out << "typedef struct " << function_name << "_state_struct {" << std::endl;
    for (const auto& parameter_statement: state_statements) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        if (parameter_type != "void") {
            *out << "\t" << get_c_type(parameter_type, parameter_name, function_name) << ";" << std::endl;
        }
    }
    *out << "} " << function_name << "_state;" << std::endl;
    *out << std::endl;
}

void mir_contract::generate_block_function(std::ostream *out, mir_statement block_statement, const std::string &function_name, reference_map* references) {
    nlohmann::json block_data = block_statement.get_ast_data();
    const auto block_name = block_data.at("name").get<std::string>();
    const mir_statements statements = block_statement.get_children({statement_type::assignment, statement_type::add_ref, statement_type::remove_ref});

    *out << function_name << "_state " << function_name << "_" << block_name << "(" << function_name << "_state state) {" << std::endl;

    // Add assignments
    for (const auto& statement : statements) {
        generate_block_statement(out, statement, function_name, references);
    }

    *out << "\treturn state;" << std::endl;
    *out << "}" << std::endl;
    *out << std::endl;
}
 
void mir_contract::generate_block_statement(std::ostream *out, mir_statement statement, const std::string &function_name, reference_map *references) {
    nlohmann::json assignment_data = statement.get_ast_data();

    if (statement.get_type() == statement_type::add_ref) {
        const std::string variable1 = assignment_data.at("variable1").get<std::string>();
        const std::string variable2 = assignment_data.at("variable2").get<std::string>();
        (*references)[variable1].push_back(variable2);
        (*references)[variable2].push_back(variable1);
        return;
    }

    if (statement.get_type() == statement_type::remove_ref) {
        const std::string variable1 = assignment_data.at("variable1").get<std::string>();
        const std::string variable2 = assignment_data.at("variable2").get<std::string>();
        (*references)[variable1].remove(variable2);
        (*references)[variable2].remove(variable1);
        return;
    }


    const std::string assignment_variable = assignment_data.at("variable").get<std::string>();
    const std::string assignment_value = assignment_data.at("value").get<std::string>();
    const bool assignment_returns = assignment_data.at("returns").get<bool>();
    mir_statements assignment_branches = statement.get_children({statement_type::branch});

    generate_block_assignment(out, assignment_variable, assignment_value, assignment_returns);

    // Add branching logic
    if (!assignment_branches.empty()) {
        for (const auto& branch_statement : assignment_branches) {
            generate_branch(out, branch_statement, function_name, assignment_variable, assignment_value);
        }
        *out << "{" << std::endl;
        *out << "\t\tassert(false);" << std::endl;
        *out << "\t}" << std::endl;
    }
}

void mir_contract::generate_block_assignment(std::ostream *out, const std::string &variable, const std::string &value, bool returns) {
    if (
        value.starts_with("u_addition(") ||
        value.starts_with("i_addition(") ||
        value.starts_with("u_multiplication(") ||
        value.starts_with("i_multiplication(")
    ) {
        *out << "\tstate." << variable << ".get0 = " << value << ".value;" << std::endl;
        *out << "\tstate." << variable << ".get1 = " << value << ".errors;" << std::endl;
    } else if (value.starts_with("ok<")) {
        *out << "\tstate." << variable << ".is_success = true;" << std::endl;
        const std::string ok_value = value.substr(3, value.size() - 4);
        if (ok_value != "void") {
            generate_block_assignment(out, variable + ".value", ok_value, false);
        }
    } else if (value.starts_with("result_error<")) {
        *out << "\tstate." << variable << ".is_success = false;" << std::endl;
    } else if (!variable.empty()) {
        if (returns) {
            *out << "\tstate." << variable << " = " << value << ";" << std::endl;
        } else {
            *out << "\t" << value << ";" << std::endl;
        }
    }
}


void mir_contract::generate_branch(std::ostream *out, const mir_statement &branch_statement, const std::string &function_name, const std::string &variable, const std::string &value) {
    nlohmann::json branch_data = branch_statement.get_ast_data();
    const std::string branch_condition = branch_data.at("condition").get<std::string>();
    const std::string branch_destination = branch_data.at("destination").get<std::string>();
    const bool branch_ignore_var = branch_data.at("ignore_var").get<bool>();
    if (variable.empty()) {
        *out << "\tif (" << value << " == " << branch_condition << ") {" << std::endl;
    } else if (branch_ignore_var) {
        *out << "\tif (" << branch_condition << ") {" << std::endl;
    } else {
        *out << "\tif (state." << variable << " == " << branch_condition << ") {" << std::endl;
    }
    *out << "\t\treturn " << function_name << "_" << branch_destination << "(state);" << std::endl;
    *out << "\t} else ";
}

void mir_contract::generate_function(std::ostream *out, const mir_statements &state_statements, const std::string &function_name, const std::string &function_return) {
    std::list<std::string> parameters;
    for (const auto& statement: state_statements) {
        if (statement.get_type() != statement_type::parameter) {
            continue;
        }
        nlohmann::json parameter_data = statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        parameters.push_back(get_c_type(parameter_type, parameter_name, function_name));
    }
    const std::string return_type = get_return_c_type(function_return, "_0", function_name);
    *out << return_type << " " << function_name << "(" << utils::join(parameters, ", ") << ") {" << std::endl;

    // Init function state
    *out << "\t" << function_name << "_state state;" << std::endl;
    for (const auto& statement: state_statements) {
        if (statement.get_ast_data()["variable_type"].get<std::string>() != "void") {
            generate_nondet(out, statement, function_name);
        }
    }
    *out << std::endl;

    // Call first block
    *out << "\tstate = " << function_name << "_bb0(state);" << std::endl;
    *out << std::endl;

    // Return value
    *out << "\treturn state._0;" << std::endl;
    *out << "}" << std::endl;
    *out << std::endl;
}

void mir_contract::generate_nondet(std::ostream *out, const mir_statement &statement, const std::string &function_name, bool in_main) {
    nlohmann::json parameter_data = statement.get_ast_data();
    const std::string parameter_name = parameter_data.at("variable").get<std::string>();
    const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();

    if (parameter_type.starts_with("array<")) {
        generate_nondet_array(out, statement, in_main);
    } else {
        if (in_main) {
            *out << "\t" << parameter_name << " = ";
        } else {
            *out << "\tstate." << parameter_name << " = ";
        }
        if (in_main || statement.get_type() == statement_type::variable) {
            *out << "nondet_" << get_return_c_type(parameter_type, parameter_name, function_name) << "();" << std::endl;
        } else {
            *out << parameter_name << ";" << std::endl;
        }
    }

}

void mir_contract::generate_nondet_array(std::ostream *out, const mir_statement& statement, const bool in_main) {
    nlohmann::json parameter_data = statement.get_ast_data();
    const std::string parameter_name = parameter_data.at("variable").get<std::string>();
    const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();

    *out << "\tunsigned int i" << parameter_name << " = nondet_u8();" << std::endl;
    if (in_main) {
        *out << "\t" << parameter_name << "[i" << parameter_name << "] = ";
    } else {
        *out << "\tstate." << parameter_name << "[i" << parameter_name << "] = ";
    }
    if (in_main || statement.get_type() == statement_type::variable) {
        *out << "nondet_" << get_c_subtype(parameter_type) << "();" << std::endl;
    } else {
        *out << parameter_name << "[i" << parameter_name << "];" << std::endl;
    }
}


mir_statement mir_contract::get_target_function(mir_statements function_statements, const std::string &function_name) {
    auto function_it = function_statements.begin();
    while (function_it != function_statements.end() && function_it->get_ast_data().at("name").get<std::string>() != function_name) {
        ++function_it;
    }
    if (function_it == function_statements.end()) {
        std::throw_with_nested(std::runtime_error("Target function not found"));
    }
    return *function_it;
}

void mir_contract::generate_main_function(std::ostream *out, const mir_statements &function_statements, const std::string &target_function_name) {
    mir_statement target_function_statement = get_target_function(function_statements, target_function_name);
    const std::string target_function_type = target_function_statement.get_ast_data().at("return_type").get<std::string>();
    const mir_statements target_function_parameters = target_function_statement.get_children({statement_type::parameter});

    *out << "int main() {" << std::endl;
    for (const auto& parameter_statement: target_function_parameters) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        *out << "\t" << get_c_type(parameter_type, parameter_name, target_function_name) << ";" << std::endl;
    }
    *out << std::endl;

    for (const auto& parameter_statement: target_function_parameters) {
        generate_nondet(out, parameter_statement, target_function_name, true);
    }
    *out << std::endl;


    *out << "\t" << get_return_c_type(target_function_type, "_0", target_function_name) << " result = " << target_function_name << "(";

    unsigned int i = 0;
    mir_statements target_parameters = target_function_statement.get_children({statement_type::parameter});
    for (const auto& parameter_statement: target_parameters) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        *out << parameter_name;
        if (i < target_parameters.size() - 1) {
            *out << ", ";
        }
        i++;
    }
    *out << ");" << std::endl;
    *out << std::endl;
    *out << "\treturn 0;" << std::endl;
    *out << "}" << std::endl;
}

std::pair<std::string, std::string> mir_contract::get_argument_pair(const std::string &raw) {
    const std::regex pattern (R"(^([^<>,]+<[^<>]+>|[^<>,]+), ([^<>,]+<[^<>]+>|[^<>,]+)$)");
    if (std::smatch match; std::regex_match(raw, match, pattern)) {
        return std::make_pair(match[1].str(), match[2].str());
    }
    std::throw_with_nested(std::runtime_error("Error parsing argument pair: " + raw));
}

std::string mir_contract::clean_type(const std::string &type) {
    std::string clean;
    for (const char c : type) {
        if (isalnum(c)) {
            clean += static_cast<char>(tolower(c));
        }
    }
    return clean;
}

std::string mir_contract::get_tuple_name(const std::string &type, const std::string &function_name) {
    const std::string raw_types = type.substr(6, type.size() - 7);
    std::list<std::string> types_list = utils::split(raw_types, ", ");
    for (auto & raw_type : types_list) {
        raw_type = clean_type(raw_type);
    }
    const std::string clean_types = utils::join(types_list, "_");
    return function_name + "_" + clean_types + "_tuple";
}

std::string mir_contract::get_result_name(const std::string &type, const std::string &function_name) {
    const std::string raw_type = type.substr(7, type.size() - 8);
    return function_name + "_" + clean_type(raw_type) + "_result";
}

std::string mir_contract::get_controlflow_name(const std::string &type, const std::string &function_name) {
    const std::string raw_types = type.substr(12, type.size() - 13);
    const auto [continue_type, break_type] = get_argument_pair(raw_types);
    return function_name + "_" + clean_type(continue_type) + "_" + clean_type(break_type) + "_controlflow";
}





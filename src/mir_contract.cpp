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

mir_contract::mir_contract(const std::string &contract_name, const std::filesystem::path &path, const mir_statements &structs, const config globals) {
    _contract_name = contract_name;
    _path = absolute(path);
    _structs = structs;
    _globals = globals;
}

mir_contract::mir_contract(const std::string &contract_name, const std::string &path, const mir_statements &structs, const config globals) : mir_contract(contract_name, std::filesystem::path(path), structs, globals) {}


std::string mir_contract::get_path() const {
    return _path;
}

c_program mir_contract::convert_to_c(const std::filesystem::path& target, const std::string& target_funtion) {
    // Open file in read mode
    std::fstream file;
    file.open(_path, std::ios::in);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to read MIR file"));
    }

    // Create AST tree
    mir_statement ast_tree = create_ast_tree(file);
    file.close();
    ast_tree.print();

    // Export AST tree as C program
    const std::filesystem::path c_file_path = export_c_program(target, ast_tree, target_funtion);

    // Export library file
    export_library_file(target);

    return c_program(_contract_name, c_file_path);

}

mir_statement mir_contract::create_ast_tree(std::istream& file) {
    // Init statement tree
    auto root = mir_statement::create_root(_contract_name);

    for (const auto& struct_statement : _structs) {
        root.add_child(struct_statement);
    }

    std::string line;
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (mir_statement::line_is_function(trimmed_line)) {
            mir_statement func = mir_statement::parse_function_header(trimmed_line);
            _function_names.insert(func.get_ast_data().at("name"));
        }
    }

    file.clear();
    file.seekg(0);

    auto current_statement_lines = std::list<std::string>();
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (mir_statement::line_is_function(trimmed_line)) {
            if (!current_statement_lines.empty() && mir_statement::line_is_function(current_statement_lines.front())) {
                mir_statement statement = mir_statement::parse_function(current_statement_lines, _structs);
                if (!statement.get_ast_data().contains("source") && statement.get_ast_data().at("name") != "entrypoint") {
                    root.add_child(statement);
                }
            }
            current_statement_lines.clear();
        }

        current_statement_lines.push_back(trimmed_line);
    }
    if (!current_statement_lines.empty() && mir_statement::line_is_function(current_statement_lines.front())) {
        mir_statement statement = mir_statement::parse_function(current_statement_lines, _structs);
        if (!statement.get_ast_data().contains("source") && statement.get_ast_data().at("name") != "entrypoint") {
            root.add_child(statement);
        }
    }
    current_statement_lines.clear();

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

    mir_statements structs = ast_tree.get_children({statement_type::data_struct});
    for (auto struct_statement : std::ranges::reverse_view(structs)) {
        generate_struct_struct(&file, struct_statement.get_children(), struct_statement.get_ast_data().at("name"));
    }

    mir_statements functions = ast_tree.get_children({statement_type::function});
    for (auto function_statement : std::ranges::reverse_view(functions)) {
        nlohmann::json function_data = function_statement.get_ast_data();
        const std::string function_name = function_data.at("name");
        const std::string function_return = function_data.at("return_type");

        mir_statements function_parameters = function_statement.get_children({statement_type::parameter});
        mir_statements function_variables = function_statement.get_children({statement_type::variable});
        mir_statements function_debugs = function_statement.get_children({statement_type::debug});
        mir_statements function_blocks = function_statement.get_children({statement_type::block});
        mir_statements function_all_variables = mir_statement::get_all_variables(function_statement, _structs);

        mir_statements function_states;
        std::ranges::copy(function_parameters, std::back_insert_iterator(function_states));
        std::ranges::copy(function_variables, std::back_insert_iterator(function_states));

        generate_structs(&file, function_states, function_name);
        generate_function(&file, function_states, function_debugs, function_name, function_return, function_all_variables, true);
    }

    for (auto function_statement : std::ranges::reverse_view(functions)) {
        nlohmann::json function_data = function_statement.get_ast_data();
        const std::string function_name = function_data.at("name");
        const std::string function_return = function_data.at("return_type");
        
        mir_statements function_parameters = function_statement.get_children({statement_type::parameter});
        mir_statements function_variables = function_statement.get_children({statement_type::variable});
        mir_statements function_debugs = function_statement.get_children({statement_type::debug});
        mir_statements function_blocks = function_statement.get_children({statement_type::block});
        mir_statements function_all_variables = mir_statement::get_all_variables(function_statement, _structs);

        mir_statements function_states;
        std::ranges::copy(function_parameters, std::back_insert_iterator(function_states));
        std::ranges::copy(function_variables, std::back_insert_iterator(function_states));

        reference_map references;

        for (const auto& block_statement : std::ranges::reverse_view(function_blocks)) {
            file << function_name << "_state " << function_name << "_" << block_statement.get_ast_data().at("name").get<std::string>() << "(" << function_name << "_state state);" << std::endl;
        }
        file << std::endl;

        for (const auto& block_statement : std::ranges::reverse_view(function_blocks)) {
            generate_block_function(&file, block_statement, function_name, &references, function_all_variables);
        }

        generate_function(&file, function_states, function_debugs, function_name, function_return, function_all_variables);
    }

    generate_main_function(&file, functions, target_function);

    file.close();
    return out;
}

std::string mir_contract::get_c_type(const std::string &type, const std::string &name, const std::string &function_name) const {
    if (type.starts_with("array<")) {
        return get_c_type(get_c_subtype(type), name + "[" + std::to_string(_globals.ARRAY_SIZE) + "]", function_name);
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
    if (type.starts_with("optional<")) {
        const std::string optional_name = get_optional_name(type, function_name);
        return optional_name + " " + name;
    }
    return type + " " + name;
}

std::string mir_contract::get_return_c_type(const std::string &type, const std::string &function_name) const {
    const std::string c_type = get_c_type(type, "_0", function_name);
    if (!c_type.ends_with(" _0")) {
        std::throw_with_nested(std::runtime_error("Unsupported return type: " + type));
    }

    return c_type.substr(0, c_type.size() - 3);
}

std::string mir_contract::get_c_value(const std::string &value) {
    if (value.starts_with("copy_array<")) {
        return get_c_value(value.substr(11, value.size() - 12));
    }
    if (value.starts_with("copy_tuple<")) {
        return get_c_value(value.substr(11, value.size() - 12));
    }
    if (value.starts_with("copy_pubkey<")) {
        return get_c_value(value.substr(12, value.size() - 13));
    }
    if (value.starts_with("copy_account_info<")) {
        return get_c_value(value.substr(18, value.size() - 19));
    }
    if (value.starts_with("copy_void_result<")) {
        return get_c_value(value.substr(17, value.size() - 18));
    }
    if (value.starts_with("copy_result<")) {
        return get_c_value(value.substr(12, value.size() - 13));
    }
    if (value.starts_with("copy_account_meta<")) {
        return get_c_value(value.substr(18, value.size() - 19));
    }
    if (value.starts_with("copy_solana_instruction<")) {
        return get_c_value(value.substr(24, value.size() - 25));
    }
    return value;
}

std::string mir_contract::get_c_subtype(const std::string &type) {
    if (type.starts_with("array<")) {
        const std::string subtype = type.substr(6, type.size()-7);
        return subtype;
    }
    return type;
}

void mir_contract::export_library_file(const std::filesystem::path &target) const {
    const std::filesystem::path out = target / "solana.c";
    std::fstream file;
    file.open(out, std::ios::out);
    if (!file.is_open()) {
        std::throw_with_nested(std::runtime_error("Unable to write external library"));
    }

    const auto asset_fs = cmrc::SolanaVerifierAssets::get_filesystem();
    const cmrc::file lib_file = asset_fs.open("assets/solana.casset");

    auto asset_file = std::string(lib_file.begin(), lib_file.end());
    asset_file = std::regex_replace(asset_file, std::regex(R"(\{\{ARRAY_SIZE\}\})"), std::to_string(_globals.ARRAY_SIZE));
    file << asset_file << std::endl;
    file.close();
}

std::string mir_contract::trim_line(const std::string &line) {
    std::string trimmed = utils::trim(line);
    const std::regex comment_regex (R"(^(.*?[;{])?(?:\s*\/\/.*)$)");
    if (std::smatch match; std::regex_match(trimmed, match, comment_regex)) {
        return match[1].str();
    }
    return trimmed;
}

void mir_contract::generate_struct_struct(std::ostream *out, const mir_statements &state_statements, const std::string &struct_name) const {
    *out << "typedef struct " << struct_name << "_struct {" << std::endl;
    for (const auto& variable : state_statements) {
        std::string var_name = variable.get_ast_data().at("variable");
        std::string var_type = variable.get_ast_data().at("variable_type");
        *out << "\t" << var_type << " " << var_name << ";" << std::endl;
    }
    *out << "} " << struct_name << ";" << std::endl;

    *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
    *out << "\t" << struct_name << " x;" << std::endl;
    for (const auto& variable : state_statements) {
        std::string var_name = variable.get_ast_data().at("variable");
        std::string var_type = variable.get_ast_data().at("variable_type");
        generate_nondet_from_name(out, "x." + var_name, var_type, struct_name);
    }
    *out << "\treturn x;" << std::endl;
    *out << "}" << std::endl;

    *out << "void serialize_" << struct_name << "(" << struct_name << " x, u8* out) {" << std::endl;
    unsigned int s_counter = 0;
    for (const auto& variable : state_statements) {
        std::string var_name = variable.get_ast_data().at("variable");
        std::string var_type = variable.get_ast_data().at("variable_type");
        generate_serialization(out, var_name, var_type, &s_counter);
    }
    *out << "}" << std::endl;

    *out << struct_name << " deserialize_" << struct_name << "(u8* in) {" << std::endl;
    *out << "\t" << struct_name << " x;" << std::endl;
    unsigned int d_counter = 0;
    for (const auto& variable : state_statements) {
        std::string var_name = variable.get_ast_data().at("variable");
        std::string var_type = variable.get_ast_data().at("variable_type");
        generate_deserialization(out, var_name, var_type, &d_counter);
    }
    *out << "\treturn x;" << std::endl;
    *out << "}" << std::endl;

    *out << std::endl;
}

void mir_contract::generate_structs(std::ostream *out, const mir_statements &state_statements, const std::string &function_name) {
    std::set<std::string> generated_structs;

    for (const auto& parameter_statement: state_statements) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        generate_struct(out, parameter_type, function_name, &generated_structs);
    }

    generate_function_struct(out, state_statements, function_name);
}

void mir_contract::generate_struct(std::ostream *out, const std::string &type, const std::string &function_name, std::set<std::string> *generated_structs) {
    if (generated_structs->contains(type)) {
        return;
    }
    if (type.starts_with("tuple<")) {
        generate_tuple_struct(out, type, function_name, generated_structs);
        generated_structs->insert(type);
    } else if (type.starts_with("result<")) {
        generate_result_struct(out, type, function_name, generated_structs);
        generated_structs->insert(type);
    } else if (type.starts_with("controlflow<")) {
        generate_controlflow_struct(out, type, function_name, generated_structs);
        generated_structs->insert(type);
    } else if (type.starts_with("optional<")) {
        generate_optional_struct(out, type, function_name, generated_structs);
        generated_structs->insert(type);
    }
}

void mir_contract::generate_tuple_struct(std::ostream *out, const std::string &type, const std::string &function_name, std::set<std::string> *generated_structs) {
    const std::string struct_name = get_tuple_name(type, function_name);
    const std::string raw_value_types = type.substr(6, type.size() - 7);
    const std::list<std::string> value_types = utils::split(raw_value_types, ", ");

    for (const auto& value_type: value_types) {
        generate_struct(out, value_type, function_name, generated_structs);
    }

    *out << "typedef struct " << struct_name << "_struct {" << std::endl;
    unsigned int tuple_i = 0;
    for (const auto& value_type: value_types) {
        *out << "\t" << get_c_type(value_type, "get" + std::to_string(tuple_i), function_name) << ";" << std::endl;
        tuple_i++;
    }
    *out << "} " << struct_name << ";" << std::endl;

    *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
    *out << "\t" << struct_name << " t;" << std::endl;
    unsigned int tuple_i2 = 0;
    for (const auto& value_type: value_types) {
        generate_nondet_from_name(out, "t.get" + std::to_string(tuple_i2), value_type, function_name);
        tuple_i2++;
    }
    *out << "\treturn t;" << std::endl;
    *out << "}" << std::endl;

    *out << std::endl;
}

void mir_contract::generate_result_struct(std::ostream *out, const std::string &type, const std::string &function_name, std::set<std::string> *generated_structs) {
    const std::string struct_name = get_result_name(type, function_name);
    const std::string success_type = type.substr(7, type.size() - 8);

    generate_struct(out, success_type, function_name, generated_structs);

    *out << "typedef struct " << struct_name << "_struct {" << std::endl;
    *out << "\tbool is_success;" << std::endl;
    if (success_type != "void") {
        *out << "\t" << get_c_type(success_type, "value", function_name) << ";" << std::endl;
    }
    *out << "} " << struct_name << ";" << std::endl;

    *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
    *out << "\t" << struct_name << " r;" << std::endl;
    generate_nondet_from_name(out, "r.is_success", "bool", function_name);
    if (success_type != "void") {
        generate_nondet_from_name(out, "r.value", success_type, function_name);
    }
    *out << "\treturn r;" << std::endl;
    *out << "}" << std::endl;

    *out << std::endl;
}

void mir_contract::generate_controlflow_struct(std::ostream *out, const std::string &type, const std::string &function_name, std::set<std::string> *generated_structs) {
    const std::string struct_name = get_controlflow_name(type, function_name);
    const std::string raw_controlflow_types = type.substr(12, type.size() - 13);
    const auto [break_value, continue_value] = mir_statement::get_argument_pair(raw_controlflow_types);

    generate_struct(out, break_value, function_name, generated_structs);
    generate_struct(out, continue_value, function_name, generated_structs);

    *out << "typedef struct " << struct_name << "_struct {" << std::endl;
    *out << "\tcontrolflow type;" << std::endl;
    if (break_value != "void") {
        *out << "\t" << get_c_type(break_value, "break_value", function_name) << ";" << std::endl;
    }
    if (continue_value != "void") {
        *out << "\t" << get_c_type(continue_value, "continue_value", function_name) << ";" << std::endl;
    }
    *out << "} " << struct_name << ";" << std::endl;

    *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
    *out << "\t" << struct_name << " c;" << std::endl;
    generate_nondet_from_name(out, "c.type", "controlflow", function_name);
    if (break_value != "void") {
        generate_nondet_from_name(out, "c.break_value", break_value, function_name);
    }
    if (continue_value != "void") {
        generate_nondet_from_name(out, "c.continue_value", continue_value, function_name);
    }
    *out << "\treturn c;" << std::endl;
    *out << "}" << std::endl;

    *out << std::endl;
}

void mir_contract::generate_optional_struct(std::ostream *out, const std::string &type, const std::string &function_name, std::set<std::string> *generated_structs) {
    const std::string struct_name = get_optional_name(type, function_name);
    const std::string optional_type = type.substr(9, type.size() - 10);

    generate_struct(out, optional_type, function_name, generated_structs);

    *out << "typedef struct " << struct_name << "_struct {" << std::endl;
    *out << "\tbool is_none;" << std::endl;
    *out << "\t" << get_c_type(optional_type, "value", function_name) << ";" << std::endl;
    *out << "} " << struct_name << ";" << std::endl;

    *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
    *out << "\t" << struct_name << " x;" << std::endl;
    generate_nondet_from_name(out, "x.is_none", "bool", function_name);
    generate_nondet_from_name(out, "x.value", optional_type, function_name);
    *out << "\treturn x;" << std::endl;
    *out << "}" << std::endl;

    *out << std::endl;
}

void mir_contract::generate_function_struct(std::ostream *out, const mir_statements &state_statements, const std::string &function_name) const {
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

void mir_contract::generate_block_function(std::ostream *out, mir_statement block_statement, const std::string &function_name, reference_map *references, const mir_statements &all_variables) {
    nlohmann::json block_data = block_statement.get_ast_data();
    const auto block_name = block_data.at("name").get<std::string>();
    const mir_statements statements = block_statement.get_children({statement_type::assignment, statement_type::add_ref, statement_type::remove_ref});

    *out << function_name << "_state " << function_name << "_" << block_name << "(" << function_name << "_state state) {" << std::endl;

    // Add assignments
    for (const auto& statement : statements) {
        generate_block_statement(out, statement, function_name, references, all_variables);
    }

    *out << "\treturn state;" << std::endl;
    *out << "}" << std::endl;
    *out << std::endl;
}
 
void mir_contract::generate_block_statement(std::ostream *out, mir_statement statement, const std::string &function_name, reference_map *references, const mir_statements& all_variables) {
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

    generate_block_assignment(out, assignment_variable, assignment_value, assignment_returns, all_variables);

    generate_reference_assignments(out, assignment_variable, *references);

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

void mir_contract::generate_block_assignment(std::ostream *out, const std::string &variable, const std::string &value, bool returns, const mir_statements &all_variables, int indents) {
    auto base_indent = std::string(indents, '\t');

    if (value.starts_with("checked<")) {
        const std::string checked_value = value.substr(8, value.size() - 9);
        *out << base_indent << "state." << variable << ".get0 = " << checked_value << ".value;" << std::endl;
        *out << base_indent << "state." << variable << ".get1 = " << checked_value << ".errors;" << std::endl;
    } else if (value.starts_with("unchecked<")) {
        const std::string checked_value = value.substr(10, value.size() - 11);
        *out << base_indent << "state." << variable << " = " << checked_value << ".value;" << std::endl;
    } else if (value.starts_with("serialize_")) {
        *out << base_indent << "state." << variable << ".is_success = true;" << std::endl;
        *out << base_indent << value << ";" << std::endl;
    } else if (value.starts_with("deserialize_")) {
        *out << base_indent << "state." << variable << ".is_success = true;" << std::endl;
        *out << base_indent << "state." << variable << ".value = " << value << ";" << std::endl;
    } else if (value.starts_with("ok<")) {
        *out << base_indent << "state." << variable << ".is_success = true;" << std::endl;
        const std::string ok_value = value.substr(3, value.size() - 4);
        if (ok_value != "void") {
            generate_block_assignment(out, variable + ".value", ok_value, true, all_variables, indents);
        }
    } else if (value.starts_with("result_error<")) {
        *out << base_indent << "state." << variable << ".is_success = false;" << std::endl;
    } else if (value.starts_with("try_void_branch<")) {
        *out << base_indent << "state." << variable << ".type = _continue;" << std::endl;
    } else if (value.starts_with("try_branch<")) {
        const std::string try_value = value.substr(11, value.size() - 12);
        *out << base_indent << "state." << variable << ".type = _continue;" << std::endl;
        generate_block_assignment(out, variable + ".continue_value", try_value, true, all_variables, indents);
    } else if (value.starts_with("from_residual<")) {
        *out << base_indent << "state." << variable << ".is_success = false;" << std::endl;
    } else if (value.starts_with("next<")) {
        const std::string iter_value = value.substr(5, value.size() - 6);
        *out << base_indent << "state." << variable << ".is_success = true;" << std::endl;
        generate_block_assignment(out, variable + ".value", get_c_value(iter_value) + "[0]", true, all_variables, indents);
        for (int i = 0; i < _globals.ARRAY_SIZE-1; i++) {
            generate_block_assignment(out, get_c_value(iter_value).substr(6) + "[" + std::to_string(i) + "]", get_c_value(iter_value) + "[" + std::to_string(i+1) + "]", true, all_variables, indents);
        }
    } else if (value.starts_with("copy_array<")) {
        const std::string array_value = value.substr(11, value.size() - 12);
        for (int i = 0; i < _globals.ARRAY_SIZE; i++) {
            generate_block_assignment(out, variable + "[" + std::to_string(i) + "]", array_value + "[" + std::to_string(i) + "]", true, all_variables, indents);
        }
    } else if (value.starts_with("copy_tuple<")) {
        const std::string tuple_value = value.substr(11, value.size() - 12);
        for (int i = 0; i < _globals.ARRAY_SIZE; i++) {
            generate_block_assignment(out, variable + ".get" + std::to_string(i), tuple_value + ".get" + std::to_string(i), true, all_variables, indents);
        }
    } else if (value.starts_with("copy_pubkey<")) {
        const std::string pubkey_value = value.substr(12, value.size() - 13);
        for (int i = 0; i < 32; i++) {
            *out << base_indent << "state." << variable << ".p" << i << " = " << pubkey_value << ".p" << i << ";" << std::endl;
        }
    } else if (value.starts_with("copy_account_info<")) {
        const std::string info_value = value.substr(18, value.size() - 19);
        for (int i = 0; i < 8; i++) {
            if (i == 0 || i == 3) {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), "copy_pubkey<" + info_value + ".get" + std::to_string(i) + ">", true, all_variables, indents);
            } else if (i == 2) {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), "copy_array<" + info_value + ".get" + std::to_string(i) + ">", true, all_variables, indents);
            } else {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), info_value + ".get" + std::to_string(i), true, all_variables, indents);
            }
        }
    } else if (value.starts_with("copy_account_meta<")) {
        const std::string meta_value = value.substr(18, value.size() - 19);
        for (int i = 0; i < 3; i++) {
            if (i == 0) {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), "copy_pubkey<" + meta_value + ".get" + std::to_string(i) + ">", true, all_variables, indents);
            } else {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), meta_value + ".get" + std::to_string(i), true, all_variables, indents);
            }
        }
    } else if (value.starts_with("copy_void_result<")) {
        const std::string result_value = value.substr(17, value.size() - 18);
        if (result_value.starts_with("state.") || _function_names.contains(utils::split(result_value, "(").front())) {
            generate_block_assignment(out, variable + ".is_success", result_value + ".is_success", true, all_variables, indents);
        } else {
            generate_block_assignment(out, variable, result_value, true, all_variables, indents);
        }
    } else if (value.starts_with("copy_result<")) {
        const std::string result_value = value.substr(12, value.size() - 13);
        if (result_value.starts_with("state.") || _function_names.contains(utils::split(result_value, "(").front())) {
            generate_block_assignment(out, variable + ".is_success", result_value + ".is_success", true, all_variables, indents);
            generate_block_assignment(out, variable + ".value", result_value + ".value", true, all_variables, indents);
        } else {
            generate_block_assignment(out, variable, result_value, true, all_variables, indents);
        }
    } else if (value.starts_with("copy_solana_instruction<")) {
        const std::string instruction_value = value.substr(24, value.size() - 25);
        for (int i = 0; i < 3; i++) {
            if (i == 0) {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), "copy_pubkey<" + instruction_value + ".get" + std::to_string(i) + ">", true, all_variables, indents);
            } else {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), "copy_array<" + instruction_value + ".get" + std::to_string(i) + ">", true, all_variables, indents);
            }
        }
    } else if (value.starts_with("init_solana_instruction<")) {
        const std::string instruction_value = value.substr(24, value.size() - 25);
        const auto instruction_values = utils::split(instruction_value, ", ");
        auto instruction_values_itr = instruction_values.begin();

        generate_block_assignment(out, variable + ".get0", "copy_pubkey<" + *instruction_values_itr + ">", true, all_variables, indents);
        ++instruction_values_itr;
        generate_block_assignment(out, variable + ".get1", "copy_array<" + *instruction_values_itr + ">", true, all_variables, indents);
        ++instruction_values_itr;
        generate_block_assignment(out, variable + ".get2", "copy_array<" + *instruction_values_itr + ">", true, all_variables, indents);
    } else if (value.starts_with("init_array<")) {
        const std::string array_value = value.substr(11, value.size() - 12);
        const auto array_values = utils::split(array_value, ", ");
        auto array_values_itr = array_values.begin();
        for (int i = 0; i < array_values.size(); i++) {
            generate_block_assignment(out, variable + "[" + std::to_string(i) + "]", *array_values_itr, true, all_variables, indents);
            ++array_values_itr;
        }
    } else if (value.starts_with("init_tuple<")) {
        const std::string tuple_value = value.substr(11, value.size() - 12);
        const auto tuple_values = utils::split(tuple_value, ", ");
        auto tuple_values_itr = tuple_values.begin();
        for (int i = 0; i < tuple_values.size(); i++) {
            generate_block_assignment(out, variable + ".get" + std::to_string(i), *tuple_values_itr, true, all_variables, indents);
            ++tuple_values_itr;
        }
    } else if (!variable.empty()) {
        const std::optional<mir_statement> statement = mir_statement::get_statement(all_variables, value.starts_with("state.") ? value.substr(6) : value);
        std::string formatted_value = value;
        if (statement.has_value()) {
            std::string type = statement.value().get_ast_data().at("variable_type");
            formatted_value = mir_statement::reformat_value_by_type(value, type);
        }

        if (formatted_value != value) {
            generate_block_assignment(out, variable, formatted_value, returns, all_variables, indents);
            return;
        }

        if (returns) {
            *out << base_indent << "state." << variable << " = " << formatted_value << ";" << std::endl;
        } else {
            *out << base_indent << formatted_value << ";" << std::endl;
        }
    } else {
        if (!returns) {
            *out << base_indent << value << ";" << std::endl;
        }
    }
}

void mir_contract::generate_reference_assignments(std::ostream *out, const std::string &variable, const reference_map &references) {
    std::set<std::string> referenced_variables;
    std::set<std::string> queue;

    if (!references.contains(variable)) {
        return;
    }

    for (const auto& ref: references.at(variable)) {
        queue.insert(ref);
    }

    while (!queue.empty()) {
        auto ref = *queue.begin();
        queue.erase(queue.begin());

        if (!referenced_variables.contains(ref) && ref != variable) {
            referenced_variables.insert(ref);
            for (const auto& r: references.at(ref)) {
                queue.insert(r);
            }
        }
    }

    for (const auto& ref: referenced_variables) {
        *out << "\t" << ref << " = state." << variable << ";" << std::endl;
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

void mir_contract::generate_function(std::ostream *out, const mir_statements &state_statements, const mir_statements &debug_statements, const std::string &
                                     function_name, const std::string &function_return, const mir_statements &all_variables, bool forward_decl) {
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
    const std::string return_type = get_return_c_type(function_return, function_name);
    *out << return_type << " " << function_name << "(" << utils::join(parameters, ", ") << ")";

    if (forward_decl) {
        *out << ";" << std::endl;
        return;
    }
    *out << " {" << std::endl;

    // Init function state
    *out << "\t" << function_name << "_state state;" << std::endl;
    for (const auto& statement: state_statements) {
        if (statement.get_ast_data()["variable_type"].get<std::string>() != "void") {
            generate_nondet_from_statement(out, statement, function_name, all_variables);
        }
    }
    *out << std::endl;

    // Call first block
    *out << "\tstate = " << function_name << "_bb0(state);" << std::endl;
    *out << std::endl;

    // Add solana verification statements
    generate_verification_statements(out, state_statements, debug_statements, function_return);

    // Return value
    *out << "\treturn state._0;" << std::endl;
    *out << "}" << std::endl;
    *out << std::endl;
}

void mir_contract::generate_nondet_from_name(std::ostream *out, const std::string &name, const std::string &type, const std::string &function_name) const {
    if (type.starts_with("array<")) {
        generate_nondet_array(out, name, type);
    } else {
        *out << "\t" << name << " = nondet_" << get_return_c_type(type, function_name) << "();" << std::endl;
    }
}

void mir_contract::generate_nondet_from_statement(std::ostream *out, const mir_statement &statement, const std::string &function_name, const mir_statements &all_variables, bool in_main) {
    nlohmann::json parameter_data = statement.get_ast_data();
    const std::string parameter_name = parameter_data.at("variable").get<std::string>();
    const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();

    std::string nondet_name;
    if (in_main) {
        nondet_name = parameter_name;
    } else {
        nondet_name = "state." + parameter_name;
    }

    if (!in_main && statement.get_type() == statement_type::parameter) {
        generate_block_assignment(out, parameter_name, mir_statement::reformat_value_by_type(parameter_name, parameter_type), true, all_variables);
    } else {
        generate_nondet_from_name(out, nondet_name, parameter_type, function_name);
    }
}

void mir_contract::generate_nondet_array(std::ostream *out, const std::string &name, const std::string &type) const {
    for (int i = 0; i < _globals.ARRAY_SIZE-1; i++) {
        *out << "\t" << name << "[" << i << "] = nondet_" << get_c_subtype(type) << "();" << std::endl;
    }
}

void mir_contract::generate_serialization(std::ostream *out, const std::string &variable_name, const std::string& variable_type, unsigned int *counter) {
    const std::regex uint_regex (R"(^u(\d+)$)");
    if (std::smatch match; std::regex_match(variable_type, match, uint_regex)) {
        const int bits = std::stoi(match[1].str());
        for (int i = 0; i < bits; i+=8) {
            *out << "\tout[" << *counter << "] = x." << variable_name << " >> " << i << " & 0xFF;" << std::endl;
            (*counter)++;
        }
    }
}

void mir_contract::generate_deserialization(std::ostream *out, const std::string &variable_name, const std::string &variable_type, unsigned int *counter) {
    const std::regex uint_regex (R"(^u(\d+)$)");
    if (std::smatch match; std::regex_match(variable_type, match, uint_regex)) {
        const int bits = std::stoi(match[1].str());
        *out << "\tx." << variable_name << " = 0;" << std::endl;
        for (int i = 0; i < bits; i+=8) {
            *out << "\tx." << variable_name << " = " << "x." << variable_name << " | in[" << *counter << "] << " << i << ";" << std::endl;
            (*counter)++;
        }
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
    mir_statements target_function_all_variables = mir_statement::get_all_variables(target_function_statement, _structs);

    *out << "int main() {" << std::endl;
    for (const auto& parameter_statement: target_function_parameters) {
        nlohmann::json parameter_data = parameter_statement.get_ast_data();
        const std::string parameter_name = parameter_data.at("variable").get<std::string>();
        const std::string parameter_type = parameter_data.at("variable_type").get<std::string>();
        *out << "\t" << get_c_type(parameter_type, parameter_name, target_function_name) << ";" << std::endl;
    }
    *out << std::endl;

    for (const auto& parameter_statement: target_function_parameters) {
        generate_nondet_from_statement(out, parameter_statement, target_function_name, target_function_all_variables, true);
    }
    *out << std::endl;


    *out << "\t" << get_return_c_type(target_function_type, target_function_name) << " result = " << target_function_name << "(";

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

void mir_contract::generate_verification_statements(std::ostream *out, const mir_statements& state_statements, const mir_statements& debug_statements, const std::string& function_return) {
    std::string program_id_name;
    std::set<std::tuple<std::string, std::string>> owner_account_infos;
    std::set<std::tuple<std::string, std::string>> signer_account_infos;

    for (const auto& debug_statement: debug_statements) {
        std::string debug_name = debug_statement.get_ast_data().at("name");
        std::string debug_variable = debug_statement.get_ast_data().at("variable");
        std::optional<mir_statement> variable_statement = mir_statement::get_statement(state_statements, debug_variable);
        if (!variable_statement.has_value()) {
            continue;
        }

        std::string variable_type = variable_statement.value().get_ast_data().at("variable_type");
        if (variable_type == "account_info") {
            if (debug_name.find("__owner") != std::string::npos) {
                owner_account_infos.insert(std::make_tuple(debug_variable, debug_name));
            }
            if (debug_name.find("__signer") != std::string::npos) {
                signer_account_infos.insert(std::make_tuple(debug_variable, debug_name));
            }
        } else if (variable_type == "pubkey" && debug_variable == "_1") {
            program_id_name = debug_name;
        }
    }

    if (function_return.starts_with("result<")) {
        // Check owner is calling function
        // LOGIC: A successful return implies that the owner called the function
        for (auto account_info: owner_account_infos) {
            std::string reason = "The variable '" + std::get<1>(account_info) + "' is missing ownership checks";
            std::string solution = "Check '" + std::get<1>(account_info) + ".owner == " + program_id_name + "'";
            *out << "\t__ESBMC_assert(!state._0.is_success || is_equal_pubkey(state._1, state." << std::get<0>(account_info) << ".get3), \"Vulnerability Found: 9; Reason: " << reason << "; Solution: " << solution << "\");" << std::endl;
        }

        // Check signers have signed the instruction
        // LOGIC: A successul return implies that the signer has signed the instruction
        for (auto account_info : signer_account_infos) {
            std::string reason = "The variable '" + std::get<1>(account_info) + "' is missing signer checks";
            std::string solution = "Check '" + std::get<1>(account_info) + ".is_signer'";
            *out << "\t__ESBMC_assert(!state._0.is_success || state." << std::get<0>(account_info) << ".get5, \"Vulnerability Found: 10; Reason: " << reason << "; Solution: " << solution << "\");" << std::endl;
        }
        *out << std::endl;
    }
}

std::string mir_contract::get_tuple_name(const std::string &type, const std::string &function_name) {
    const std::string raw_types = type.substr(6, type.size() - 7);
    std::list<std::string> types_list = utils::split(raw_types, ", ");
    for (auto & raw_type : types_list) {
        raw_type = utils::clean(raw_type);
    }
    const std::string clean_types = utils::join(types_list, "_");
    return function_name + "_" + clean_types + "_tuple";
}

std::string mir_contract::get_result_name(const std::string &type, const std::string &function_name) {
    const std::string raw_type = type.substr(7, type.size() - 8);
    return function_name + "_" + utils::clean(raw_type) + "_result";
}

std::string mir_contract::get_controlflow_name(const std::string &type, const std::string &function_name) {
    const std::string raw_types = type.substr(12, type.size() - 13);
    const auto [break_type, continue_type] = mir_statement::get_argument_pair(raw_types);
    return function_name + "_" + utils::clean(break_type) + "_" + utils::clean(continue_type) + "_controlflow";
}

std::string mir_contract::get_optional_name(const std::string &type, const std::string &function_name) {
    const std::string raw_type = type.substr(9, type.size() - 10);
    return function_name + "_" + utils::clean(raw_type) + "_optional";
}

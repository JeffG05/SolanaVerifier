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
    mir_statement ast_tree = create_ast_tree(file, target_funtion);
    file.close();
    // ast_tree.print();

    // Export AST tree as C program
    const std::filesystem::path c_file_path = export_c_program(target, ast_tree, target_funtion);

    // Export library file
    export_library_file(target);

    return c_program(_contract_name, c_file_path);

}

mir_statement mir_contract::create_ast_tree(std::istream &file, const std::string& target) {
    // Init statement tree
    auto root = mir_statement::create_root(_contract_name);

    for (const auto& struct_statement : _structs) {
        root.add_child(struct_statement);
    }

    std::string line;
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (mir_statement::line_is_function(trimmed_line)) {
            std::optional<mir_statement> func = mir_statement::parse_function_header(trimmed_line);
            if (func.has_value()) {
                _function_names.insert(func.value().get_ast_data().at("name"));
            }
        }
    }

    file.clear();
    file.seekg(0);

    auto current_statement_lines = std::list<std::string>();
    while (getline(file, line)) {
        std::string trimmed_line = trim_line(line);
        if (mir_statement::line_is_function(trimmed_line)) {
            if (!current_statement_lines.empty() && mir_statement::line_is_function(current_statement_lines.front())) {
                std::optional<mir_statement> statement = mir_statement::parse_function(current_statement_lines, _structs);
                if (statement.has_value() && !statement.value().get_ast_data().contains("source") && statement.value().get_ast_data().at("name") != "entrypoint") {
                    for (auto block : statement.value().get_children({statement_type::block})) {
                        for (const auto& assignment : block.get_children({statement_type::assignment})) {
                            std::string value = assignment.get_ast_data().at("value");
                            if (value.find("checked<") != std::string::npos) {
                                std::string maths_function = value.substr(value.find("checked<") + 8, value.rfind(')') - value.find("checked<") - 7);
                                mir_statement maths_statement = mir_statement::parse_maths(maths_function);
                                bool already_exists = false;
                                for (const auto& existing_maths : statement->get_children({statement_type::maths})) {
                                    if (existing_maths.get_ast_data().at("value").get<std::string>() == maths_statement.get_ast_data().at("value").get<std::string>()) {
                                        already_exists = true;
                                        break;
                                    }
                                }
                                if (!already_exists) {
                                    statement->add_child(maths_statement);
                                }
                            }
                        }
                    }
                    root.add_child(statement.value());
                }
            }
            current_statement_lines.clear();
        }

        current_statement_lines.push_back(trimmed_line);
    }
    if (!current_statement_lines.empty() && mir_statement::line_is_function(current_statement_lines.front())) {
        std::optional<mir_statement> statement = mir_statement::parse_function(current_statement_lines, _structs);
        if (statement.has_value() && !statement.value().get_ast_data().contains("source") && statement.value().get_ast_data().at("name") != "entrypoint") {
            for (auto block : statement.value().get_children({statement_type::block})) {
                for (const auto& assignment : block.get_children({statement_type::assignment})) {
                    std::string value = assignment.get_ast_data().at("value");
                    if (value.find("checked<") != std::string::npos) {
                        std::string maths_function = value.substr(value.find("checked<") + 8, value.rfind(')') - value.find("checked<") - 7);
                        mir_statement maths_statement = mir_statement::parse_maths(maths_function);
                        bool already_exists = false;
                        for (const auto& existing_maths : statement->get_children({statement_type::maths})) {
                            if (existing_maths.get_ast_data().at("value").get<std::string>() == maths_statement.get_ast_data().at("value").get<std::string>()) {
                                already_exists = true;
                                break;
                            }
                        }
                        if (!already_exists) {
                            statement->add_child(maths_statement);
                        }
                    }
                }
            }
            root.add_child(statement.value());
        }
    }
    current_statement_lines.clear();

    return trim_ast_tree(root, target);
}

mir_statement mir_contract::trim_ast_tree(mir_statement tree, const std::string& target) const {
    std::set<std::string> _remove_functions = _function_names;
    _remove_functions.erase(target);

    for (auto function_statement : tree.get_children({statement_type::function})) {
        for (auto block_statement : function_statement.get_children({statement_type::block})) {
            for (const auto& assignment_statement : block_statement.get_children({statement_type::assignment})) {
                std::string value = assignment_statement.get_ast_data().at("value");
                for (const auto& function_name : _remove_functions) {
                    std::regex function_call_regex (R"(^(?:.*<|.*\s)?)" + function_name + R"(\(.+$)");
                    if (std::smatch match; std::regex_match(value, match, function_call_regex)) {
                        _remove_functions.erase(function_name);
                        break;
                    }
                }
            }
        }
    }

    mir_statement new_root = mir_statement::extract_node(tree);
    for (const auto& statement : tree.get_children()) {
        if (statement.get_type() == statement_type::function) {
            if (_remove_functions.contains(statement.get_ast_data().at("name"))) {
                continue;
            }
            new_root.add_child(statement);
        } else {
            new_root.add_child(statement);
        }
    }

    return new_root;
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

    // Add const definitions
    file << "pubkey SYSTEM_PROGRAM_ID;" << std::endl;
    file << "pubkey SYSVAR_RENT_ID;" << std::endl;
    file << std::endl;

    mir_statements structs = ast_tree.get_children({statement_type::data_struct});
    for (auto struct_statement : std::ranges::reverse_view(structs)) {
        generate_struct_struct(&file, struct_statement.get_children(), struct_statement.get_ast_data().at("name"));
    }

    mir_statements enum_structs = ast_tree.get_children({statement_type::data_enum_struct});
    for (auto struct_statement : std::ranges::reverse_view(enum_structs)) {
        generate_enum_struct_struct(&file, struct_statement.get_children(), struct_statement.get_ast_data().at("name"));
    }

    mir_statements enums = ast_tree.get_children({statement_type::data_enum});
    for (auto struct_statement : std::ranges::reverse_view(enums)) {
        generate_enum_struct(&file, struct_statement.get_children({statement_type::variable}), struct_statement.get_ast_data().at("name"));
    }

    mir_statements functions = ast_tree.get_children({statement_type::function});
    for (auto function_statement : std::ranges::reverse_view(functions)) {
        nlohmann::json function_data = function_statement.get_ast_data();
        const std::string function_name = function_data.at("name");
        const std::string function_return = function_data.at("return_type");

        mir_statements function_maths = function_statement.get_children({statement_type::maths});
        mir_statements function_parameters = function_statement.get_children({statement_type::parameter});
        mir_statements function_variables = function_statement.get_children({statement_type::variable});
        mir_statements function_debugs = function_statement.get_children({statement_type::debug});
        mir_statements function_blocks = function_statement.get_children({statement_type::block});
        mir_statements function_all_variables = mir_statement::get_all_variables(function_statement, _structs, functions);

        mir_statements function_states;
        std::ranges::copy(function_parameters, std::back_insert_iterator(function_states));
        std::ranges::copy(function_variables, std::back_insert_iterator(function_states));

        for (const auto& s : function_maths) {
            std::string operator_name = s.get_ast_data().at("value");
            generate_maths_function(&file, operator_name, function_name);
        }
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
        mir_statements function_all_variables = mir_statement::get_all_variables(function_statement, _structs, functions);

        mir_statements function_states;
        std::ranges::copy(function_parameters, std::back_insert_iterator(function_states));
        std::ranges::copy(function_variables, std::back_insert_iterator(function_states));

        for (const auto& block_statement : std::ranges::reverse_view(function_blocks)) {
            file << function_name << "_state " << function_name << "_" << block_statement.get_ast_data().at("name").get<std::string>() << "(" << function_name << "_state state);" << std::endl;
        }
        file << std::endl;

        for (const auto& block_statement : std::ranges::reverse_view(function_blocks)) {
            generate_block_function(&file, block_statement, function_name, function_all_variables);
        }

        generate_function(&file, function_states, function_debugs, function_name, function_return, function_all_variables);
    }

    generate_main_function(&file, functions, target_function, functions);

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
    if (c_type.ends_with(" _0")) {
        return c_type.substr(0, c_type.size() - 3);
    }
    if (c_type.ends_with("]")) {
        return utils::split(c_type, " ").front() + "*";
    }

    std::throw_with_nested(std::runtime_error("Unsupported return type: " + type));
}

std::string mir_contract::get_c_value(const std::string &value) {
    if (value.starts_with("copy_array<")) {
        return get_c_value(value.substr(11, value.size() - 12));
    }
    if (value.starts_with("copy_tuple<")) {
        return get_c_value(value.substr(11, value.size() - 12));
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

void mir_contract::generate_enum_struct_struct(std::ostream *out, const mir_statements &state_statements, const std::string &struct_name) const {
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

    *out << std::endl;
}

void mir_contract::generate_enum_struct(std::ostream *out, const mir_statements &state_statements, const std::string &struct_name) const {
    *out << "typedef struct " << struct_name << "_struct {" << std::endl;
    *out << "\tstring value;" << std::endl;
    for (const auto& variable : state_statements) {
        std::string var_name = variable.get_ast_data().at("variable");
        std::string var_type = variable.get_ast_data().at("variable_type");
        *out << "\t" << var_type << " " << var_name << ";" << std::endl;
    }
    *out << "} " << struct_name << ";" << std::endl;

    *out << struct_name << " nondet_" << struct_name << "() {" << std::endl;
    *out << "\t" << struct_name << " x;" << std::endl;
    generate_nondet_from_name(out, "x.value", "string", struct_name);
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

void mir_contract::generate_block_function(std::ostream *out, mir_statement block_statement, const std::string &function_name, const mir_statements &all_variables) {
    nlohmann::json block_data = block_statement.get_ast_data();
    const auto block_name = block_data.at("name").get<std::string>();
    const mir_statements statements = block_statement.get_children({statement_type::assignment});

    *out << function_name << "_state " << function_name << "_" << block_name << "(" << function_name << "_state state) {" << std::endl;

    // Add assignments
    for (const auto& statement : statements) {
        generate_block_statement(out, statement, function_name, all_variables);
    }

    *out << "\treturn state;" << std::endl;
    *out << "}" << std::endl;
    *out << std::endl;
}
 
void mir_contract::generate_block_statement(std::ostream *out, mir_statement statement, const std::string &function_name, const mir_statements& all_variables) {
    nlohmann::json assignment_data = statement.get_ast_data();

    const std::string assignment_variable = assignment_data.at("variable").get<std::string>();
    const std::string assignment_value = assignment_data.at("value").get<std::string>();
    const bool assignment_returns = assignment_data.at("returns").get<bool>();
    mir_statements assignment_branches = statement.get_children({statement_type::branch});

    generate_block_assignment(out, assignment_variable, assignment_value, assignment_returns, all_variables, function_name);

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

std::tuple<std::string, std::string> mir_contract::parse_indexed(const std::string& identifier, const std::string& indexed_variable) {
    std::string buffer;
    std::string value;
    bool found_identifier = false;
    int identifier_level = 0;
    for (const auto c: indexed_variable) {
        buffer += c;

        if (c == '<') {
            identifier_level += 1;
        } else if (c == '>') {
            identifier_level -= 1;
        }

        if (found_identifier && value.empty()) {
            if (identifier_level == 0) {
                value = buffer.substr(0, buffer.size()-1);
                buffer = "";
            }
        }

        if (!found_identifier && buffer == identifier + "<") {
            found_identifier = true;
            buffer = "";
        }
    }

    return std::make_tuple(value, buffer);
}

void mir_contract::generate_block_assignment(std::ostream *out, const std::string &variable, const std::string &value, bool returns, const mir_statements &all_variables, const std::string &function_name, int indents, bool no_state, bool prevent_modify) {
    auto base_indent = std::string(indents, '\t');
    auto var_prefix = no_state ? "" : "state.";

    if (value.starts_with("ignore<")) {
        return;
    } if (value.starts_with("checked<")) {
        const std::string checked_value = function_name + "_" + value.substr(8, value.size() - 10) + ", true)";
        *out << base_indent << var_prefix << variable << ".get0 = " << checked_value << ".value;" << std::endl;
        *out << base_indent << var_prefix << variable << ".get1 = " << checked_value << ".errors;" << std::endl;
    } else if (value.starts_with("unchecked<")) {
        const std::string checked_value = function_name + "_" + value.substr(10, value.size() - 12) + ", false)";
        *out << base_indent << var_prefix << variable << " = " << checked_value << ".value;" << std::endl;
    } else if (value.starts_with("option_checked<")) {
        const std::string checked_value = function_name + "_" + value.substr(15, value.size() - 17) + ", true)";
        *out << base_indent << var_prefix << variable << ".value = " << checked_value << ".value;" << std::endl;
        *out << base_indent << var_prefix << variable << ".is_none = " << checked_value << ".errors;" << std::endl;
    } else if (value.starts_with("serialize<")) {
        const std::string serialize_value = value.substr(10, value.size() - 11);
        std::list<std::string> serialize_values = utils::split(serialize_value, ", ", 3);
        if (serialize_values.size() == 2) {
            serialize_values.push_back(var_prefix + variable + ".value");
        }

        const std::string func_name = "serialize_" + serialize_values.front();
        serialize_values.pop_front();
        const std::string func_params = utils::join(serialize_values, ", ");
        const std::string func = func_name + "(" + func_params + ")";

        *out << base_indent << var_prefix << variable << ".is_success = true;" << std::endl;
        *out << base_indent << func << ";" << std::endl;
    } else if (value.starts_with("deserialize<")) {
        const std::string deserialize_value = value.substr(12, value.size() - 13);
        std::list<std::string> deserialize_values = utils::split(deserialize_value, ", ", 2);

        const std::string func_name = "deserialize_" + deserialize_values.front();
        deserialize_values.pop_front();
        const std::string func_param = deserialize_values.front();
        const std::string func = func_name + "(" + func_param + ")";

        *out << base_indent << var_prefix << variable << ".is_success = true;" << std::endl;
        *out << base_indent << var_prefix << variable << ".value = " << func << ";" << std::endl;
    } else if (value.starts_with("ok<")) {
        *out << base_indent << var_prefix << variable << ".is_success = true;" << std::endl;
        const std::string ok_value = value.substr(3, value.size() - 4);
        if (ok_value != "void") {
            generate_block_assignment(out, variable + ".value", ok_value, true, all_variables, function_name, indents, no_state);
        }
    } else if (value.starts_with("optional_none<")) {
        *out << base_indent << var_prefix << variable << ".is_none = true;" << std::endl;
    } else if (value.starts_with("result_error<")) {
        *out << base_indent << var_prefix << variable << ".is_success = false;" << std::endl;
    } else if (value.starts_with("result_unwrap<")) {
        auto [result_value, indexer] = parse_indexed("result_unwrap", value);
        *out << base_indent << "if (!" + result_value + ".is_success) {" << std::endl;
        generate_block_assignment(out, variable, "return_error<>", true, all_variables, function_name, indents+1, no_state);
        *out << base_indent << "}" << std::endl;
        *out << base_indent << var_prefix << variable << " = " << result_value << ".value" + indexer + ";" << std::endl;
    } else if (value.starts_with("result_void_unwrap<")) {
        auto [result_value, indexer] = parse_indexed("result_void_unwrap", value);
        *out << base_indent << "if (!" + result_value + ".is_success) {" << std::endl;
        generate_block_assignment(out, variable, "return_error<>", true, all_variables, function_name, indents+1, no_state);
        *out << base_indent << "}" << std::endl;
    } else if (value.starts_with("return_error<")) {
        std::optional<mir_statement> return_statement = mir_statement::get_statement(all_variables, "_0");
        if (return_statement.has_value() && return_statement.value().get_ast_data().at("variable_type").get<std::string>().starts_with("result<")) {
            generate_block_assignment(out, "_0", "result_error<>", true, all_variables, function_name, indents, no_state);
            *out << base_indent << "return state;" << std::endl;
        } else {
            *out << base_indent << "assert(false);" << std::endl;
        }
    } else if (value.starts_with("try_void_branch<")) {
        *out << base_indent << var_prefix << variable << ".type = _continue;" << std::endl;
    } else if (value.starts_with("try_branch<")) {
        const std::string try_value = value.substr(11, value.size() - 12);
        *out << base_indent << var_prefix << variable << ".type = _continue;" << std::endl;
        generate_block_assignment(out, variable + ".continue_value", try_value, true, all_variables, function_name, indents, no_state);
    } else if (value.starts_with("from_residual<")) {
        *out << base_indent << var_prefix << variable << ".is_success = false;" << std::endl;
    } else if (value.starts_with("next<")) {
        const std::string iter_value = value.substr(5, value.size() - 6);
        *out << base_indent << var_prefix << variable << ".is_success = true;" << std::endl;
        generate_block_assignment(out, variable + ".value", get_c_value(iter_value) + "[0]", true, all_variables, function_name, indents, no_state);
        for (int i = 0; i < _globals.ARRAY_SIZE-1; i++) {
            generate_block_assignment(out, get_c_value(iter_value).substr(6) + "[" + std::to_string(i) + "]", get_c_value(iter_value) + "[" + std::to_string(i+1) + "]", true, all_variables, function_name, indents, no_state);
        }
    } else if (value.starts_with("invoke<")) {
        *out << base_indent << var_prefix << variable << ".is_success = true;" << std::endl;
    } else if (value.starts_with("conversion<")) {
        const auto [conversion_value, indexer] = parse_indexed("conversion", value);
        auto conversion_values = utils::split(conversion_value, ", ", 2);

        const std::string conversion_var = conversion_values.front();
        const std::string conversion_type = conversion_values.back();

        const std::optional<mir_statement> original_var_statement = mir_statement::get_statement(all_variables, conversion_var.starts_with(var_prefix) ? conversion_var.substr(6) : conversion_var);
        if (original_var_statement.has_value()) {
            const std::string original_type = original_var_statement.value().get_ast_data().at("variable_type");
            if (conversion_type == original_type) {
                generate_block_assignment(out, variable, conversion_var + indexer, true, all_variables, function_name, indents, no_state);
            } else {
                for (const auto& s : _structs) {
                    if (s.get_ast_data().at("name").get<std::string>() == original_type && s.get_type() == statement_type::data_enum) {
                        generate_block_assignment(out, variable, conversion_var + ".value_" + utils::to_lower(conversion_type) += indexer, true, all_variables, function_name, indents, no_state);
                        return;
                    }
                }
                generate_block_assignment(out, variable, "((" + conversion_type + ") " + conversion_var + ")" + indexer, true, all_variables, function_name, indents, no_state);
            }
        } else {
            generate_block_assignment(out, variable, "((" + conversion_type + ") " + conversion_var + ")" + indexer, true, all_variables, function_name, indents, no_state);
        }
    } else if (value.starts_with("find_program_address<")) {
        const std::string subvalue = value.substr(21, value.size() - 22);
        generate_block_assignment(out, variable + ".get0", "sol_find_program_address(" + subvalue + ").get0", true, all_variables, function_name, indents, no_state);
        generate_block_assignment(out, variable + ".get1", "sol_find_program_address(" + subvalue + ").get1", true, all_variables, function_name, indents, no_state);
    } else if (value.starts_with("create_program_address<")) {
        const std::string subvalue = value.substr(23, value.size() - 24);
        generate_block_assignment(out, variable + ".is_success", "true", true, all_variables, function_name, indents, no_state);
        generate_block_assignment(out, variable + ".value", "sol_create_program_address(" + subvalue + ")", true, all_variables, function_name, indents, no_state);
    } else if (value.starts_with("str_as_bytes<")) {
        const std::string subvalue = value.substr(13, value.size() - 14);
        generate_block_assignment(out, variable, "str_as_bytes(" + subvalue + ", " + var_prefix + variable + ")", false, all_variables, function_name, indents, no_state, true);
    } else if (value.starts_with("pubkey_as_bytes<")) {
        const std::string subvalue = value.substr(16, value.size() - 17);
        generate_block_assignment(out, variable, "pubkey_as_bytes(" + subvalue + ", " + var_prefix + variable + ")", false, all_variables, function_name, indents, no_state, true);
    }  else if (value.starts_with("enum_discriminant<")) {
        const std::string subvalue = value.substr(18, value.size() - 19);
        std::optional<mir_statement> enum_statement = mir_statement::get_statement(all_variables, subvalue.starts_with("state.") ? subvalue.substr(6) : subvalue);
        if (enum_statement.has_value()) {
            for (auto s : _structs) {
                if (s.get_type() != statement_type::data_enum || s.get_ast_data().at("name").get<std::string>() != enum_statement.value().get_ast_data().at("variable_type").get<std::string>()) {
                    continue;
                }
                mir_statements options = s.get_children({statement_type::data_enum_option});
                std::string option_if;
                while (options.size() > 1) {
                    option_if += "is_equal_string(" + subvalue + ".value, \"" + options.front().get_ast_data().at("name").get<std::string>() + "\") ? " + std::to_string(options.front().get_ast_data().at("value").get<int>()) + " : ";
                    options.pop_front();
                }
                option_if += std::to_string(options.front().get_ast_data().at("value").get<int>());
                generate_block_assignment(out, variable, option_if, true, all_variables, function_name, indents, no_state);
                break;
            }
        }
    } else if (value.starts_with("any<")) {
        const std::string subvalue = value.substr(4, value.size() - 5);
        std::list<std::string> any_values = utils::split(subvalue, ", ", 2);

        std::string arr = any_values.front();
        std::string test_function_name = any_values.back().substr(0, any_values.back().size()-2);
        std::string indexer = "i_" + utils::clean(variable);
        generate_block_assignment(out, variable, "false", true, all_variables, function_name, indents, no_state);
        *out << base_indent << "for (int " + indexer + " = 0; " + indexer + " < " + std::to_string(_globals.ARRAY_SIZE) + "; " + indexer + "++) {" << std::endl;
        *out << base_indent << "\tif (" << test_function_name << "(" + arr + "[" + indexer + "])) {" << std::endl;
        generate_block_assignment(out, variable, "true", true, all_variables, function_name, indents+2, no_state);
        *out << base_indent << "\t\tbreak;" << std::endl;
        *out << base_indent << "\t}" << std::endl;
        *out << base_indent << "}" << std::endl;
    } else if (value.starts_with("len<")) {
        generate_block_assignment(out, variable, std::to_string(_globals.ARRAY_SIZE), true, all_variables, function_name, indents, no_state);
    } else if (value.starts_with("const_func<")) {
        const std::string subvalue = value.substr(11, value.size() - 14);
        std::optional<mir_statement> function_return = mir_statement::get_statement(all_variables, variable);
        std::string return_type;
        if (function_return.has_value()) {
            return_type = function_return.value().get_ast_data().at("variable_type");
        }
        if (return_type.starts_with("array<")) {
            generate_block_assignment(out, variable, subvalue + "(*" + var_prefix + variable + ")", false, all_variables, function_name, indents, no_state, true);
        } else {
            generate_block_assignment(out, variable, subvalue + "()", true, all_variables, function_name, indents, no_state);
        }
    } else if (value.starts_with("copy_array<")) {
        const std::string array_value = value.substr(11, value.size() - 12);
        for (int i = 0; i < _globals.ARRAY_SIZE; i++) {
            generate_block_assignment(out, variable + "[" + std::to_string(i) + "]", array_value + "[" + std::to_string(i) + "]", true, all_variables, function_name, indents, no_state);
        }
    } else if (value.starts_with("copy_tuple<")) {
        const std::string tuple_value = value.substr(11, value.size() - 12);
        for (int i = 0; i < _globals.ARRAY_SIZE; i++) {
            bool found = false;
            for (const auto& v : all_variables) {
                if (v.get_ast_data().at("variable") == variable + ".get" + std::to_string(i)) {
                    found = true;
                    generate_block_assignment(out, variable + ".get" + std::to_string(i), tuple_value + ".get" + std::to_string(i), true, all_variables, function_name, indents, no_state);
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
    } else if (value.starts_with("copy_account_info<")) {
        const std::string info_value = value.substr(18, value.size() - 19);
        for (int i = 0; i < 8; i++) {
            if (i == 2) {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), "copy_array<" + info_value + ".get" + std::to_string(i) + ">", true, all_variables, function_name, indents, no_state);
            } else {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), info_value + ".get" + std::to_string(i), true, all_variables, function_name, indents, no_state);
            }
        }
        generate_block_assignment(out, variable + ".data_is_empty", info_value + ".data_is_empty", true, all_variables, function_name, indents, no_state);
    } else if (value.starts_with("copy_account_meta<")) {
        const std::string meta_value = value.substr(18, value.size() - 19);
        for (int i = 0; i < 3; i++) {
            generate_block_assignment(out, variable + ".get" + std::to_string(i), meta_value + ".get" + std::to_string(i), true, all_variables, function_name, indents, no_state);
        }
    } else if (value.starts_with("copy_void_result<")) {
        const std::string result_value = value.substr(17, value.size() - 18);
        if (result_value.starts_with("state.") || _function_names.contains(utils::split(result_value, "(", 2).front())) {
            generate_block_assignment(out, variable + ".is_success", result_value + ".is_success", true, all_variables, function_name, indents, no_state);
        } else {
            generate_block_assignment(out, variable, result_value, true, all_variables, function_name, indents, no_state, true);
        }
    } else if (value.starts_with("copy_result<")) {
        const std::string result_value = value.substr(12, value.size() - 13);
        if (result_value.starts_with("state.") || _function_names.contains(utils::split(result_value, "(", 2).front())) {
            generate_block_assignment(out, variable + ".is_success", result_value + ".is_success", true, all_variables, function_name, indents, no_state);
            generate_block_assignment(out, variable + ".value", result_value + ".value", true, all_variables, function_name, indents, no_state);
        } else {
            generate_block_assignment(out, variable, result_value, true, all_variables, function_name, indents, no_state, true);
        }
    } else if (value.starts_with("copy_solana_instruction<")) {
        const std::string instruction_value = value.substr(24, value.size() - 25);
        for (int i = 0; i < 3; i++) {
            if (i == 0) {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), instruction_value + ".get" + std::to_string(i), true, all_variables, function_name, indents, no_state);
            } else {
                generate_block_assignment(out, variable + ".get" + std::to_string(i), "copy_array<" + instruction_value + ".get" + std::to_string(i) + ">", true, all_variables, function_name, indents, no_state);
            }
        }
    } else if (value.starts_with("init_solana_instruction<")) {
        const std::string instruction_value = value.substr(24, value.size() - 25);
        const auto instruction_values = utils::split(instruction_value, ", ", 3);
        auto instruction_values_itr = instruction_values.begin();

        generate_block_assignment(out, variable + ".get0", *instruction_values_itr, true, all_variables, function_name, indents, no_state);
        ++instruction_values_itr;
        generate_block_assignment(out, variable + ".get1", "copy_array<" + *instruction_values_itr + ">", true, all_variables, function_name, indents, no_state);
        ++instruction_values_itr;
        generate_block_assignment(out, variable + ".get2", "copy_array<" + *instruction_values_itr + ">", true, all_variables, function_name, indents, no_state);
    } else if (value.starts_with("init_array<")) {
        const std::string array_value = value.substr(11, value.size() - 12);
        const auto array_values = utils::split(array_value, ", ");
        auto array_values_itr = array_values.begin();
        for (int i = 0; i < array_values.size(); i++) {
            generate_block_assignment(out, variable + "[" + std::to_string(i) + "]", *array_values_itr, true, all_variables, function_name, indents, no_state);
            ++array_values_itr;
        }
    } else if (value.starts_with("init_tuple<")) {
        const std::string tuple_value = value.substr(11, value.size() - 12);
        const auto tuple_values = utils::split(tuple_value, ", ");
        auto tuple_values_itr = tuple_values.begin();
        for (int i = 0; i < tuple_values.size(); i++) {
            generate_block_assignment(out, variable + ".get" + std::to_string(i), *tuple_values_itr, true, all_variables, function_name, indents, no_state);
            ++tuple_values_itr;
        }
    } else if (value.starts_with("init_enum<")) {
        const std::string enum_value = value.substr(10, value.size() - 11);
        auto enum_values = utils::split(enum_value, ", ");
        const std::string enum_value_name = enum_values.front();
        generate_block_assignment(out, variable + ".value", "\"" + enum_value_name + "\"", true, all_variables, function_name, indents, no_state);
        enum_values.pop_front();

        const std::optional<mir_statement> value_statement = mir_statement::get_statement(all_variables, variable + ".value_" + enum_value_name);
        if (value_statement.has_value()) {
            int i = 0;
            while (!enum_values.empty()) {
                std::string enum_value_variable = variable + ".value_" += enum_value_name + ".get" + std::to_string(i);
                generate_block_assignment(out, enum_value_variable, enum_values.front(), true, all_variables, function_name, indents, no_state);
                ++i;
                enum_values.pop_front();
            }
        }
    } else if (value.starts_with("init_account_meta<")) {
        const std::string meta_value = value.substr(18, value.size() - 19);
        const auto meta_values = utils::split(meta_value, ", ", 3);
        auto meta_values_itr = meta_values.begin();
        for (int i = 0; i < meta_values.size(); i++) {
            generate_block_assignment(out, variable + ".get" + std::to_string(i), *meta_values_itr, true, all_variables, function_name, indents, no_state);
            ++meta_values_itr;
        }
    } else if (std::smatch m; std::regex_match(value, m, std::regex(R"((.*?)(const_func<.+>)(.*))"))) {
        std::string const_func_name = m[2].str().substr(11, m[2].str().size()-14);
        std::optional<mir_statement> function_return = mir_statement::get_statement(all_variables, const_func_name);
        std::string return_type;
        if (function_return.has_value()) {
            return_type = function_return.value().get_ast_data().at("variable_type");
        }
        *out << base_indent << get_c_type(return_type, variable + "_temp_" + const_func_name, function_name) << ";" << std::endl;
        mir_statements new_all_variables = all_variables;
        new_all_variables.push_back(mir_statement::new_variable(variable + "_temp_" + const_func_name, return_type));
        generate_block_assignment(out, variable + "_temp_" + const_func_name, m[2].str(), true, new_all_variables, function_name, indents,true);
        generate_block_assignment(out, variable, m[1].str() + variable + "_temp_" + const_func_name + m[3].str(), returns, new_all_variables, function_name, indents, no_state);
    } else if (!variable.empty()) {
        std::string formatted_value = value;
        if (!prevent_modify) {
            std::optional<mir_statement> statement = mir_statement::get_statement(all_variables, variable);
            if (statement.has_value()) {
                formatted_value = mir_statement::reformat_value_by_type(value, statement.value().get_ast_data().at("variable_type"));
            }

            if (formatted_value != value) {
                generate_block_assignment(out, variable, formatted_value, returns, all_variables, function_name, indents, no_state);
                return;
            }
        }

        if (returns) {
            *out << base_indent << var_prefix << variable << " = " << formatted_value << ";" << std::endl;
        } else {
            *out << base_indent << formatted_value << ";" << std::endl;
        }
    } else {
        if (!returns) {
            *out << base_indent << value << ";" << std::endl;
        }
    }
}

void mir_contract::generate_branch(std::ostream *out, const mir_statement &branch_statement, const std::string &function_name, const std::string &variable, const std::string &value) {
    nlohmann::json branch_data = branch_statement.get_ast_data();
    const std::string branch_condition = branch_data.at("condition").get<std::string>();
    const std::string branch_destination = branch_data.at("destination").get<std::string>();
    const bool branch_ignore_var = branch_data.at("ignore_var").get<bool>();
    if (branch_ignore_var) {
        *out << "\tif (" << branch_condition << ") {" << std::endl;
    } else if (variable.empty()) {
        *out << "\tif (" << value << " == " << branch_condition << ") {" << std::endl;
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
    if (return_type.ends_with('*')) {
        if (parameters.empty()) {
            *out << "void " << function_name << "(" << return_type << " out)";
        } else {
            *out << "void " << function_name << "(" << return_type << " out, " << utils::join(parameters, ", ") << ")";
        }
    } else {
        *out << return_type << " " << function_name << "(" << utils::join(parameters, ", ") << ")";
    }

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
    if (return_type.ends_with('*')) {
        int depth = 0;
        std::string type = function_return;
        while (type.starts_with("array<")) {
            depth += 1;
            type = type.substr(6);
        }

        int indexes[depth];
        for (int d = 0; d < depth; d++) {
            indexes[d] = 0;
        }
        for (int i = 0; i < pow(_globals.ARRAY_SIZE, depth); i++) {
            *out << "\tout[" << std::to_string(i) << "] = state._0";
            for (int d = depth-1; d >= 0; d--) {
                *out << "[" << indexes[d] << "]";
            }
            *out << ";" << std::endl;

            int current_depth = 0;
            indexes[current_depth] += 1;
            while(current_depth < depth-1 && indexes[current_depth] >= _globals.ARRAY_SIZE) {
                indexes[current_depth] = 0;
                current_depth += 1;
                indexes[current_depth] += 1;
            }
        }
    } else {
        *out << "\treturn state._0;" << std::endl;
    }
    *out << "}" << std::endl;
    *out << std::endl;
}

void mir_contract::generate_nondet_from_name(std::ostream *out, const std::string &name, const std::string &type, const std::string &function_name) const {
    if (type.starts_with("array<")) {
        generate_nondet_array(out, name, type, function_name);
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
        generate_block_assignment(out, parameter_name, mir_statement::reformat_value_by_type(parameter_name, parameter_type), true, all_variables, function_name);
    } else {
        generate_nondet_from_name(out, nondet_name, parameter_type, function_name);
    }
}

void mir_contract::generate_nondet_array(std::ostream *out, const std::string &name, const std::string &type, const std::string &function_name) const {
    for (int i = 0; i < _globals.ARRAY_SIZE; i++) {
        generate_nondet_from_name(out, name + "[" + std::to_string(i) + "]", get_c_subtype(type), function_name);
    }
}

void mir_contract::generate_serialization(std::ostream *out, const std::string &variable_name, const std::string& variable_type, unsigned int *counter) {
    std::smatch match;
    const std::regex uint_regex (R"(^u(\d+|size)$)");
    const std::regex float_regex (R"(^f(\d+)$)");
    if (std::regex_match(variable_type, match, uint_regex)) {
        int bits;
        if (match[1].str() == "size") {
            bits = 64;
        } else {
            bits = std::stoi(match[1].str());
        }
        for (int i = 0; i < bits; i+=8) {
            *out << "\tout[" << *counter << "] = x." << variable_name << " >> " << i << " & 0xFF;" << std::endl;
            (*counter)++;
        }
    } else if (std::regex_match(variable_type, match, float_regex)) {
        const std::string int_equiv = "u" + match[1].str();
        const int bits = std::stoi(match[1].str());
        *out << "\t" << int_equiv << "* " << utils::clean(variable_name) << "_ptr = (" << int_equiv << "*) &x." << variable_name << ";" << std::endl;
        for (int i = 0; i < bits; i+=8) {
            *out << "\tout[" << *counter << "] = (*" << utils::clean(variable_name) << "_ptr) >> " << i << " & 0xFF;" << std::endl;
            (*counter)++;
        }
    }
}

void mir_contract::generate_deserialization(std::ostream *out, const std::string &variable_name, const std::string &variable_type, unsigned int *counter) {
    std::smatch match;
    const std::regex uint_regex (R"(^u(\d+|size)$)");
    const std::regex float_regex (R"(^f(\d+)$)");
    if (std::regex_match(variable_type, match, uint_regex)) {
        int bits;
        if (match[1].str() == "size") {
            bits = 64;
        } else {
            bits = std::stoi(match[1].str());
        }
        *out << "\tx." << variable_name << " = 0;" << std::endl;
        for (int i = 0; i < bits; i+=8) {
            *out << "\tx." << variable_name << " = x." << variable_name << " | in[" << *counter << "] << " << i << ";" << std::endl;
            (*counter)++;
        }
    } else if (std::regex_match(variable_type, match, float_regex)) {
        const std::string int_equiv = "u" + match[1].str();
        const int bits = std::stoi(match[1].str());
        *out << "\t" << int_equiv << " " << utils::clean(variable_name) << "_int = 0;" << std::endl;
        for (int i = 0; i < bits; i+=8) {
            *out << "\t" << utils::clean(variable_name) << "_int = " << utils::clean(variable_name) << "_int | in[" << *counter << "] << " << i << ";" << std::endl;
            (*counter)++;
        }
        *out << "\t" << match[0].str() << "* " << utils::clean(variable_name) << "_ptr = (" << match[0].str() << "*) &" << utils::clean(variable_name) << "_int;" << std::endl;
        *out << "\tx." << variable_name << " = *" << utils::clean(variable_name) << "_ptr;" << std::endl;
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

void mir_contract::generate_main_function(std::ostream *out, const mir_statements &function_statements, const std::string &target_function_name, const mir_statements& functions) {
    mir_statement target_function_statement = get_target_function(function_statements, target_function_name);
    const std::string target_function_type = target_function_statement.get_ast_data().at("return_type").get<std::string>();
    const mir_statements target_function_parameters = target_function_statement.get_children({statement_type::parameter});
    mir_statements target_function_all_variables = mir_statement::get_all_variables(target_function_statement, _structs, functions);

    *out << "int main() {" << std::endl;

    *out << "\tSYSTEM_PROGRAM_ID = nondet_pubkey();" << std::endl;
    *out << "\tSYSVAR_RENT_ID = nondet_pubkey();" << std::endl;
    *out << std::endl;

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

void mir_contract::generate_maths_function(std::ostream *out, const std::string &operator_name, const std::string &function_name) {
    if (operator_name == "u_addition") {
        *out << "unsigned_math_result " << function_name << "_u_addition(unsigned long long int a, unsigned long long int b, unsigned long long int max, bool checked) {" << std::endl;
        *out << "\tunsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a + b;" << std::endl;
        *out << "\tresult.errors = a > max - b;" << std::endl;
        *out << "\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 0; Reason: Addition overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the addition overflow\");" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "i_addition") {
        *out << "signed_math_result " << function_name << "_i_addition(signed long long int a, signed long long int b, signed long long int max, signed long long int min, bool checked) {" << std::endl;
        *out << "\tsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a + b;" << std::endl;
        *out << "\tif (b < 0) {" << std::endl;
        *out << "\t\tresult.errors = a < min - b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 1; Reason: Addition underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the addition underflow\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = a > max - b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 0; Reason: Addition overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the addition overflow\");" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "f_addition") {
        *out << "float_math_result " << function_name << "_f_addition(double a, double b, double max, double min, bool checked) {" << std::endl;
        *out << "\tfloat_math_result result;" << std::endl;
        *out << "\tresult.value = a + b;" << std::endl;
        *out << "\t if (b < 0) {" << std::endl;
        *out << "\t\tresult.errors = a < min - b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 1; Reason: Addition underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the addition underflow\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = a > max - b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 0; Reason: Addition overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the addition overflow\");" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "u_subtraction") {
        *out << "unsigned_math_result " << function_name << "_u_subtraction(unsigned long long int a, unsigned long long int b, unsigned long long int max, bool checked) {" << std::endl;
        *out << "\tunsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a - b;" << std::endl;
        *out << "\tresult.errors = a < b;" << std::endl;
        *out << "\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 3; Reason: Subtraction underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the subtraction underflow\");" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "i_subtraction") {
        *out << "signed_math_result " << function_name << "_i_subtraction(signed long long int a, signed long long int b, signed long long int max, signed long long int min, bool checked) {" << std::endl;
        *out << "\tsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a - b;" << std::endl;
        *out << "\tif (b < 0) {" << std::endl;
        *out << "\t\tresult.errors = a > max + b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 2; Reason: Subtraction overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the subtraction overflow\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = a < min + b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 3; Reason: Subtraction underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the subtraction underflow\");" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "f_subtraction") {
        *out << "float_math_result " << function_name << "_f_subtraction(double a, double b, double max, double min, bool checked) {" << std::endl;
        *out << "\tfloat_math_result result;" << std::endl;
        *out << "\tresult.value = a - b;" << std::endl;
        *out << "\tif (b < 0) {" << std::endl;
        *out << "\t\tresult.errors = a > max + b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 2; Reason: Subtraction overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the subtraction overflow\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = a < min + b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 3; Reason: Subtraction underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the subtraction underflow\");" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "u_multiplication") {
        *out << "unsigned_math_result " << function_name << "_u_multiplication(unsigned long long int a, unsigned long long int b, unsigned long long int max, bool checked) {" << std::endl;
        *out << "\tunsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a * b;" << std::endl;
        *out << "\tif (b == 0) {" << std::endl;
        *out << "\t\tresult.errors = false;" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = a > max / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 4; Reason: Multiplication overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication overflow\");" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "i_multiplication") {
        *out << "signed_math_result " << function_name << "_i_multiplication(signed long long int a, signed long long int b, signed long long int max, signed long long int min, bool checked) {" << std::endl;
        *out << "\tsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a * b;" << std::endl;
        *out << "\tif (b == 0) {" << std::endl;
        *out << "\t\tresult.errors = false;" << std::endl;
        *out << "\t} else if ((b > 0) && (a > 0)) {" << std::endl;
        *out << "\t\tresult.errors = a > max / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 4; Reason: Multiplication overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication overflow\");" << std::endl;
        *out << "\t} else if ((b < 0) && (a < 0)) {" << std::endl;
        *out << "\t\tresult.errors = a < max / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 4; Reason: Multiplication overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication overflow\");" << std::endl;
        *out << "\t} else if ((b > 0) && (a < 0)) {" << std::endl;
        *out << "\t\tresult.errors = a < min / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 5; Reason: Multiplication underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication underflow\");" << std::endl;
        *out << "\t} else if ((b < 0) && (a > 0)) {" << std::endl;
        *out << "\t\tresult.errors = a > min / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 5; Reason: Multiplication underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication underflow\");" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "f_multiplication") {
        *out << "float_math_result " << function_name << "_f_multiplication(double a, double b, double max, double min, bool checked) {" << std::endl;
        *out << "\tfloat_math_result result;" << std::endl;
        *out << "\tresult.value = a * b;" << std::endl;
        *out << "\tif (b == 0) {" << std::endl;
        *out << "\t\tresult.errors = false;" << std::endl;
        *out << "\t} else if ((b > 0) && (a > 0)) {" << std::endl;
        *out << "\t\tresult.errors = a > max / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 4; Reason: Multiplication overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication overflow\");" << std::endl;
        *out << "\t} else if ((b < 0) && (a < 0)) {" << std::endl;
        *out << "\t\tresult.errors = a < max / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 4; Reason: Multiplication overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication overflow\");" << std::endl;
        *out << "\t} else if ((b > 0) && (a < 0)) {" << std::endl;
        *out << "\t\tresult.errors = a < min / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 5; Reason: Multiplication underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication underflow\");" << std::endl;
        *out << "\t} else if ((b < 0) && (a > 0)) {" << std::endl;
        *out << "\t\tresult.errors = a > min / b;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 5; Reason: Multiplication underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the multiplication underflow\");" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "u_division") {
        *out << "unsigned_math_result " << function_name << "_u_division(unsigned long long int a, unsigned long long int b, unsigned long long int max, bool checked) {" << std::endl;
        *out << "\tunsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a / b;" << std::endl;
        *out << "\tif (b == 0) {" << std::endl;
        *out << "\t\tresult.errors = true;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 7; Reason: Division by zero in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the division by zero\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = false;" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "i_division") {
        *out << "signed_math_result " << function_name << "_i_division(signed long long int a, signed long long int b, signed long long int max, signed long long int min, bool checked) {" << std::endl;
        *out << "\tsigned_math_result result;" << std::endl;
        *out << "\tresult.value = a / b;" << std::endl;
        *out << "\tif (b == 0) {" << std::endl;
        *out << "\t\tresult.errors = true;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 7; Reason: Division by zero in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the division by zero\");" << std::endl;
        *out << "\t} else if (b == -1) {" << std::endl;
        *out << "\t\tresult.errors = a == min;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 6; Reason: Division overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the division overflow\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = false;" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "f_division") {
        *out << "float_math_result " << function_name << "_f_division(double a, double b, double max, double min, bool checked) {" << std::endl;
        *out << "\tfloat_math_result result;" << std::endl;
        *out << "\tresult.value = a / b;" << std::endl;
        *out << "\tif (b == 0) {" << std::endl;
        *out << "\t\tresult.errors = true;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 7; Reason: Division by zero in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the division by zero\");" << std::endl;
        *out << "\t} else if (b == -1) {" << std::endl;
        *out << "\t\tresult.errors = a == min;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 6; Reason: Division overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the division overflow\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = false;" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "u_negation") {
        *out << "unsigned_math_result " << function_name << "_u_negation(unsigned long long int a, unsigned long long int max, bool checked) {" << std::endl;
        *out << "\tunsigned_math_result result;" << std::endl;
        *out << "\tresult.value = -a;" << std::endl;
        *out << "\tresult.errors = true;" << std::endl;
        *out << "\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 12; Reason: Negation underflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the negation underflow\");" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "i_negation") {
        *out << "signed_math_result " << function_name << "_i_negation(signed long long int a, signed long long int max, signed long long int min, bool checked) {" << std::endl;
        *out << "\tsigned_math_result result;" << std::endl;
        *out << "\tresult.value = -a;" << std::endl;
        *out << "\tif (a == min) {" << std::endl;
        *out << "\t\tresult.errors = true;" << std::endl;
        *out << "\t\t__ESBMC_assert(checked || !result.errors, \"Vulnerability Found: 8; Reason: Negation overflow in the function '" << function_name << "'; Solution: Ensure contract inputs are validated to prevent the negation overflow\");" << std::endl;
        *out << "\t} else {" << std::endl;
        *out << "\t\tresult.errors = false;" << std::endl;
        *out << "\t}" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    } else if (operator_name == "f_negation") {
        *out << "float_math_result " << function_name << "_f_negation(double a, double max, bool checked) {" << std::endl;
        *out << "\tfloat_math_result result;" << std::endl;
        *out << "\tresult.value = -a;" << std::endl;
        *out << "\tresult.errors = false;" << std::endl;
        *out << "\treturn result;" << std::endl;
        *out << "}" << std::endl;
    }
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
            std::string solution = "Add 'assert_eq!(" + std::get<1>(account_info) + ".owner, " + program_id_name + ")'";
            *out << "\t__ESBMC_assert(!state._0.is_success || state._1 == state." << std::get<0>(account_info) << ".get3, \"Vulnerability Found: 9; Reason: " << reason << "; Solution: " << solution << "\");" << std::endl;
        }

        // Check signers have signed the instruction
        // LOGIC: A successul return implies that the signer has signed the instruction
        for (auto account_info : signer_account_infos) {
            std::string reason = "The variable '" + std::get<1>(account_info) + "' is missing signer checks";
            std::string solution = "Add 'assert!(" + std::get<1>(account_info) + ".is_signer)'";
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

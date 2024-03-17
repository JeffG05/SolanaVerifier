#include <iostream>
#include <regex>
#include "mir_statement.h"
#include "utils.h"
#include "mir-types/mir_type_converter.h"
#include "mir-values/mir_value_converter.h"

mir_statement::mir_statement(const statement_type type, nlohmann::json data) {
    _type = type;
    _ast_tree["type"] = get_string_type();
    _ast_tree["children"] = nlohmann::json::array();
    for (auto& [key, value] : data.items()) {
        _ast_tree[key] = value;
    }
}

mir_statement mir_statement::extract_node(const mir_statement &statement) {
    return { statement.get_type(), statement.get_ast_data() };
}

mir_statement mir_statement::new_variable(const std::string& variable, const std::string& variable_type) {
    nlohmann::json data;
    data["variable"] = variable;
    data["variable_type"] = variable_type;
    return { statement_type::variable, data };
}

statement_type mir_statement::get_type() const {
    return _type;
}

std::string mir_statement::get_string_type() const {
    switch (_type) {
        case statement_type::root:
            return "Contract Root";
        case statement_type::function:
            return "Function";
        case statement_type::block:
            return "Block";
        case statement_type::assignment:
            return "Assignment";
        case statement_type::variable:
            return "Variable";
        case statement_type::parameter:
            return "Parameter";
        case statement_type::return_type:
            return "Return";
        case statement_type::branch:
            return "Branch";
        case statement_type::data_struct:
            return "Struct";
        case statement_type::debug:
            return "Debug";
        case statement_type::data_enum:
            return "Enum";
        case statement_type::data_enum_struct:
            return "Enum Struct";
        default:
            return "Unknown Statement";
    }
}

nlohmann::json mir_statement::get_ast() const {
    return _ast_tree;
}

nlohmann::json mir_statement::get_ast_data() const {
    nlohmann::json data;
    for (auto& [key, value] : _ast_tree.items()) {
        if (key != "type" && key != "children") {
            data[key] = value;
        }
    }
    return data;
}

mir_statements mir_statement::get_children() {
    mir_statements children;
    for (const auto& child: _ast_tree["children"]) {
        mir_statement child_statement = parse_json(child);
        children.push_back(child_statement);
    }
    return children;
}

mir_statements mir_statement::get_children(const std::initializer_list<statement_type> types) {
    mir_statements children;
    for (const auto& child: _ast_tree["children"]) {
        mir_statement child_statement = parse_json(child);
        for (const auto type: types) {
            if (child_statement.get_type() == type) {
                children.push_back(child_statement);
                break;
            }
        }
    }
    return children;
}

void mir_statement::update_ast_data(const std::string &key, const nlohmann::basic_json<>& new_value) {
    if (!get_ast_data().contains(key)) {
        std::throw_with_nested(std::runtime_error("Key does not exist in AST data: " + key));
    }
    _ast_tree[key] = new_value;
}

void mir_statement::add_child(const mir_statement& child) {
    _ast_tree["children"].push_back(child.get_ast());
}

void mir_statement::print(const int indent_level) {
    const auto indents = std::string(indent_level*2, ' ');
    const nlohmann::json data = get_ast_data();
    if (data.empty()) {
        std::cout << indents << get_string_type() << std::endl;
    } else {
        std::cout << indents << get_string_type() << " (";
        unsigned int i = 0;
        for (auto& [key, value] : data.items()) {
            std::cout << key << "=" << value;
            if (i < data.size() - 1) {
                std::cout << ",";
            }
            i++;
        }
        std::cout << ")" << std::endl;
    }
    for (const auto& child: _ast_tree["children"]) {
        mir_statement parsed_child = parse_json(child);
        parsed_child.print(indent_level + 1);
    }
}

mir_statement mir_statement::parse_json(const nlohmann::json &json) {
    statement_type type;
    const auto string_type = json["type"].get<std::string>();
    if (string_type == "Contract Root") {
        type = statement_type::root;
    } else if (string_type == "Function") {
        type = statement_type::function;
    } else if (string_type == "Block") {
        type = statement_type::block;
    } else if (string_type == "Assignment") {
        type = statement_type::assignment;
    } else if (string_type == "Variable") {
        type = statement_type::variable;
    } else if (string_type == "Parameter") {
        type = statement_type::parameter;
    } else if (string_type == "Return") {
        type = statement_type::return_type;
    } else if (string_type == "Branch") {
        type = statement_type::branch;
    } else if (string_type == "Struct") {
        type = statement_type::data_struct;
    } else if (string_type == "Debug") {
        type = statement_type::debug;
    } else if (string_type == "Enum") {
        type = statement_type::data_enum;
    } else if (string_type == "Enum Struct") {
        type = statement_type::data_enum_struct;
    } else {
        type = statement_type::unknown;
    }

    auto statement = mir_statement(type, extract_data(json));
    for (const auto& child: json["children"]) {
        const mir_statement parsed_child = parse_json(child);
        statement.add_child(parsed_child);
    }
    return statement;
}

nlohmann::json extract_data(nlohmann::json json) {
    nlohmann::json data;
    for (auto& [key, value] : json.items()) {
        if (key != "type" && key != "children") {
            data[key] = value;
        }
    }
    return data;
}

mir_statement mir_statement::create_root(const std::string &contract_name) {
    nlohmann::json data;
    data["contract"] = contract_name;
    return {statement_type::root, data};
}

std::optional<mir_statement> mir_statement::parse_function(std::list<std::string> lines, const mir_statements& structs) {
    // Create function header
    const std::string header_line = lines.front();
    std::optional<mir_statement> function_header_maybe = parse_function_header(header_line);
    if (!function_header_maybe.has_value()) {
        return std::nullopt;
    }
    mir_statement function_header = function_header_maybe.value();
    lines.pop_front();

    // Create variables
    while (!lines.front().empty()) {
        if (lines.front().starts_with("let")) {
            mir_statement variable_statement = parse_variable(lines.front());
            function_header.add_child(variable_statement);
        } else if (lines.front().starts_with("debug")) {
            std::optional<mir_statement> debug_statement = parse_debug(lines.front());
            if (debug_statement.has_value()) {
                function_header.add_child(debug_statement.value());
            }
        }
        lines.pop_front();
    }

    mir_statements all_variables = get_all_variables(function_header, structs);

    // Create blocks
    std::string function_name = function_header.get_ast_data().at("name");
    int promoted_i = 0;
    auto current_block_lines = std::list<std::string>();
    while (!lines.empty()) {
        const std::string line = lines.front();
        if (line_is_block(line)) {
            if (!current_block_lines.empty() && line_is_block(current_block_lines.front())) {
                mir_statement statement = parse_block(current_block_lines, all_variables, function_name, &promoted_i);
                function_header.add_child(statement);
            }
            current_block_lines.clear();
        }
        current_block_lines.push_back(line);
        lines.pop_front();
    }

    if (!current_block_lines.empty() && line_is_block(current_block_lines.front())) {
        mir_statement statement = parse_block(current_block_lines, all_variables, function_name, &promoted_i);
        function_header.add_child(statement);
    }
    current_block_lines.clear();

    return function_header;
}

std::optional<mir_statement> mir_statement::parse_function_header(const std::string& line) {
    const std::regex imported_lib_function_regex (R"(^fn (<.+?>)::(.+?)\((.*?)\) -> (.+?) \{$)");
    const std::regex imported_local_function_regex (R"(^fn (.+?)::(.+?)\((.*?)\) -> (.+?) \{$)");
    const std::regex local_function_regex (R"(^fn (.+?)\((.*?)\) -> (.*?) \{$)");
    const std::regex promoted_function_regex (R"(^promoted\[(\d+)\] in (.+): (.+?) = \{$)");

    nlohmann::json function_data;
    std::string parameters;
    std::smatch match;
    if (regex_match(line, match, imported_lib_function_regex)) {
        function_data["source"] = match[1].str();
        function_data["name"] = match[2].str();
        function_data["return_type"] = convert_type(match[4].str());
        parameters = match[3].str();
    } else if (regex_match(line, match, imported_local_function_regex)) {
        function_data["name"] = match[1].str() + "__" + match[2].str();
        function_data["return_type"] = convert_type(match[4].str());
        parameters = match[3].str();
    } else if (regex_match(line, match, local_function_regex)) {
        function_data["name"] = match[1].str();
        function_data["return_type"] = convert_type(match[3].str());
        parameters = match[2].str();
    } else if (regex_match(line, match, promoted_function_regex)) {
        if (match[2].str().starts_with("<")) {
            return std::nullopt;
        }
        function_data["name"] = "promoted_" + match[2].str() + "_" + match[1].str();
        function_data["return_type"] = convert_type(match[3].str());
        parameters = "";
    }

    auto function_statement = mir_statement(statement_type::function, function_data);
    for (const auto& param: utils::split(parameters, ", ")) {
        mir_statement parameter_statement = parse_variable(param);
        function_statement.add_child(parameter_statement);
    }
    return function_statement;
}

mir_statement mir_statement::parse_block(std::list<std::string> lines, const mir_statements &variables, const std::string& function_name, int *promoted_i) {
    // Create block header
    const std::string header_line = lines.front();
    mir_statement block = parse_block_header(header_line);
    lines.pop_front();

    // Create assignments
    while (!lines.empty()) {
        std::string line = lines.front();
        if (std::optional<mir_statements> assignments = parse_assignment(line, variables); assignments.has_value()) {
            for (auto& assignment: assignments.value()) {
                if (assignment.get_type() == statement_type::assignment && assignment.get_ast_data().at("value") == "promoted<>") {
                    std::string promoted_func = "promoted_" + function_name + "_" + std::to_string(*promoted_i) + "()";
                    assignment.update_ast_data("value", promoted_func);
                    *promoted_i += 1;
                }
                block.add_child(assignment);
            }
        }
        lines.pop_front();
    }

    return block;
}

mir_statement mir_statement::parse_block_header(const std::string &line) {
    nlohmann::json data;
    const std::regex block_regex (R"(^(bb\d+)(?: \(cleanup\))?: \{$)");
    if (std::smatch match; std::regex_match(line, match, block_regex)) {
        data["name"] = match[1].str();
    }
    return {statement_type::block, data};
}


std::optional<mir_statements> mir_statement::parse_assignment(const std::string &line, const mir_statements& variables) {
    nlohmann::json data;
    std::string branching;
    std::string assertion;

    const std::regex assignment_parts (R"(^(.+) = (.+?)(?: -> (.+))?;$)");
    const std::regex assert_parts (R"(^assert\((.+), \"(.+)\".*\) -> (.+);$)");
    const std::regex goto_parts (R"(^goto -> (.+);$)");
    const std::regex drop_parts (R"(^drop\(.+\) -> (.+);$)");
    const std::regex unreachable_parts (R"(^unreachable;$)");
    const std::regex switchint_parts (R"(^switchInt\((.+)\) -> (.+);$)");
    std::smatch match;
    if (regex_match(line, match, assignment_parts)) {
        auto [value1, returns1] = convert_value(match[1].str(), variables);
        data["variable"] = value1.starts_with("state.") ? value1.substr(6) : value1;
        auto [value2, returns2] = convert_value(match[2].str(), variables);
        data["value"] = value2;
        data["returns"] = returns2;
        branching = match[3].str();
    } else if (regex_match(line, match, assert_parts)) {
        data["variable"] = "";
        auto [value, returns] = convert_value(match[1].str(), variables);
        assertion = "__ESBMC_assert(" + value + ", \"Vulnerability Found: " + match[2].str() + "\")";
        data["value"] = "true";
        data["returns"] = true;
        branching = match[3].str();
    } else if (regex_match(line, match, goto_parts)) {
        data["variable"] = "";
        data["value"] = "true";
        data["returns"] = true;
        branching = match[1].str();
    } else if (regex_match(line, match, drop_parts)) {
        data["variable"] = "";
        data["value"] = "true";
        data["returns"] = true;
        branching = match[1].str();
    } else if (regex_match(line, match, unreachable_parts)) {
        data["variable"] = "";
        data["value"] = "assert(false)";
        data["returns"] = false;
        branching = "";
    } else if (regex_match(line, match, switchint_parts)) {
        data["variable"] = "";
        auto [value, returns] = convert_value(match[1].str(), variables);
        data["value"] = value;
        data["returns"] = returns;
        branching = match[2].str();
    } else {
        return std::nullopt;
    }

    std::string variable_name = data.at("variable");
    if (!variable_name.empty()) {
        std::optional<mir_statement> variable_statement = get_statement(variables, variable_name);
        if (variable_statement.has_value()) {
            std::string variable_type = variable_statement.value().get_ast_data().at("variable_type");
            data["value"] = reformat_value_by_type(data.at("value"), variable_type);
        }
    }

    mir_statements all_statements;

    if (!assertion.empty()) {
        nlohmann::json assertion_data;
        assertion_data["variable"] = "";
        assertion_data["value"] = assertion;
        assertion_data["returns"] = false;
        auto assertion_statement = mir_statement(statement_type::assignment, assertion_data);
        all_statements.push_back(assertion_statement);
    }

    auto assignment_statement = mir_statement(statement_type::assignment, data);
    if (!branching.empty()) {
        if (branching.starts_with("[") && branching.ends_with("]")) {
            branching = branching.substr(1, branching.size() - 2);
        }
        for (const auto& branch_str: utils::split(branching, ", ")) {
            if (std::optional<mir_statement> branch_statement = parse_branch(branch_str); branch_statement.has_value()) {
                assignment_statement.add_child(branch_statement.value());
            }
        }
    }
    all_statements.push_back(assignment_statement);

    return all_statements;
}

std::string mir_statement::reformat_value_by_type(const std::string &value, const std::string &type) {
    if (type.starts_with("array<")) {
        if (!value.starts_with("init_array<") && !value.starts_with("str_as_bytes<")) {
            return "copy_array<" + value + ">";
        }
        return value;
    }
    if (type.starts_with("tuple<")) {
        if (!value.starts_with("init_tuple<") && !value.starts_with("find_program_address<")) {
            return "copy_tuple<" + value + ">";
        }
        return value;
    }
    if (type == "pubkey") {
        return "copy_pubkey<" + value + ">";
    }
    if (type == "account_info") {
        return "copy_account_info<" + value + ">";
    }
    if (type == "account_meta") {
        if (!value.starts_with("init_account_meta<")) {
            return "copy_account_meta<" + value + ">";
        }
        return value;
    }
    if (type == "solana_instruction") {
        if (!value.starts_with("init_solana_instruction<")) {
            return "copy_solana_instruction<" + value + ">";
        }
        return value;
    }
    if (type.starts_with("result<")) {
        if (type == "result<void>") {
            return "copy_void_result<" + value + ">";
        }
        return "copy_result<" + value + ">";
    }
    return value;
}

mir_statement mir_statement::parse_variable(const std::string &line) {
    std::smatch match;
    const std::regex var_regex ("^let(?: mut)? (_\\d+): (.+?);$");
    const std::regex param_regex ("^(_\\d+): (.+?)$");

    if (std::regex_match(line, match, var_regex)) {
        return new_variable(match[1].str(), convert_type(match[2].str()));
    }

    nlohmann::json data;
    if (std::regex_match(line, match, param_regex)) {
        data["variable"] = match[1].str();
        data["variable_type"] = convert_type(match[2].str());
        return {statement_type::parameter, data};
    }

    return {statement_type::variable, data};
}

std::optional<mir_statement> mir_statement::parse_debug(const std::string &line) {
    std::smatch match;
    const std::regex debug_regex ("^debug (.+) => (_\\d+);$");

    if (std::regex_match(line, match, debug_regex)) {
        nlohmann::json data;
        data["name"] = match[1].str();
        data["variable"] = match[2].str();
        return mir_statement(statement_type::debug, data);
    }

    return std::nullopt;
}

std::optional<mir_statement> mir_statement::parse_branch(const std::string &line) {
    nlohmann::json data;

    std::smatch match;
    const std::regex return_regex ("^return: (bb\\d+)$");
    const std::regex success_regex ("^success: (bb\\d+)$");
    const std::regex switch_int_regex ("^(\\d+): (bb\\d+)$");
    const std::regex otherwise_regex ("^otherwise: (bb\\d+)$");
    const std::regex unwind_regex ("^unwind: (bb\\d+)$");
    const std::regex block_regex ("^bb\\d+$");

    if (std::regex_match(line, match, return_regex)) {
        data["condition"] = "true";
        data["destination"] = match[1].str();
        data["ignore_var"] = true;
    } else if (std::regex_match(line, match, success_regex)) {
        data["condition"] = "true";
        data["destination"] = match[1].str();
        data["ignore_var"] = false;
    } else if (std::regex_match(line, match, switch_int_regex)) {
        data["condition"] = match[1].str();
        data["destination"] = match[2].str();
        data["ignore_var"] = false;
    } else if (std::regex_match(line, match, otherwise_regex)) {
        data["condition"] = "true";
        data["destination"] = match[1].str();
        data["ignore_var"] = true;
    } else if (std::regex_match(line, match, unwind_regex)) {
        data["condition"] = "false";
        data["destination"] = match[1].str();
        data["ignore_var"] = true;
    } else if (std::regex_match(line, match, block_regex)) {
        data["condition"] = "true";
        data["destination"] = match[0].str();
        data["ignore_var"] = true;
    } else {
        return std::nullopt;
    }

    return mir_statement(statement_type::branch, data);
}

std::string mir_statement::convert_type(const std::string &type) {
    return mir_type_converter::convert(type);
}

std::tuple<std::string, bool> mir_statement::convert_value(const std::string &value, const mir_statements &variables) {
    return mir_value_converter::convert(value, variables);
}

bool mir_statement::line_is_function(const std::string &line) {
    return line.starts_with("fn ") || line.starts_with("promoted[");
}

bool mir_statement::line_is_block(const std::string &line) {
    const std::regex block_regex (R"(^bb\d+(?: \(cleanup\))?: \{$)");
    std::smatch match;
    return std::regex_match(line, match, block_regex);
}


bool mir_statement::line_is_statement_start(const std::string &line) {
    return line_is_function(line);
}

std::pair<std::string, std::string> mir_statement::get_argument_pair(const std::string &raw) {
    const std::regex pattern (R"(^([^<>,]+<[^<>]+>|[^<>,]+), ([^<>,]+<[^<>]+>|[^<>,]+)$)");
    if (std::smatch match; std::regex_match(raw, match, pattern)) {
        return std::make_pair(match[1].str(), match[2].str());
    }
    std::throw_with_nested(std::runtime_error("Error parsing argument pair: " + raw));
}

std::optional<mir_statement> mir_statement::get_statement(const mir_statements &variables, const std::string &name) {
    const std::regex indexed_regex (R"(\[.+?\])");
    std::string search_name = std::regex_replace(name, indexed_regex, ".t");

    for (const auto& variable: variables) {
        std::string var_name = variable.get_ast_data().at("variable");
        if (var_name == search_name) {
            return variable;
        }
    }
    return std::nullopt;
}

mir_statements mir_statement::get_all_variables(mir_statement function_header, const mir_statements& structs) {
    mir_statements all_variables;
    for (const auto& variable: function_header.get_children({statement_type::variable, statement_type::parameter})) {
        utils::extend(&all_variables, get_subvariables(variable, structs));
    }
    return all_variables;
}

mir_statements mir_statement::get_subvariables(const mir_statement &variable, const mir_statements &structs) {
    mir_statements subvariables;
    subvariables.push_back(variable);

    const std::string name = variable.get_ast_data().at("variable");
    const std::string type = variable.get_ast_data().at("variable_type");

    for (auto struct_statement: structs) {
        if (type == struct_statement.get_ast_data().at("name")) {
            for (const auto& child_variable: struct_statement.get_children()) {
                const std::string child_type = child_variable.get_ast_data().at("variable_type");
                std::string new_var_name = name + ".";
                new_var_name += child_variable.get_ast_data().at("variable");
                const mir_statement subvariable = new_variable(new_var_name, child_type);
                utils::extend(&subvariables, get_subvariables(subvariable, structs));
            }
        }
    }
    if (type == "pubkey") {
        for (int i = 0; i < 32; i++) {
            const mir_statement p = new_variable(name + ".p" + std::to_string(i), "u8");
            utils::extend(&subvariables, get_subvariables(p, structs));
        }
    }
    if (type == "account_info") {
        const mir_statement get0 = new_variable(name + ".get0", "pubkey");
        const mir_statement get1 = new_variable(name + ".get1", "u64");
        const mir_statement get2 = new_variable(name + ".get2", "array<u8>");
        const mir_statement get3 = new_variable(name + ".get3", "pubkey");
        const mir_statement get4 = new_variable(name + ".get4", "u64");
        const mir_statement get5 = new_variable(name + ".get5", "bool");
        const mir_statement get6 = new_variable(name + ".get6", "bool");
        const mir_statement get7 = new_variable(name + ".get7", "bool");

        utils::extend(&subvariables, get_subvariables(get0, structs));
        utils::extend(&subvariables, get_subvariables(get1, structs));
        utils::extend(&subvariables, get_subvariables(get2, structs));
        utils::extend(&subvariables, get_subvariables(get3, structs));
        utils::extend(&subvariables, get_subvariables(get4, structs));
        utils::extend(&subvariables, get_subvariables(get5, structs));
        utils::extend(&subvariables, get_subvariables(get6, structs));
        utils::extend(&subvariables, get_subvariables(get7, structs));
        return subvariables;
    }
    if (type == "account_meta") {
        const mir_statement get0 = new_variable(name + ".get0", "pubkey");
        const mir_statement get1 = new_variable(name + ".get1", "bool");
        const mir_statement get2 = new_variable(name + ".get2", "bool");

        utils::extend(&subvariables, get_subvariables(get0, structs));
        utils::extend(&subvariables, get_subvariables(get1, structs));
        utils::extend(&subvariables, get_subvariables(get2, structs));
        return subvariables;
    }
    if (type == "solana_instruction") {
        const mir_statement get0 = new_variable(name + ".get0", "pubkey");
        const mir_statement get1 = new_variable(name + ".get1", "array<account_meta>");
        const mir_statement get2 = new_variable(name + ".get2", "array<u8>");

        utils::extend(&subvariables, get_subvariables(get0, structs));
        utils::extend(&subvariables, get_subvariables(get1, structs));
        utils::extend(&subvariables, get_subvariables(get2, structs));
        return subvariables;
    }
    if (type.starts_with("controlflow<")) {
        std::string raw_controlflow_types = type.substr(12, type.size() - 13);
        std::pair<std::string, std::string> types = get_argument_pair(raw_controlflow_types);

        const mir_statement break_value = new_variable(name + ".break_value", types.first);
        const mir_statement continue_value = new_variable(name + ".continue_value", types.second);

        utils::extend(&subvariables, get_subvariables(break_value, structs));
        utils::extend(&subvariables, get_subvariables(continue_value, structs));
        return subvariables;
    }
    if (type.starts_with("result<")) {
        std::string success_type = type.substr(7, type.size() - 8);

        const mir_statement is_success = new_variable(name + ".is_success", "bool");
        utils::extend(&subvariables, get_subvariables(is_success, structs));

        const mir_statement value = new_variable(name + ".value", success_type);
        utils::extend(&subvariables, get_subvariables(value, structs));
        return subvariables;
    }
    if (type.starts_with("tuple<")) {
        const std::string raw_value_types = type.substr(6, type.size() - 7);
        const std::list<std::string> value_types = utils::split(raw_value_types, ", ");

        unsigned int i = 0;
        for (const auto& value_type: value_types) {
            const mir_statement statement = new_variable(name + ".get" + std::to_string(i), value_type);
            utils::extend(&subvariables, get_subvariables(statement, structs));
            i++;
        }
        return subvariables;
    }
    if (type.starts_with("array<")) {
        std::string array_type = type.substr(6, type.size() - 7);
        const mir_statement array_item = new_variable(name + ".t", array_type);
        utils::extend(&subvariables, get_subvariables(array_item, structs));
    }
    if (type.starts_with("optional<")) {
        std::string optional_type = type.substr(9, type.size() - 10);

        const mir_statement is_none = new_variable(name + ".is_none", "bool");
        utils::extend(&subvariables, get_subvariables(is_none, structs));

        const mir_statement value = new_variable(name + ".value", optional_type);
        utils::extend(&subvariables, get_subvariables(value, structs));
        return subvariables;
    }

    return subvariables;
}














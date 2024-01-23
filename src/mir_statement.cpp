#include <iostream>
#include <regex>
#include "mir_statement.h"
#include "utils.h"

mir_statement::mir_statement(const statement_type type, nlohmann::json data) {
    _type = type;
    _ast_tree["type"] = get_string_type();
    _ast_tree["children"] = nlohmann::json::array();
    for (auto& [key, value] : data.items()) {
        _ast_tree[key] = value;
    }
}

statement_type mir_statement::get_type() const {
    return _type;
}

std::string mir_statement::get_string_type() const {
    switch (_type) {
        case root:
            return "Contract Root";
        case function:
            return "Function";
        case block:
            return "Block";
        case assignment:
            return "Assignment";
        case variable:
            return "Variable";
        case parameter:
            return "Parameter";
        case return_type:
            return "Return";
        case branch:
            return "Branch";
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

std::list<mir_statement> mir_statement::get_children() {
    std::list<mir_statement> children;
    for (const auto& child: _ast_tree["children"]) {
        mir_statement child_statement = parse_json(child);
        children.push_back(child_statement);
    }
    return children;
}

std::list<mir_statement> mir_statement::get_children(std::initializer_list<statement_type> types) {
    std::list<mir_statement> children;
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
        type = root;
    } else if (string_type == "Function") {
        type = function;
    } else if (string_type == "Block") {
        type = block;
    } else if (string_type == "Assignment") {
        type = assignment;
    } else if (string_type == "Variable") {
        type = variable;
    } else if (string_type == "Parameter") {
        type = parameter;
    } else if (string_type == "Return") {
        type = return_type;
    } else if (string_type == "Branch") {
        type = branch;
    } else {
        type = unknown;
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
    return {root, data};
}

std::optional<mir_statement> mir_statement::parse_lines(const std::list<std::string> &lines) {
    if (line_is_function(lines.front())) {
        return parse_function(lines);
    }

    if (line_is_block(lines.front())) {
        return parse_block(lines);
    }

    return std::nullopt;
}


mir_statement mir_statement::parse_function(std::list<std::string> lines) {
    // Create function header
    const std::string header_line = lines.front();
    mir_statement function_header = parse_function_header(header_line);
    lines.pop_front();

    // Create variables
    while (lines.front().starts_with("debug")) {
        lines.pop_front();
    }

    while (lines.front().starts_with("let")) {
        mir_statement variable_statement = parse_variable(lines.front());
        function_header.add_child(variable_statement);
        lines.pop_front();
    }

    // Create blocks
    auto current_block_lines = std::list<std::string>();
    while (!lines.empty()) {
        const std::string line = lines.front();
        if (line_is_block(line)) {
            if (!current_block_lines.empty()) {
                if (std::optional<mir_statement> statement = parse_lines(current_block_lines); statement.has_value()) {
                    function_header.add_child(statement.value());
                }
                current_block_lines.clear();
            }
        }
        current_block_lines.push_back(line);
        lines.pop_front();
    }

    if (!current_block_lines.empty()) {
        if (const std::optional<mir_statement> statement = parse_lines(current_block_lines); statement.has_value()) {
            function_header.add_child(statement.value());
        }
        current_block_lines.clear();
    }

    return function_header;
}

mir_statement mir_statement::parse_function_header(const std::string& line) {
    const std::regex imported_lib_function_regex (R"(^fn (<.+?>)::(.+?)\((.*?)\) -> (.*?) \{$)");
    const std::regex imported_local_function_regex (R"(^fn (.+?)::(.+?)\((.*?)\) -> (.*?) \{$)");
    const std::regex local_function_regex (R"(^fn (.+?)\((.*?)\) -> (.*?) \{$)");

    nlohmann::json function_data;
    std::string parameters;
    std::smatch match;
    if (regex_match(line, match, imported_lib_function_regex) || regex_match(line, match, imported_local_function_regex)) {
        function_data["source"] = match[1].str();
        function_data["name"] = match[2].str();
        function_data["return_type"] = convert_type(match[4].str());
        parameters = match[3].str();
    } else if (regex_match(line, match, local_function_regex)) {
        function_data["name"] = match[1].str();
        function_data["return_type"] = convert_type(match[3].str());
        parameters = match[2].str();
    }

    auto function_statement = mir_statement(function, function_data);
    for (const auto& param: utils::split(parameters, ", ")) {
        mir_statement parameter_statement = parse_variable(param);
        function_statement.add_child(parameter_statement);
    }
    return function_statement;
}

std::optional<mir_statement> mir_statement::parse_block(std::list<std::string> lines) {
    // Create block header
    const std::string header_line = lines.front();
    mir_statement block = parse_block_header(header_line);
    lines.pop_front();

    // Create assignments
    while (!lines.empty()) {
        std::string line = lines.front();
        if (std::optional<mir_statement> assignment = parse_assignment(line); assignment.has_value()) {
            block.add_child(assignment.value());
        }
        lines.pop_front();
    }

    return block;
}

mir_statement mir_statement::parse_block_header(const std::string &line) {
    nlohmann::json data;
    const std::regex block_regex (R"(^(bb\d+): \{$)");
    if (std::smatch match; std::regex_match(line, match, block_regex)) {
        data["name"] = match[1].str();
    }
    return {block, data};
}


std::optional<mir_statement> mir_statement::parse_assignment(const std::string &line) {
    nlohmann::json data;
    std::string branching;
    const std::regex assignment_parts (R"(^(\w+) = (.*?)(?: -> (.*))?;$)");
    if (std::smatch match; regex_match(line, match, assignment_parts)) {
        data["variable"] = match[1].str();
        data["value"] = convert_value(match[2].str());
        branching = match[3].str();
    } else {
        return std::nullopt;
    }

    auto assignment_statement = mir_statement(assignment, data);
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
    return assignment_statement;
}

mir_statement mir_statement::parse_variable(const std::string &line) {
    nlohmann::json data;

    std::smatch match;
    const std::regex var_regex ("^let(?: mut)? (_\\d+): (.+?);$");
    const std::regex param_regex ("^(_\\d+): (.+?)$");

    if (std::regex_match(line, match, var_regex)) {
        data["variable"] = match[1].str();
        data["variable_type"] = convert_type(match[2].str());
        return {variable, data};
    }

    if (std::regex_match(line, match, param_regex)) {
        data["variable"] = match[1].str();
        data["variable_type"] = convert_type(match[2].str());
        return {parameter, data};
    }

    return {variable, data};
}

std::optional<mir_statement> mir_statement::parse_branch(const std::string &line) {
    nlohmann::json data;

    std::smatch match;
    const std::regex return_regex ("^return: (bb\\d+)$");
    const std::regex switch_int_regex ("^(\\d+): (bb\\d+)$");
    const std::regex otherwise_regex ("^otherwise: (bb\\d+)$");
    const std::regex unwind_regex ("^unwind: (bb\\d+)$");
    const std::regex block_regex ("^bb\\d+$");

    if (std::regex_match(line, match, return_regex)) {
        data["condition"] = "true";
        data["destination"] = match[1].str();
    } else if (std::regex_match(line, match, switch_int_regex)) {
        data["condition"] = match[1].str();
        data["destination"] = match[2].str();
    } else if (std::regex_match(line, match, otherwise_regex)) {
        data["condition"] = "true";
        data["destination"] = match[1].str();
    } else if (std::regex_match(line, match, unwind_regex)) {
        data["condition"] = "false";
        data["destination"] = match[1].str();
    } else if (std::regex_match(line, match, block_regex)) {
        data["condition"] = "true";
        data["destination"] = match[0].str();
    } else {
        return std::nullopt;
    }

    return mir_statement(branch, data);
}


std::string mir_statement::convert_type(const std::string &type) {
    if (type.starts_with("&")) {
        return convert_type(type.substr(1));
    }
    if (type.starts_with("*")) {
        return convert_type(type.substr(1));
    }
    if (type.starts_with("mut ")) {
        return convert_type(type.substr(4));
    }
    if (type.starts_with("[") && type.ends_with("]")) {
        return "array " + convert_type(type.substr(1, type.size()-2));
    }
    if (type.starts_with("std::vec::Vec<") && type.ends_with(">")) {
        return "array " + convert_type(type.substr(14, type.size()-15));
    }
    if (type.starts_with("`") && type.ends_with("`")) {
        return convert_type(type.substr(1, type.size()-2));
    }
    if (type == "Result<(), ProgramError>" || type == "std::result::Result<(), solana_program::program_error::ProgramError>") {
        return "program_result";
    }
    if (type == "u8" || type == "u16" || type == "u32" || type == "u64" || type == "usize") {
        return type;
    }
    if (type == "i8" || type == "i16" || type == "i32" || type == "i64" || type == "usize") {
        return type;
    }
    if (type == "str") {
        return "string";
    }
    if (type == "()") {
        return "bool";
    }
    if (type == "AccountInfo<'_>" || type == "solana_program::account_info::AccountInfo<'_>") {
        return "account_info";
    }
    if (type == "Pubkey" || type == "solana_program::pubkey::Pubkey") {
        return "pubkey";
    }

    return type;
}

std::string mir_statement::convert_value(const std::string &value) {
    if (value.starts_with("&")) {
        return convert_value(value.substr(1));
    }

    if (value.starts_with("*")) {
        return convert_value(value.substr(1));
    }

    std::smatch match;

    const std::regex variable_regex ("^_\\d+$");
    if (std::regex_match(value, match, variable_regex)) {
        return "state." + value;
    }

    const std::regex string_const ("^const (\".*\")$");
    if (std::regex_match(value, match, string_const)) {
        return match[1].str();
    }

    const std::regex ok_result (R"(^Result::<\(\), ProgramError>::Ok\(.*\)$)");
    if (std::regex_match(value, match, ok_result)) {
        return "ok";
    }

    const std::regex sol_log (R"(^solana_program::log::sol_log\((.+)\)$)");
    if (std::regex_match(value, match, sol_log)) {
        return "print(" + convert_value(match[1].str()) + ")";
    }

    const std::regex deref_copy (R"(^deref_copy \((.+)\)$)");
    if (std::regex_match(value, match, deref_copy)) {
        return convert_value(match[1].str());
    }

    const std::regex deref (R"(^<.+ as Deref>::deref\((.+)\)$)");
    if (std::regex_match(value, match, deref)) {
        return convert_value(match[1].str());
    }

    const std::regex array_indexer (R"(^\((.+)\.(\d+): .+\)$)");
    if (std::regex_match(value, match, array_indexer)) {
        return convert_value(match[1].str()) + "[" + match[2].str() + "]";
    }

    const std::regex move (R"(^move (.+)$)");
    if (std::regex_match(value, match, move)) {
        return convert_value(match[1].str());
    }

    return value;
}


bool mir_statement::line_is_function(const std::string &line) {
    if (line.starts_with("fn")) {
        if (!line.starts_with("fn entrypoint(")) {
            return true;
        }
    }
    return false;
}

bool mir_statement::line_is_block(const std::string &line) {
    const std::regex block_regex (R"(^bb\d+: \{$)");
    std::smatch match;
    return std::regex_match(line, match, block_regex);
}


bool mir_statement::line_is_statement_start(const std::string &line) {
    return line_is_function(line);
}












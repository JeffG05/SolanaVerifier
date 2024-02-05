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
        case statement_type::add_ref:
            return "Add Reference";
        case statement_type::remove_ref:
            return "Remove Reference";
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
    } else if (string_type == "Add Reference") {
        type = statement_type::add_ref;
    } else if (string_type == "Remove Reference") {
        type = statement_type::remove_ref;
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

std::optional<mir_statement> mir_statement::parse_lines(const std::list<std::string> &lines, const mir_statements& variables) {
    if (line_is_function(lines.front())) {
        return parse_function(lines);
    }

    if (line_is_block(lines.front())) {
        return parse_block(lines, variables);
    }

    return std::nullopt;
}


mir_statement mir_statement::parse_function(std::list<std::string> lines) {
    // Create function header
    const std::string header_line = lines.front();
    mir_statement function_header = parse_function_header(header_line);
    lines.pop_front();

    // Create variables
    while (!lines.front().empty()) {
        if (lines.front().starts_with("let")) {
            mir_statement variable_statement = parse_variable(lines.front());
            function_header.add_child(variable_statement);
        }
        lines.pop_front();
    }

    mir_statements all_variables = function_header.get_children();

    // Create blocks
    auto current_block_lines = std::list<std::string>();
    while (!lines.empty()) {
        const std::string line = lines.front();
        if (line_is_block(line)) {
            if (!current_block_lines.empty()) {
                if (std::optional<mir_statement> statement = parse_lines(current_block_lines, all_variables); statement.has_value()) {
                    function_header.add_child(statement.value());
                }
                current_block_lines.clear();
            }
        }
        current_block_lines.push_back(line);
        lines.pop_front();
    }

    if (!current_block_lines.empty()) {
        if (const std::optional<mir_statement> statement = parse_lines(current_block_lines, all_variables); statement.has_value()) {
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

    auto function_statement = mir_statement(statement_type::function, function_data);
    for (const auto& param: utils::split(parameters, ", ")) {
        mir_statement parameter_statement = parse_variable(param);
        function_statement.add_child(parameter_statement);
    }
    return function_statement;
}

std::optional<mir_statement> mir_statement::parse_block(std::list<std::string> lines, const mir_statements &variables) {
    // Create block header
    const std::string header_line = lines.front();
    mir_statement block = parse_block_header(header_line);
    lines.pop_front();

    // Create assignments
    while (!lines.empty()) {
        std::string line = lines.front();
        if (std::optional<mir_statements> assignments = parse_assignment(line, variables); assignments.has_value()) {
            for (const auto& assignment: assignments.value()) {
                block.add_child(assignment);
            }
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
    return {statement_type::block, data};
}


std::optional<mir_statements> mir_statement::parse_assignment(const std::string &line, const mir_statements& variables) {
    nlohmann::json data;
    std::string branching;
    std::string add_ref_to;
    std::string remove_ref_to;
    const std::regex assignment_parts (R"(^(\w+) = (.*?)(?: -> (.*))?;$)");
    const std::regex assert_parts (R"(^assert\((.+), ".*\) -> (.*);$)");
    std::smatch match;
    if (regex_match(line, match, assignment_parts)) {
        data["variable"] = match[1].str();
        auto [value, returns, add_ref, remove_ref] = convert_value(match[2].str(), variables);
        data["value"] = value;
        data["returns"] = returns;
        add_ref_to = add_ref;
        remove_ref_to = remove_ref;
        branching = match[3].str();
    } else if (regex_match(line, match, assert_parts)) {
        data["variable"] = "";
        auto [value, returns, add_ref, remove_ref] = convert_value(match[1].str(), variables);
        data["value"] = value;
        data["returns"] = returns;
        add_ref_to = add_ref;
        remove_ref_to = remove_ref;
        branching = match[2].str();
    } else {
        return std::nullopt;
    }

    mir_statements all_statements;

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

    if (!add_ref_to.empty()) {
        for (const auto& add_ref : utils::split(add_ref_to, ", ")) {
            nlohmann::json add_data;
            add_data["variable1"] = std::get<0>(mir_value_converter::convert(data["variable"].get<std::string>(), variables));
            add_data["variable2"] = add_ref;
            if (add_data["variable1"].get<std::string>().empty() || add_data["variable2"].get<std::string>().empty()) {
                continue;
            }
            auto add_ref_statement = mir_statement(statement_type::add_ref, add_data);
            all_statements.push_back(add_ref_statement);
        }
    }

    if (!remove_ref_to.empty()) {
        for (const auto& remove_ref : utils::split(remove_ref_to, ", ")) {
            nlohmann::json remove_data;
            remove_data["variable1"] = std::get<0>(mir_value_converter::convert(data["variable"].get<std::string>(), variables));
            remove_data["variable2"] = remove_ref;
            if (remove_data["variable1"].get<std::string>().empty() || remove_data["variable2"].get<std::string>().empty()) {
                continue;
            }
            auto remove_ref_statement = mir_statement(statement_type::remove_ref, remove_data);
            all_statements.push_back(remove_ref_statement);
        }
    }

    return all_statements;
}

mir_statement mir_statement::parse_variable(const std::string &line) {
    nlohmann::json data;

    std::smatch match;
    const std::regex var_regex ("^let(?: mut)? (_\\d+): (.+?);$");
    const std::regex param_regex ("^(_\\d+): (.+?)$");

    if (std::regex_match(line, match, var_regex)) {
        data["variable"] = match[1].str();
        data["variable_type"] = convert_type(match[2].str());
        return {statement_type::variable, data};
    }

    if (std::regex_match(line, match, param_regex)) {
        data["variable"] = match[1].str();
        data["variable_type"] = convert_type(match[2].str());
        return {statement_type::parameter, data};
    }

    return {statement_type::variable, data};
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

std::tuple<std::string, bool, std::string, std::string> mir_statement::convert_value(const std::string &value, const mir_statements &variables) {
    return mir_value_converter::convert(value, variables);
}


// std::string mir_statement::convert_value(const std::string &value, const mir_statements& variables) {
//     if (value.starts_with("&")) {
//         return convert_value(value.substr(1), variables);
//     }
//
//     if (value.starts_with("*")) {
//         return convert_value(value.substr(1), variables);
//     }
//
//     if (value.starts_with("!")) {
//         return "!(" + convert_value(value.substr(1), variables) + ")";
//     }
//
//     std::smatch match;
//
//     const std::regex variable_regex ("^_\\d+$");
//     if (std::regex_match(value, match, variable_regex)) {
//         return "state." + value;
//     }
//
//     const std::regex string_const ("^const (\".*\")$");
//     if (std::regex_match(value, match, string_const)) {
//         return match[1].str();
//     }
//
//     const std::regex int_const ("^const (\\d+)_[ui]\\d+$");
//     if (std::regex_match(value, match, int_const)) {
//         return match[1].str();
//     }
//
//     const std::regex ok_result (R"(^Result::<\(\), ProgramError>::Ok\(.*\)$)");
//     if (std::regex_match(value, match, ok_result)) {
//         return "ok";
//     }
//
//     const std::regex sol_log (R"(^solana_program::log::sol_log\((.+)\)$)");
//     if (std::regex_match(value, match, sol_log)) {
//         return "print(" + convert_value(match[1].str(), variables) + ")";
//     }
//
//     const std::regex deref_copy (R"(^deref_copy \((.+)\)$)");
//     if (std::regex_match(value, match, deref_copy)) {
//         return convert_value(match[1].str(), variables);
//     }
//
//     const std::regex deref (R"(^<.+ as Deref>::deref\((.+)\)$)");
//     if (std::regex_match(value, match, deref)) {
//         return convert_value(match[1].str(), variables);
//     }
//
//     const std::regex tuple_indexer (R"(^\((.+)\.(\d+): .+\)$)");
//     if (std::regex_match(value, match, tuple_indexer)) {
//         return convert_value(match[1].str(), variables) + ".get" + match[2].str();
//     }
//
//     const std::regex move (R"(^move (.+)$)");
//     if (std::regex_match(value, match, move)) {
//         return convert_value(match[1].str(), variables);
//     }
//
//     const std::regex checked_add (R"(^CheckedAdd\((.+), (.+)\)$)");
//     if (std::regex_match(value, match, checked_add)) {
//         std::string variable_type;
//         for (const auto& variable: variables) {
//             std::string var_name = variable.get_ast_data().at("variable");
//             if (var_name == match[1].str() || var_name == match[2].str()) {
//                 variable_type = variable.get_ast_data().at("variable_type");
//                 break;
//             }
//         }
//
//         for (char& c : variable_type) {
//             c = static_cast<char>(toupper(c));
//         }
//
//         std::string func = "addition(" + convert_value(match[1].str(), variables) + ", " + convert_value(match[2].str(), variables) + ", MAX_" + variable_type;
//
//         if (variable_type.starts_with("U")) {
//             return "u_" + func + ")";
//         }
//         if (variable_type.starts_with("I")) {
//             return "i_" + func + ", MIN_" + variable_type + ")";
//         }
//         return "unsupported_" + func + ", MIN_" + variable_type + ")";
//         std::throw_with_nested(std::runtime_error("Unsupported addition"));
//     }
//
//     const std::regex checked_mul (R"(^CheckedMul\((.+), (.+)\)$)");
//     if (std::regex_match(value, match, checked_mul)) {
//         std::string variable_type;
//         for (const auto& variable: variables) {
//             std::string var_name = variable.get_ast_data().at("variable");
//             if (var_name == match[1].str() || var_name == match[2].str()) {
//                 variable_type = variable.get_ast_data().at("variable_type");
//                 break;
//             }
//         }
//
//         for (char& c : variable_type) {
//             c = static_cast<char>(toupper(c));
//         }
//
//         std::string func = "multiplication(" + convert_value(match[1].str(), variables) + ", " + convert_value(match[2].str(), variables) + ", MAX_" + variable_type;
//
//         if (variable_type.starts_with("U")) {
//             return "u_" + func + ")";
//         }
//         if (variable_type.starts_with("I")) {
//             return "i_" + func + ", MIN_" + variable_type + ")";
//         }
//         std::throw_with_nested(std::runtime_error("Unsupported multiplication"));
//     }
//
//     const std::regex custom_function (R"(^(.+)\((.*)\)$)");
//     if (std::regex_match(value, match, custom_function)) {
//         const std::list<std::string> listed_types = utils::split(match[2].str(), ", ");
//         std::list<std::string> converted_types;
//         for (const auto& raw_type: listed_types) {
//             converted_types.push_back(convert_value(raw_type, variables));
//         }
//         return match[1].str() + "(" + utils::join(converted_types, ", ") + ")";
//     }
//
//     return value;
// }


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

std::optional<mir_statement> mir_statement::get_statement(const mir_statements &variables, const std::string &name) {
    for (const auto& variable: variables) {
        std::string var_name = variable.get_ast_data().at("variable");
        if (var_name == name) {
            return variable;
        }
    }
    return std::nullopt;
}














#ifndef MIR_STATEMENT_H
#define MIR_STATEMENT_H

#include <map>
#include <list>
#include <nlohmann/json.hpp>

class mir_statement;
typedef std::list<mir_statement> mir_statements;
enum class statement_type { unknown, root, function, block, assignment, variable, parameter, return_type, branch, data_struct, debug, data_enum, data_enum_struct, data_enum_option, maths };

class mir_statement {
public:
    mir_statement(statement_type type, nlohmann::json data);
    static mir_statement extract_node(const mir_statement& statement);
    static mir_statement new_variable(const std::string& variable, const std::string& variable_type);

    [[nodiscard]] statement_type get_type() const;
    [[nodiscard]] std::string get_string_type() const;
    [[nodiscard]] nlohmann::json get_ast() const;
    [[nodiscard]] nlohmann::json get_ast_data() const;
    [[nodiscard]] mir_statements get_children();
    [[nodiscard]] mir_statements get_children(std::initializer_list<statement_type> types);

    void update_ast_data(const std::string &key, const nlohmann::basic_json<>& new_value);

    void add_child(const mir_statement& child);
    void print(int indent_level = 0);

    static std::pair<std::string, std::string> get_argument_pair(const std::string &raw);
    static std::optional<mir_statement> get_statement(const mir_statements& variables, const std::string& name);

    static std::optional<mir_statement> parse_function(std::list<std::string> lines, const mir_statements& structs);
    static std::optional<mir_statement> parse_function_header(const std::string& line);
    static mir_statement parse_maths(const std::string& line);

    static mir_statement create_root(const std::string &contract_name);
    static bool line_is_statement_start(const std::string &line);
    static bool line_is_function(const std::string &line);
    static bool line_is_block(const std::string& line);
    static std::string convert_type(const std::string& type);

    static mir_statements get_all_variables(mir_statement function_header, const mir_statements& structs, const mir_statements& functions);
    static std::string reformat_value_by_type(const std::string& value, const std::string& type);

private:
    statement_type _type;
    nlohmann::json _ast_tree;

    static mir_statement parse_json(const nlohmann::json &json);
    static mir_statement parse_block(std::list<std::string> lines, const mir_statements &variables);
    static mir_statement parse_block_header(const std::string& line);
    static std::optional<mir_statements> parse_assignment(const std::string& line, const mir_statements& variables);
    static mir_statement parse_variable(const std::string& line);
    static std::optional<mir_statement> parse_debug(const std::string& line);
    static std::optional<mir_statement> parse_branch(const std::string& line);

    static std::tuple<std::string, bool> convert_value(const std::string& value, const mir_statements& variables);

    static mir_statements get_subvariables(const mir_statement& variable, const mir_statements& structs);
};

[[nodiscard]] nlohmann::json extract_data(nlohmann::json json);

#endif //MIR_STATEMENT_H

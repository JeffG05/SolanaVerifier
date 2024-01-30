#ifndef MIR_STATEMENT_H
#define MIR_STATEMENT_H

#include <list>
#include<nlohmann/json.hpp>

enum statement_type { unknown, root, function, block, assignment, variable, parameter, return_type, branch };

class mir_statement {
public:
    mir_statement(statement_type type, nlohmann::json data);

    [[nodiscard]] statement_type get_type() const;
    [[nodiscard]] std::string get_string_type() const;
    [[nodiscard]] nlohmann::json get_ast() const;
    [[nodiscard]] nlohmann::json get_ast_data() const;
    [[nodiscard]] std::list<mir_statement> get_children();
    [[nodiscard]] std::list<mir_statement> get_children(std::initializer_list<statement_type> types);

    void add_child(const mir_statement& child);

    void print(int indent_level = 0);

    static mir_statement parse_json(const nlohmann::json &json);
    static std::optional<mir_statement> parse_lines(const std::list<std::string> &lines, const std::list<mir_statement>& variables = {});
    static mir_statement parse_function(std::list<std::string> lines);
    static mir_statement parse_function_header(const std::string& line);
    static std::optional<mir_statement> parse_block(std::list<std::string> lines, const std::list<mir_statement> &variables);
    static mir_statement parse_block_header(const std::string& line);
    static std::optional<mir_statement> parse_assignment(const std::string& line, const std::list<mir_statement>& variables);
    static mir_statement parse_variable(const std::string& line);
    static std::optional<mir_statement> parse_branch(const std::string& line);

    static std::string convert_type(const std::string& type);
    static std::string convert_value(const std::string& value, const std::list<mir_statement>& variables);

    static mir_statement create_root(const std::string &contract_name);

    static bool line_is_function(const std::string &line);
    static bool line_is_block(const std::string& line);
    static bool line_is_statement_start(const std::string &line);

protected:
    statement_type _type;
    nlohmann::json _ast_tree;
};

[[nodiscard]] nlohmann::json extract_data(nlohmann::json json);

#endif //MIR_STATEMENT_H

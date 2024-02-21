#ifndef MIR_CONTRACT_H
#define MIR_CONTRACT_H

#include <set>
#include <string>
#include <filesystem>

#include "config.h"
#include "mir_statement.h"
#include "c_program.h"

typedef std::unordered_map<std::string, std::list<std::string>> reference_map;

class mir_contract {
public:
    mir_contract(const std::string &contract_name, const std::filesystem::path& path, const mir_statements &structs, config globals);
    mir_contract(const std::string &contract_name, const std::string& path, const mir_statements &structs, config globals);

    [[nodiscard]] std::string get_path() const;

    [[nodiscard]] c_program convert_to_c(const std::filesystem::path& target, const std::string& target_funtion);
    std::filesystem::path export_c_program(const std::filesystem::path& target, mir_statement ast_tree, const std::string& target_function);
    void export_library_file(const std::filesystem::path& target) const;


private:
    std::string _contract_name;
    std::filesystem::path _path;
    mir_statements _structs;
    config _globals{};

    mir_statement create_ast_tree(std::istream& file) const;
    static void generate_verification_statements(std::ostream* out, const mir_statement& target_function) ;

    [[nodiscard]] static std::string get_c_subtype(const std::string& type);
    [[nodiscard]] std::string get_c_type(const std::string& type, const std::string& name, const std::string& function_name) const;
    [[nodiscard]] std::string get_return_c_type(const std::string& type, const std::string& function_name) const;
    [[nodiscard]] static std::string get_c_value(const std::string& value);

    static std::string get_tuple_name(const std::string& type, const std::string& function_name);
    static std::string get_result_name(const std::string& type, const std::string& function_name);
    static std::string get_controlflow_name(const std::string& type, const std::string& function_name);

    void generate_structs(std::ostream* out, const mir_statements &state_statements,  const std::string& function_name);
    void generate_struct(std::ostream* out, const std::string& type, const std::string& function_name, std::set<std::string>* generated_structs);
    void generate_result_struct(std::ostream* out, const std::string& type, const std::string& function_name, std::set<std::string>* generated_structs);
    void generate_tuple_struct(std::ostream* out, const std::string& type, const std::string &function_name, std::set<std::string>* generated_structs);
    void generate_controlflow_struct(std::ostream* out, const std::string& type, const std::string &function_name, std::set<std::string>* generated_structs);
    void generate_function_struct(std::ostream* out, const mir_statements &state_statements, const std::string& function_name) const;
    void generate_struct_struct(std::ostream* out, const mir_statements &state_statements, const std::string& struct_name) const;

    void generate_block_function(std::ostream* out, mir_statement block_statement, const std::string& function_name, reference_map* references);
    void generate_block_statement(std::ostream* out, mir_statement statement, const std::string& function_name, reference_map* references);
    void generate_block_assignment(std::ostream* out, const std::string &variable, const std::string &value, bool returns);
    static void generate_reference_assignments(std::ostream* out, const std::string &variable, const reference_map &references);
    static void generate_branch(std::ostream* out, const mir_statement &branch_statement, const std::string& function_name, const std::string& variable, const std::string& value);
    void generate_function(std::ostream* out, const mir_statements &state_statements, const std::string& function_name, const std::string& function_return) const;
    void generate_main_function(std::ostream* out, const mir_statements &function_statements, const std::string& target_function_name) const;

    void generate_nondet(std::ostream* out, const mir_statement& statement, const std::string& function_name, bool in_main = false) const;
    void generate_nondet_array(std::ostream* out, const mir_statement& statement, bool in_main = false) const;

    static void generate_serialization(std::ostream* out, const std::string &variable_name, const std::string& variable_type, unsigned int* counter);
    static void generate_deserialization(std::ostream* out, const std::string &variable_name, const std::string& variable_type, unsigned int* counter);

    static mir_statement get_target_function(mir_statements function_statements, const std::string &function_name);
    static std::string trim_line(const std::string& line);
    static std::string clean_type(const std::string &type);
};

#endif //MIR_CONTRACT_H

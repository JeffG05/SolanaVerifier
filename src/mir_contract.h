#ifndef MIR_CONTRACT_H
#define MIR_CONTRACT_H

#include <string>
#include <filesystem>
#include "mir_statement.h"
#include "c_program.h"

class mir_contract {
public:
    mir_contract(const std::string &contract_name, const std::filesystem::path& path);
    mir_contract(const std::string &contract_name, const std::string& path);

    [[nodiscard]] std::string get_path() const;

    [[nodiscard]] c_program convert_to_c(const std::filesystem::path& target, const std::string& target_funtion) const;

    static std::filesystem::path export_c_program(const std::filesystem::path& target, mir_statement ast_tree, const std::string& target_function);
    static std::filesystem::path export_library_file(const std::filesystem::path& target);

    mir_statement create_ast_tree(std::istream& file) const;
    [[nodiscard]] static std::string ast_variable_to_c(const std::string& type);
    [[nodiscard]] static std::string ast_variable_to_c(const std::string& type, const std::string& name);
    static std::string trim_line(const std::string& line);


protected:
    std::string _contract_name;
    std::filesystem::path _path;
};

#endif //MIR_CONTRACT_H

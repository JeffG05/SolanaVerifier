#ifndef C_PROGRAM_H
#define C_PROGRAM_H

#include <filesystem>

class c_program {
public:
    explicit c_program(const std::filesystem::path& path);
    explicit c_program(const std::string& path);

    [[nodiscard]] std::string get_path() const;

protected:
    std::filesystem::path _path;
};

#endif //C_PROGRAM_H

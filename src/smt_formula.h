#ifndef SMT_FORMULA_H
#define SMT_FORMULA_H

#include <filesystem>

class smt_formula {
public:
    explicit smt_formula(const std::filesystem::path &path);
    explicit smt_formula(const std::string &path);

private:
    std::filesystem::path _path;

};



#endif //SMT_FORMULA_H

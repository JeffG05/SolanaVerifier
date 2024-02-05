#ifndef MIR_VALUE_CONVERTER_H
#define MIR_VALUE_CONVERTER_H

#include<string>
#include "mir_statement.h"
#include "mir_value.h"

class mir_value_converter {
public:
    static std::tuple<std::string, bool, std::string, std::string> convert(const std::string& mir, const std::list<mir_statement>& variables);
private:
    static std::vector<mir_value> _all_values;
};

#endif //MIR_VALUE_CONVERTER_H

#ifndef CONST_NUMBER_MIN_MIR_VALUE_H
#define CONST_NUMBER_MIN_MIR_VALUE_H

#include "mir-values/mir_value.h"
#include "mir-values/mir_value_converter.h"

class const_number_min_mir_value : public mir_value {
public:
    const_number_min_mir_value() : mir_value(
        std::regex (R"(^const ([ui](?:8|16|32|64|size))::MIN$)"),
        [](const std::smatch &match, const std::list<mir_statement>& variables) {
            std::string type = match[1].str();
            for (char& c : type) {
                c = static_cast<char>(toupper(c));
            }
            return std::make_tuple("MIN_" + type, true, "", "");
        }
    ) {}
};

#endif //CONST_NUMBER_MIN_MIR_VALUE_H

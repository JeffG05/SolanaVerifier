#ifndef TUPLE_MIR_TYPE_H
#define TUPLE_MIR_TYPE_H

#include <sstream>
#include "mir-types/mir_type.h"

class tuple_mir_type : public mir_type {
public:
    tuple_mir_type() : mir_type(
        std::regex (R"(^\((.+)(?:, (.+))*\)$)"),
        [](const std::smatch &match) {
            std::stringstream str;
            str << "tuple<";
            for (int i = 1; i < match.size() - 1; i++) {
                if (i > 1) {
                    str << ", ";
                }
                str << match[i].str();
            }
            str << ">";
            return str.str();
        }
    ) {}
};

#endif //TUPLE_MIR_TYPE_H

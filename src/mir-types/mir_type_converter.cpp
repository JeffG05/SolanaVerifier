#include "mir_type_converter.h"

#include "mir-types/mir_type.h"
#include "types/account_info_mir_type.h"
#include "types/array_mir_type.h"
#include "types/backtick_mir_type.h"
#include "types/borrow_mir_type.h"
#include "types/int_mir_type.h"
#include "types/pubkey_mir_type.h"
#include "types/result_mir_type.h"
#include "types/string_mir_type.h"
#include "types/tuple_mir_type.h"
#include "types/uint_mir_type.h"
#include "types/void_mir_type.h"

std::string mir_type_converter::convert(const std::string& mir) {
    for (const auto& type: _all_types) {
        if (auto parsed = type.try_parse(mir); parsed.has_value()) {
            return parsed.value();
        }
    }
    return mir;
}

std::vector<mir_type> mir_type_converter::_all_types = {
    uint_mir_type(),
    int_mir_type(),
    borrow_mir_type(),
    pubkey_mir_type(),
    array_mir_type(),
    account_info_mir_type(),
    result_mir_type(),
    void_mir_type(),
    tuple_mir_type(),
    backtick_mir_type(),
    string_mir_type()
};
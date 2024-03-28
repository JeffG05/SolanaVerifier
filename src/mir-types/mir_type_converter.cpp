#include "mir_type_converter.h"
#include "mir-types/mir_type.h"
#include "types/account_info_mir_type.h"
#include "types/account_meta_mir_type.h"
#include "types/argument_mir_type.h"
#include "types/arguments_mir_type.h"
#include "types/array_mir_type.h"
#include "types/backtick_mir_type.h"
#include "types/borrow_mir_type.h"
#include "types/box_mir_type.h"
#include "types/const_mir_type.h"
#include "types/control_flow_mir_type.h"
#include "types/deref_mir_type.h"
#include "types/error_mir_type.h"
#include "types/float_mir_type.h"
#include "types/infallible_mir_type.h"
#include "types/int_mir_type.h"
#include "types/iter_mir_type.h"
#include "types/mutable_mir_type.h"
#include "types/optional_mir_type.h"
#include "types/panic_assert_kind_mir_type.h"
#include "types/pubkey_mir_type.h"
#include "types/rc_mir_type.h"
#include "types/ref_cell_mir_type.h"
#include "types/ref_mut_mir_type.h"
#include "types/ref_mir_type.h"
#include "types/rent_mir_type.h"
#include "types/result_mir_type.h"
#include "types/solana_instruction_mir_type.h"
#include "types/string_mir_type.h"
#include "types/tuple_mir_type.h"
#include "types/uint_mir_type.h"
#include "types/vec_mir_type.h"
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
    string_mir_type(),
    mutable_mir_type(),
    ref_mir_type(),
    ref_cell_mir_type(),
    ref_mut_mir_type(),
    rc_mir_type(),
    infallible_mir_type(),
    error_mir_type(),
    control_flow_mir_type(),
    iter_mir_type(),
    vec_mir_type(),
    account_meta_mir_type(),
    solana_instruction_mir_type(),
    deref_mir_type(),
    box_mir_type(),
    const_mir_type(),
    panic_assert_kind_mir_type(),
    optional_mir_type(),
    arguments_mir_type(),
    float_mir_type(),
    rent_mir_type(),
    argument_mir_type()
};
include(FetchContent)
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

include(CMakeRC.cmake)
cmrc_add_resource_library(SolanaVerifierAssets assets/solana.c)

find_package(Boost 1.74 COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(SolanaVerifier
        main.cpp
        solana_contract.cpp
        mir_contract.cpp
        solana_contract.h
        mir_contract.h
        mir_statement.cpp
        mir_statement.h
        utils.cpp
        utils.h
        c_program.cpp
        c_program.h
        controller.cpp
        controller.h
        smt_formula.cpp
        smt_formula.h
        verification_result.cpp
        verification_result.h
        mir-types/mir_type.cpp
        mir-types/mir_type.h
        mir-types/mir_type_converter.cpp
        mir-types/mir_type_converter.h
        mir-types/types/borrow_mir_type.h
        mir-types/types/uint_mir_type.h
        mir-types/types/int_mir_type.h
        mir-types/types/pubkey_mir_type.h
        mir-types/types/array_mir_type.h
        mir-types/types/account_info_mir_type.h
        mir-types/types/result_mir_type.h
        mir-types/types/void_mir_type.h
        mir-types/types/tuple_mir_type.h
        mir-values/mir_value.cpp
        mir-values/mir_value.h
        mir-values/mir_value_converter.cpp
        mir-values/mir_value_converter.h
        mir-values/values/ok_result_mir_value.h
        mir-values/values/const_number_mir_value.h
        mir-values/values/const_void_mir_value.h
        mir-values/values/checked_mul_mir_value.h
        mir-values/values/checked_add_mir_value.h
        mir-values/values/variable_mir_value.h
        mir-values/values/move_mir_value.h
        mir-values/values/tuple_indexer_mir_value.h
        mir-values/values/tuple_indexer_mir_value.h
        mir-values/values/negation_mir_value.h
        mir-values/values/function_mir_value.h
        mir-types/types/backtick_mir_type.h
        mir-types/types/string_mir_type.h
        mir-values/values/const_string_mir_value.h
        mir-values/values/print_mir_value.h
        mir-types/types/mutable_mir_type.h
        mir-types/types/ref_mir_type.h
        mir-types/types/ref_cell_mir_type.h
        mir-types/types/ref_mut_mir_type.h
        mir-types/types/rc_mir_type.h
        mir-types/types/infallible_mir_type.h
        mir-types/types/error_mir_type.h
        mir-types/types/control_flow_mir_type.h
        mir-types/types/iter_mir_type.h
)

target_link_libraries(SolanaVerifier
        PRIVATE tomlplusplus::tomlplusplus
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE SolanaVerifierAssets
        PRIVATE ${Boost_LIBRARIES}
)

target_include_directories(SolanaVerifier
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
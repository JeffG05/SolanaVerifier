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

add_executable(SolanaVerifier main.cpp solana_contract.cpp mir_contract.cpp solana_contract.h mir_contract.h
        mir_statement.cpp
        mir_statement.h
        utils.cpp
        utils.h)

target_link_libraries(SolanaVerifier
        PRIVATE tomlplusplus::tomlplusplus
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE SolanaVerifierAssets
)

target_include_directories(SolanaVerifier
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
cmake_minimum_required(VERSION 3.22)
project(SolanaVerifier VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${SolanaVerifier_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SolanaVerifier_VERSION_MINOR}")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)
cmake_minimum_required (VERSION 3.15)

project (installer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

# Add source to this project's executable.
aux_source_directory(. SRC_LIST)
aux_source_directory(./backend BACKEND_LIST)
find_package(OpenSSL REQUIRED) # statically linking openssl requires crypt32.lib
find_package(ZLIB REQUIRED)
find_package(lz4 REQUIRED)

add_library(backend STATIC ${BACKEND_LIST})
target_link_libraries(backend PRIVATE Crypt32 OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB lz4::lz4)

add_definitions(-D_AFXDLL)
set(CMAKE_MFC_FLAG 1)
add_executable (installer WIN32 ${SRC_LIST} "installerApp.rc" "app.manifest")
target_link_libraries(installer PRIVATE backend)
target_link_options(installer PRIVATE "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'")
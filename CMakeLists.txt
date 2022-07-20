cmake_minimum_required(VERSION 3.0)
project(mod_yandex_transcribe)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SW REQUIRED freeswitch)

add_library(mod_yandex_transcribe SHARED src/mod_yandex_transcribe.c)

set_target_properties(mod_yandex_transcribe
        PROPERTIES PREFIX "")

add_dependencies(mod_yandex_transcribe
        yandex_cloudapi_cpp)

target_sources(mod_yandex_transcribe
        PRIVATE ${yandex_grpc_BINARY_DIR})

target_include_directories(mod_yandex_transcribe
        PRIVATE ${SW_INCLUDE_DIRS}
        PRIVATE ${Protobuf_INCLUDE_DIRS}
        PRIVATE ${yandex_grpc_BINARY_DIR})

target_link_libraries(mod_yandex_transcribe
        ${SW_LINK_LIBRARIES})

target_link_directories(mod_yandex_transcribe
        PRIVATE ${SW_LIBRARY_DIRS})

target_compile_options(mod_yandex_transcribe
        PRIVATE ${SW_CFLAGS})
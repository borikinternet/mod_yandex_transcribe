cmake_minimum_required(VERSION 3.0)
project(mod_yandex_transcribe)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib)

find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SW REQUIRED freeswitch)

add_library(mod_yandex_transcribe SHARED src/mod_yandex_transcribe.c src/yandex_iam_token.cpp)

set_target_properties(mod_yandex_transcribe
        PROPERTIES PREFIX "")

target_include_directories(mod_yandex_transcribe PRIVATE
        include
        ${SW_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIRS}
        ${yandex_grpc_BINARY_DIR}
        ${jwt_cpp_SOURCE_DIR}/include
        ${nlohmann_json_SOURCE_DIR}/single_include
        ${hinnant_date_cpp_SOURCE_DIR}/include)

target_link_libraries(mod_yandex_transcribe
        yandex_cloudapi_obj
        ${SW_LINK_LIBRARIES})

target_link_directories(mod_yandex_transcribe PRIVATE
        ${SW_LIBRARY_DIRS})

target_compile_options(mod_yandex_transcribe PRIVATE
        ${SW_CFLAGS})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_executable(iamtoken_tests EXCLUDE_FROM_ALL)
target_sources(iamtoken_tests PRIVATE
        tests/iamtoken_tests.cpp
        src/yandex_iam_token.cpp)
target_include_directories(iamtoken_tests PRIVATE
        include
        ${Boost_INCLUDE_DIRS}
        ${SW_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIRS}
        ${yandex_grpc_BINARY_DIR}
        ${jwt_cpp_SOURCE_DIR}/include
        ${nlohmann_json_SOURCE_DIR}/single_include
        ${hinnant_date_cpp_SOURCE_DIR}/include)
target_link_directories(iamtoken_tests PRIVATE
        ${Boost_LIBRARY_DIR}
        ${SW_LIBRARY_DIRS})
target_link_libraries(iamtoken_tests
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${SW_LINK_LIBRARIES}
        curl
        crypto)
target_compile_options(iamtoken_tests PRIVATE
        ${SW_CFLAGS})

add_test(NAME iamtoken_tests COMMAND iamtoken_tests)

enable_testing()
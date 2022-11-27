cmake_minimum_required(VERSION 3.0)
project(mod_yandex_transcribe)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SW REQUIRED freeswitch)

add_library(mod_yandex_transcribe MODULE src/mod_yandex_transcribe.c src/YandexIamToken.cpp src/YandexSttSession.cpp)
set_target_properties(mod_yandex_transcribe
        PROPERTIES PREFIX "")
target_compile_options(mod_yandex_transcribe PRIVATE
        ${SW_CFLAGS}
        -fPIC)
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
        ${SW_LINK_LIBRARIES}
        ssl
        curl
        crypto
        yandex_cloudapi_obj
        gRPC::grpc++
#        gRPC::grpc++_reflection
        protobuf::libprotobuf)
target_link_options(mod_yandex_transcribe PRIVATE
        ${SW_LDFLAGS})
target_link_directories(mod_yandex_transcribe PRIVATE
        ${SW_LIBRARY_DIRS})
install(TARGETS mod_yandex_transcribe
        LIBRARY
        DESTINATION /usr/lib64/freeswitch/mod)
install(FILES conf/yandex-transcribe.conf.xml
        DESTINATION /etc/freeswitch/autoload_configs)

add_library(mod_vosk SHARED EXCLUDE_FROM_ALL examples/mod_vosk.c)
set_target_properties(mod_vosk
        PROPERTIES PREFIX "")
target_include_directories(mod_vosk PRIVATE
        ${SW_INCLUDE_DIRS}
        ${libks_SOURCE_DIR}/src/include)
target_link_libraries(mod_vosk
        ks
        ${SW_LINK_LIBRARIES})
target_link_directories(mod_vosk PRIVATE
        ${SW_LIBRARY_DIRS})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

configure_file(tests/config.h.in tests/config.h)

add_executable(iamtoken_tests EXCLUDE_FROM_ALL tests/iamtoken_tests.cpp)
target_sources(iamtoken_tests PRIVATE
        tests/iamtoken_tests.cpp
        src/YandexIamToken.cpp
        src/YandexSttSession.cpp)
target_include_directories(iamtoken_tests PRIVATE
        include
        ${Boost_INCLUDE_DIRS}
        ${SW_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIRS}
        ${yandex_grpc_BINARY_DIR}
        ${jwt_cpp_SOURCE_DIR}/include
        ${nlohmann_json_SOURCE_DIR}/single_include
        ${hinnant_date_cpp_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR})
target_link_directories(iamtoken_tests PRIVATE
        ${Boost_LIBRARY_DIR}
        ${SW_LIBRARY_DIRS})
target_link_libraries(iamtoken_tests
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${SW_LINK_LIBRARIES}
        ssl
        curl
        crypto
        yandex_cloudapi_obj
        gRPC::grpc++
        gRPC::grpc++_reflection
        protobuf::libprotobuf)
target_compile_options(iamtoken_tests PRIVATE
        ${SW_CFLAGS})

add_test(NAME iamtoken_tests COMMAND iamtoken_tests)

enable_testing()
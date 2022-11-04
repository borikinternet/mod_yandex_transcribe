cmake_minimum_required(VERSION 3.0)
project(mod_yandex_transcribe)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(NLOHMANN_JSON
        GIT_REPOSITORY https://github.com/nlohmann/json.git)
FetchContent_Declare(YANDEX_GRPC
        GIT_REPOSITORY https://github.com/borikinternet/yandex-cloudapi.git)
FetchContent_Declare(JWT_CPP
        GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git)
FetchContent_Declare(HINNANT_DATE_CPP
        GIT_REPOSITORY https://github.com/HowardHinnant/date.git
        GIT_TAG cac99da8dc88be719a728dc1b597b0ac307c1800)
FetchContent_Declare(libks
        GIT_REPOSITORY https://github.com/signalwire/libks.git)
FetchContent_MakeAvailable(YANDEX_GRPC JWT_CPP HINNANT_DATE_CPP NLOHMANN_JSON libks)

set(yandex_grpc_SOURCE_DIR ${yandex_grpc_SOURCE_DIR} PARENT_SCOPE)
set(yandex_grpc_BINARY_DIR ${yandex_grpc_BINARY_DIR} PARENT_SCOPE)
set(libks_SOURCE_DIR ${libks_SOURCE_DIR} PARENT_SCOPE)
set(libks_BINARY_DIR ${libks_BINARY_DIR} PARENT_SCOPE)
set(jwt_cpp_SOURCE_DIR ${jwt_cpp_SOURCE_DIR} PARENT_SCOPE)
set(hinnant_date_cpp_SOURCE_DIR ${hinnant_date_cpp_SOURCE_DIR} PARENT_SCOPE)
set(nlohmann_json_SOURCE_DIR ${nlohmann_json_SOURCE_DIR} PARENT_SCOPE)
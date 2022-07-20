cmake_minimum_required(VERSION 3.0)
project(yandex-cloudapi)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(YANDEX_GRPC
        GIT_REPOSITORY https://github.com/borikinternet/yandex-cloudapi.git)
FetchContent_MakeAvailable(YANDEX_GRPC)

set(yandex_grpc_SOURCE_DIR ${yandex_grpc_SOURCE_DIR} PARENT_SCOPE)
set(yandex_grpc_BINARY_DIR ${yandex_grpc_BINARY_DIR} PARENT_SCOPE)
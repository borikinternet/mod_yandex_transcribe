cmake_minimum_required(VERSION 3.0)
project(mod_yandex_transcribe)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(yandex-cloudapi)

set(yandex_grpc_SOURCE_DIR ${yandex_grpc_SOURCE_DIR} PARENT_SCOPE)
set(yandex_grpc_BINARY_DIR ${yandex_grpc_BINARY_DIR} PARENT_SCOPE)

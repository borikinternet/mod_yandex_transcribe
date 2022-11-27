cmake_minimum_required(VERSION 3.0)
project(mod_yandex_transcribe)

set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git)
FetchContent_Declare(yandex_grpc
        GIT_REPOSITORY https://github.com/borikinternet/yandex-cloudapi.git)
FetchContent_Declare(jwt_cpp
        GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git)
FetchContent_Declare(hinnant_date_cpp
        GIT_REPOSITORY https://github.com/HowardHinnant/date.git
        GIT_TAG cac99da8dc88be719a728dc1b597b0ac307c1800)
FetchContent_Declare(libks
        GIT_REPOSITORY https://github.com/signalwire/libks.git)
foreach(entry yandex_grpc jwt_cpp hinnant_date_cpp nlohmann_json libks)
    FetchContent_GetProperties(${entry})
    set(sd_ ${entry}_SOURCE_DIR)
    set(bd_ ${entry}_BINARY_DIR)
    if (NOT ${entry}_POPULATED)
        FetchContent_Populate(${entry})
        add_subdirectory(${${sd_}} ${${bd_}} EXCLUDE_FROM_ALL)
    endif ()
    set(${entry}_SOURCE_DIR ${${sd_}} PARENT_SCOPE)
    set(${entry}_BINARY_DIR ${${bd_}} PARENT_SCOPE)
endforeach()
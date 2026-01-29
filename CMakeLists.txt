cmake_minimum_required(VERSION 4.0)
project(uranus)

set(CMAKE_CXX_STANDARD 23)

if (MSVC)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")  # /MDd
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")       # /MD
    endif()
endif ()

if (WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
    option(USE_SOLUTION_FOLDERS ON)
endif ()

add_compile_definitions(ASIO_STANDALONE)
add_compile_definitions(ASIO_HAS_CO_AWAIT)

# Import Third Library

if (WIN32)
    set(THIRD_LIBRARY_DIR D:/library/install)
    set(CMAKE_INSTALL_PREFIX D:/library/install/uranus)
elseif (APPLE)
    set(THIRD_LIBRARY_DIR ~/Documents/library/install)
    set(CMAKE_INSTALL_PREFIX ~/Documents/library/install/uranus)
elseif (UNIX)
    set(THIRD_LIBRARY_DIR ~/Documents/library/install)
    set(CMAKE_INSTALL_PREFIX ~/Documents/library/install/uranus)
endif ()

#list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/spdlog)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/asio)
#list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/mimalloc)
#list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/nlohmann_json)
#list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/YAML_CPP)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/abseil-cpp)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/protobuf)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/zlib)
#list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/MONGO_CXX_DRIVER)

include(GenerateExportHeader)
include(FetchContent)

set(BUILD_TESTING OFF CACHE BOOL "Not build test" FORCE)
set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Build spdlog as SHARED library" FORCE)
set(YAML_BUILD_SHARED_LIBS ON CACHE BOOL "Build yaml-cpp as SHARED library" FORCE)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 0.8.0
)


FetchContent_Declare(
        mimalloc
        GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
        GIT_TAG v3.2.7
)

FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(yaml-cpp)
FetchContent_MakeAvailable(mimalloc)

find_package(asio CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)
find_package(OpenSSL COMPONENTS REQUIRED)

add_subdirectory(base-src)
add_subdirectory(actor-src)
add_subdirectory(modules/config-src)
add_subdirectory(modules/login-src)
add_subdirectory(modules/logger-src)
add_subdirectory(modules/database-src)
add_subdirectory(gameplay/protobuf)
add_subdirectory(gameplay/common)
add_subdirectory(uranus-src)

add_subdirectory(gameplay/player)
add_subdirectory(gameplay/friend)
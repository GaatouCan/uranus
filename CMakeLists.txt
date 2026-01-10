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

list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/spdlog)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/asio)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/mimalloc)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/nlohmann_json)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/YAML_CPP)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/protobuf)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/zlib)

include(GenerateExportHeader)

find_package(spdlog CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)
find_package(OpenSSL COMPONENTS REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)


add_subdirectory(base-src)
add_subdirectory(actor-src)
add_subdirectory(modules/config-src)
add_subdirectory(modules/login-src)
#add_subdirectory(protobuf)
#add_subdirectory(http-src)
add_subdirectory(uranus-src)
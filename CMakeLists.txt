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
set(THIRD_LIBRARY_DIR D:/library/install)
set(CMAKE_INSTALL_PREFIX D:/library/install/uranus)

list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/spdlog)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/asio)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/libmimalloc)
list(APPEND CMAKE_PREFIX_PATH ${THIRD_LIBRARY_DIR}/nlohmann_json)

find_package(spdlog CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)

# The Core Library
file(GLOB_RECURSE CORE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/core-src/*.h)
file(GLOB_RECURSE CORE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/core-src/*.cpp)
file(GLOB_RECURSE CORE_FILES  ${CORE_HEADER} ${CORE_SOURCE})


if (MSVC)
    foreach(file ${CORE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_executable(core main.cpp ${CORE_FILES})
target_compile_definitions(core PRIVATE URANUS_EXPORT)
target_link_libraries(core PUBLIC spdlog::spdlog)
target_link_libraries(core PUBLIC asio::asio)
target_link_libraries(core PUBLIC mimalloc)

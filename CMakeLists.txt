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

add_library(core SHARED ${CORE_FILES})
target_compile_definitions(core PRIVATE URANUS_EXPORT)
target_link_libraries(core PUBLIC spdlog::spdlog)
target_link_libraries(core PUBLIC asio::asio)
target_link_libraries(core PUBLIC mimalloc)
target_include_directories(core PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core-src>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(core PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(core PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

add_subdirectory(package)

# Uranus executable
file(GLOB_RECURSE URANUS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/uranus-src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/uranus-src/*.cpp)

if (MSVC)
    foreach(file ${URANUS_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_executable(uranus ${URANUS_FILES})
target_link_libraries(uranus PUBLIC core)
target_link_libraries(uranus PUBLIC package-impl)
target_include_directories(uranus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core-src)
target_include_directories(uranus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/package)

set_target_properties(uranus PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(uranus PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

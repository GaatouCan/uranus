file(GLOB_RECURSE BASE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${BASE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(base SHARED ${BASE_FILES})

generate_export_header(base
        EXPORT_MACRO_NAME BASE_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/base/base.export.h
)

target_compile_definitions(base PRIVATE base_EXPORTS)
target_compile_definitions(base PUBLIC URANUS_SSL)
target_include_directories(base
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/network
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/base
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_link_libraries(base PUBLIC mimalloc)
target_link_libraries(base PUBLIC asio::asio)
target_link_libraries(base PUBLIC spdlog::spdlog)
target_link_libraries(base PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(base PUBLIC yaml-cpp::yaml-cpp)
target_link_libraries(base PUBLIC OpenSSL::SSL OpenSSL::Crypto)

# Logger Level
target_compile_definitions(base PUBLIC SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG)

set_target_properties(base PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(base PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(base PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

file(GLOB_RECURSE CONFIG_MODULE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${CONFIG_MODULE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(config SHARED ${CONFIG_MODULE_FILES})

generate_export_header(config
        EXPORT_MACRO_NAME CONFIG_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/config/config.export.h
)

target_compile_definitions(config PRIVATE config_EXPORTS)
target_include_directories(config
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/config
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(config PUBLIC actor)
target_link_libraries(config PUBLIC yaml-cpp::yaml-cpp)
target_link_libraries(config PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(config PUBLIC spdlog::spdlog)

set_target_properties(config PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(config PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(config PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

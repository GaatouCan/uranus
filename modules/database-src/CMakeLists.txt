file(GLOB_RECURSE DATABASE_MODULE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${DATABASE_MODULE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "." "" GROUP "${PARENT_DIR}")
        string(REPLACE "." "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

find_package(mongocxx CONFIG REQUIRED)

add_library(database SHARED ${DATABASE_MODULE_FILES})

generate_export_header(database
        EXPORT_MACRO_NAME DATABASE_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/database/database.export.h
)

target_compile_definitions(database PRIVATE database_EXPORTS)
target_include_directories(database
        PRIVATE include/database
        PUBLIC include
)

target_link_libraries(database PUBLIC actor)
target_link_libraries(database PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(database PUBLIC mongo::mongocxx_shared)

set_target_properties(database PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(database PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(database PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
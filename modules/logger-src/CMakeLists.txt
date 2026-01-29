file(GLOB_RECURSE LOGGER_MODULE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${LOGGER_MODULE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "." "" GROUP "${PARENT_DIR}")
        string(REPLACE "." "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach ()
endif ()

add_library(logger SHARED ${LOGGER_MODULE_FILES})

generate_export_header(logger
        EXPORT_MACRO_NAME LOGGER_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/logger/logger.export.h
)

target_compile_definitions(logger PRIVATE logger_EXPORTS)
target_include_directories(logger
        PRIVATE include/logger
        PUBLIC include
)

target_link_libraries(logger PUBLIC actor)

set_target_properties(logger PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(logger PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(logger PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
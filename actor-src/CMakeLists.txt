file(GLOB_RECURSE PACKAGE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${PACKAGE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(actor SHARED ${PACKAGE_FILES})

generate_export_header(actor
        EXPORT_MACRO_NAME ACTOR_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/actor/actor.export.h
)

target_compile_definitions(actor PRIVATE actor_EXPORTS)
target_include_directories(actor
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/actor
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(actor PUBLIC base)

set_target_properties(actor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(actor PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(actor PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

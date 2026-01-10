file(GLOB_RECURSE LOGIN_AUTH_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${LOGIN_AUTH_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "." "" GROUP "${PARENT_DIR}")
        string(REPLACE "." "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(login SHARED ${LOGIN_AUTH_FILES})

generate_export_header(login
        EXPORT_MACRO_NAME LOGIN_API
        EXPORT_FILE_NAME include/login/login.export.h
)

target_compile_definitions(login PRIVATE login_EXPORTS)
target_include_directories(login
        PRIVATE include/login
        PUBLIC include
)

target_link_libraries(login PUBLIC actor)
target_link_libraries(login PUBLIC spdlog::spdlog)

set_target_properties(login PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(login PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(login PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

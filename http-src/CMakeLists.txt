file(GLOB_RECURSE HTTP_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

if (MSVC)
    foreach(file ${HTTP_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(http SHARED ${HTTP_FILES})


generate_export_header(http
        EXPORT_MACRO_NAME HTTP_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/http/http.export.h
)

target_compile_definitions(http PRIVATE http_EXPORTS)

target_include_directories(http
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/http
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(http PUBLIC base)
target_link_libraries(http PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(http PUBLIC spdlog::spdlog)

set_target_properties(http PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(http PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(http PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

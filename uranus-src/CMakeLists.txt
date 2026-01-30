find_package(Protobuf CONFIG REQUIRED)

file(GLOB_RECURSE URANUS_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

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

target_link_libraries(uranus PUBLIC common)
target_link_libraries(uranus PUBLIC config)
target_link_libraries(uranus PUBLIC login)
target_link_libraries(uranus PUBLIC logger)
target_link_libraries(uranus PUBLIC database)

target_include_directories(uranus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(uranus PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(uranus PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(uranus PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

add_custom_command(
        TARGET uranus
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:spdlog::spdlog>
        $<TARGET_FILE_DIR:uranus>
)

add_custom_command(
        TARGET uranus
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:yaml-cpp::yaml-cpp>
        $<TARGET_FILE_DIR:uranus>
)

add_custom_command(
        TARGET uranus
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:mimalloc>
        $<TARGET_FILE_DIR:uranus>
)

if (WIN32)
    FetchContent_GetProperties(mimalloc SOURCE_DIR MIMALLOC_SOURCE_DIR)
    add_custom_command(
            TARGET uranus
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MIMALLOC_SOURCE_DIR}/bin/mimalloc-redirect.dll"
            $<TARGET_FILE_DIR:uranus>
    )
endif ()
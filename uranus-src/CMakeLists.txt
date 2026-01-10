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

target_include_directories(uranus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(uranus PUBLIC base)
target_link_libraries(uranus PUBLIC actor)
target_link_libraries(uranus PUBLIC config)
target_link_libraries(uranus PUBLIC spdlog::spdlog)

#target_link_libraries(uranus PUBLIC pb_login)

#target_link_libraries(uranus PUBLIC protobuf::libprotobuf-lite)

set_target_properties(uranus PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(uranus PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(uranus PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

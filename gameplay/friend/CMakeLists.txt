file(GLOB_RECURSE FRIEND_SERVICE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${FRIEND_SERVICE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "." "" GROUP "${PARENT_DIR}")
        string(REPLACE "." "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(friend_service SHARED ${FRIEND_SERVICE_FILES})

target_include_directories(friend_service PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(friend_service PUBLIC common)
target_link_libraries(friend_service PUBLIC config)
target_link_libraries(friend_service PUBLIC login)
target_link_libraries(friend_service PUBLIC logger)
target_link_libraries(friend_service PUBLIC database)
target_link_libraries(friend_service PUBLIC proto_static)

set_target_properties(friend_service PROPERTIES OUTPUT_NAME "friend")
set_target_properties(friend_service PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/core)
set_target_properties(friend_service PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/core)
set_target_properties(friend_service PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/core/lib)
file(GLOB_RECURSE GAME_PLAYER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${GAME_PLAYER_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "." "" GROUP "${PARENT_DIR}")
        string(REPLACE "." "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(player SHARED ${GAME_PLAYER_FILES} ../ProtocolID.h)

target_include_directories(player PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(player PUBLIC actor)
target_link_libraries(player PUBLIC config)
target_link_libraries(player PUBLIC logger)
target_link_libraries(player PUBLIC database)
target_link_libraries(player PUBLIC proto_static)

set_target_properties(player PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/player)
set_target_properties(player PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/player)
set_target_properties(player PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/player/lib)
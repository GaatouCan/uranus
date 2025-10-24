file(GLOB_RECURSE GAME_PLAYER ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${GAME_PLAYER})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(game-player SHARED ${GAME_PLAYER})
target_link_libraries(game-player PUBLIC core)
target_compile_definitions(game-player PRIVATE URANUS_SERVICE)

set_target_properties(game-player PROPERTIES OUTPUT_NAME "player")
set_target_properties(game-player PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/player)
set_target_properties(game-player PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/player)
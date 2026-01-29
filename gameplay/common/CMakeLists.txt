file(GLOB_RECURSE GAMEPLAY_COMMON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

if (MSVC)
    foreach(file ${GAMEPLAY_COMMON_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "." "" GROUP "${PARENT_DIR}")
        string(REPLACE "." "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(common INTERFACE ${GAMEPLAY_COMMON_FILES})

target_link_libraries(common INTERFACE actor)
target_link_libraries(common INTERFACE nlohmann_json::nlohmann_json)

target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
find_package(OpenSSL COMPONENTS REQUIRED)

file(GLOB_RECURSE NETWORK_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${NETWORK_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(network-impl STATIC ${NETWORK_FILES})
target_link_libraries(network-impl PUBLIC core)
target_link_libraries(network-impl PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(network-impl PUBLIC ${CMAKE_SOURCE_DIR}/core-src)

set_target_properties(network-impl PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
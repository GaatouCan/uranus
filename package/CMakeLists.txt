find_package(OpenSSL COMPONENTS REQUIRED)

file(GLOB_RECURSE PACKAGE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (MSVC)
    foreach(file ${PACKAGE_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

add_library(package-impl STATIC ${PACKAGE_FILES})
target_link_libraries(package-impl PUBLIC core)
target_link_libraries(package-impl PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(package-impl PUBLIC ${CMAKE_SOURCE_DIR}/core-src)

set_target_properties(package-impl PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
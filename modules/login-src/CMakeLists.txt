file(GLOB_RECURSE LOGIN_AUTH_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

if (MSVC)
    foreach(file ${LOGIN_AUTH_FILES})
        get_filename_component(PARENT_DIR "${file}" DIRECTORY)
        string(REPLACE "." "" GROUP "${PARENT_DIR}")
        string(REPLACE "." "\\" GROUP "${GROUP}")
        set(GROUP "${GROUP}")
        source_group("${GROUP}" FILES "${file}")
    endforeach()
endif ()

# Login Protocol define
find_package(Protobuf CONFIG REQUIRED)

if (CMAKE_CROSSCOMPILING)
    find_program(PROTOBUF_PROTOC protoc)
else ()
    set(PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif ()

file(GLOB_RECURSE LOGIN_PROTO_LIST ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/def/*.proto)

foreach (proto ${LOGIN_PROTO_LIST})
    get_filename_component(proto_name ${proto} NAME_WE)
    set(proto_src ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/gen/${proto_name}.pb.cc)
    set(proto_hdr ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/gen/${proto_name}.pb.h)
    add_custom_command(
            OUTPUT ${proto_src} ${proto_hdr}
            COMMAND ${PROTOBUF_PROTOC}
            ARGS --cpp_out ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/gen -I ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/def ${proto}
            DEPENDS ${proto})

    list(APPEND LOGIN_GEN_FILES ${proto_hdr} ${proto_src})
endforeach ()

add_library(login SHARED ${LOGIN_AUTH_FILES} ${LOGIN_GEN_FILES})

generate_export_header(login
        EXPORT_MACRO_NAME LOGIN_API
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/login/login.export.h
)

target_compile_definitions(login PRIVATE login_EXPORTS)
target_compile_definitions(login PRIVATE ABSL_USES_STD_STRING_VIEW)
target_include_directories(login
        PRIVATE include/login
        PRIVATE protobuf/gen
        PUBLIC include
)

target_link_libraries(login PUBLIC actor)
target_link_libraries(login PUBLIC spdlog::spdlog)
target_link_libraries(login PUBLIC protobuf::libprotobuf-lite)

set_target_properties(login PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(login PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
set_target_properties(login PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

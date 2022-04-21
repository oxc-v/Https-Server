file(GLOB SOURCE_FILE "*.cpp" "*.hpp")
file(GLOB PROTO_FILE "*.proto")

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

add_executable(services ${SOURCE_FILE} ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(services PUBLIC 
    https_server
    ${Protobuf_LIBRARIES}
)

target_include_directories(services PUBLIC 
    ${Protobuf_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_BINARY_DIR}
)
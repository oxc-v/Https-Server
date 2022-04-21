# asio include directory
set(ASIO_INCLUDE_DIR "/home/oxc/code/third_library/asio/include")

# format include directory
set(FMT_INCLUDE_DIR "/home/oxc/code/third_library/fmt/include")

# format lib directory
set(FMT_LIB_DIR "/home/oxc/code/third_library/fmt/lib")

# brotli root directory
set(BROTLI_USE_STATIC_LIBS TRUE)
set(BROTLI_ROOT_DIR "/home/oxc/code/third_library/brotli")

file(GLOB SOURCE_FILE "*.cpp")
file(GLOB INCLUDE_FILE "*.hpp")
file(GLOB FMT_LIBRARYS ${FMT_LIB_DIR}/*.a)

find_package(ZLIB REQUIRED)
find_package(Brotli REQUIRED)

if (Brotli_FOUND AND ZLIB_FOUND)
    add_library(https_server STATIC ${SOURCE_FILE} ${INCLUDE_FILE})

    target_include_directories(https_server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories (https_server PUBLIC 
        ${Brotli_INCLUDE_DIR}
        ${ASIO_INCLUDE_DIR}
        ${FMT_INCLUDE_DIR}
    )
    target_link_libraries (https_server
        crypto ssl pthread
        Brotli::encoder Brotli::common 
        ZLIB::ZLIB
        ${FMT_LIBRARYS}
    )
else ()
    MESSAGE(FATAL_ERROR "Can't find Brotli or GZIP")
endif()
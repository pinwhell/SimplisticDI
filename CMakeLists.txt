cmake_minimum_required(VERSION 3.16)
project(sdi)
add_library(sdi INTERFACE)
target_include_directories(sdi INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_compile_features(sdi INTERFACE cxx_std_20)
add_subdirectory(tests)

install(TARGETS sdi
        EXPORT sdi-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(EXPORT sdi-targets
        FILE sdi-targets.cmake
        NAMESPACE sdi::
        DESTINATION lib/cmake/sdi)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include)

configure_file(cmake/sdi-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sdi-config.cmake
    @ONLY)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/sdi-config.cmake
    DESTINATION lib/cmake/sdi)
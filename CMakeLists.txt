cmake_minimum_required(VERSION 3.16)
project(simplistic-di)
add_library(di INTERFACE)
add_library(simplistic::di ALIAS di)
include_directories(include)
target_include_directories(di INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_compile_features(di INTERFACE cxx_std_20)
add_subdirectory(tests)

install(TARGETS di
        EXPORT simplistic-di-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)

install(EXPORT simplistic-di-targets
        FILE simplistic-di-targets.cmake
        NAMESPACE simplistic::
        DESTINATION lib/cmake/simplistic)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include)

configure_file(cmake/simplistic-di-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/simplistic-di-config.cmake
    @ONLY)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/simplistic-di-config.cmake
    DESTINATION lib/cmake/simplistic)
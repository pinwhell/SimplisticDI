add_executable(playground playground.cpp)
target_link_libraries(playground simplistic::di)

add_executable(tests tests.cpp)
target_link_libraries(tests simplistic::di)
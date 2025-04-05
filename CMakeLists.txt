cmake_minimum_required(VERSION 3.20)
project(KlientV2 C)

set(CMAKE_C_STANDARD 99)
add_link_options(-pthread)
add_link_options(-lrt)
add_link_options(-lncurses)
add_executable(client client.c)
target_link_libraries(client -lncurses -pthread -lrt)

cmake_minimum_required(VERSION 3.16)

project(proximal_interpolation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -Ofast -flto=auto -march=native")

set(CMAKE_BUILD_TYPE Release)

include_directories("../json/include")

add_compile_definitions(ZERO_DISTANCE_HANDLING)

add_executable(proximal_interpolation main.cpp
    helper_funcs.h
    type_cast.h
    utils.h
    tools.h
    debug.h
    tests.h
    config.h
    config.cpp
    point.h
    kdtree.h
)

include(GNUInstallDirs)
install(TARGETS proximal_interpolation
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

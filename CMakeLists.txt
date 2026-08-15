cmake_minimum_required(VERSION 3.28)

if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum macOS deployment target" FORCE)
endif()

project(HandmadeHero LANGUAGES C CXX OBJC OBJCXX)

add_subdirectory(Game)
add_subdirectory("Platform macOS")

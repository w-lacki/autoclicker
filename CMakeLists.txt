cmake_minimum_required(VERSION 3.29)
project(clicker)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static -lpthread")

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_executable(clicker
        src/utils.h
        src/utils.cpp
        src/main.cpp
        src/recorder.h
        src/recorder.cpp
        src/clicker.cpp
        src/clicker.h
        src/hooks.h

)
target_link_libraries(clicker PRIVATE nlohmann_json::nlohmann_json)

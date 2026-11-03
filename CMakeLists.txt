cmake_minimum_required(VERSION 3.20)
project(SmartParking CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/utf-8 /W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_library(parking_core STATIC
    src/Car.cpp
    src/ParkingSpot.cpp
    src/ParkingLot.cpp
    src/Strategy.cpp
    src/Config.cpp
    src/Simulation.cpp)
target_include_directories(parking_core PUBLIC src)

add_executable(tests tests/tests.cpp)
target_link_libraries(tests PRIVATE parking_core)

add_executable(bench tests/bench.cpp)
target_link_libraries(bench PRIVATE parking_core)

enable_testing()
add_test(NAME tests COMMAND tests)

# Графикалық бөлік SFML 3 табылса ғана жиналады
find_package(SFML 3 COMPONENTS Graphics QUIET)
if(SFML_FOUND)
    add_executable(parking src/main.cpp)
    target_link_libraries(parking PRIVATE parking_core SFML::Graphics)
else()
    message(WARNING "SFML 3 табылмады - тек tests және bench жиналады")
endif()

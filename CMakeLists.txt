cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(PackageDownloader)

set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt5 REQUIRED COMPONENTS Core Network)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB CORE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
)
file(GLOB APP_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/Application/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Application/*.cpp"
)

add_executable(${PROJECT_NAME} ${CORE_SOURCES} ${APP_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Network)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

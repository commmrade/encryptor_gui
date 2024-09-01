cmake_minimum_required(VERSION 3.5.0)
project(proj VERSION 0.1.0 LANGUAGES C CXX)


find_package(PkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})



add_executable(proj main.cpp App.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall -Wextra -Wpedantic
)

target_link_libraries(proj ${GTKMM_LIBRARIES})

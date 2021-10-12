cmake_minimum_required(VERSION 3.14)
project(CfbSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  cfbSimTests
  tests/testMain.cpp
)
target_link_libraries(
  cfbSimTests
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(cfbSimTests)

add_executable(main main.cpp)

add_custom_command(TARGET main POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:main>/data)
                   
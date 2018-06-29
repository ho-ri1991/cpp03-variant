cmake_minimum_required(VERSION 3.11)
project(cpp03-variant)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
message(STATUS "${Boost_INCLUDE_DIRS}")

add_executable(cpp03-variant variant_test.cpp)

enable_testing()

add_executable(variant_test variant_test.cpp)
add_test(
        NAME variant_test
        COMMAND $<TARGET_FILE:variant_test>
)
set_property(TEST variant_test PROPERTY LABELS variant)


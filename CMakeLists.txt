# Adjust this!
# libc++ Path
set(LIBXX_DIR "F:/Paul/Computer/Programmieren/C++/llvm-mingw-20250212-ucrt-x86_64/x86_64-w64-mingw32")
# Tcl/Tk Path
set(TCLTK_DIR "F:/Paul/Computer/Programmieren/C++/tktcl")

cmake_minimum_required(VERSION 3.29.5 FATAL_ERROR)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
Set(FETCHCONTENT_QUIET FALSE)

# dont know why, vs generator complains about this
#if (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    set(CMAKE_CXX_MODULE_STD ON)
#endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/bigobj)
endif()

set(TCLTK_FULL_DIR "${TCLTK_DIR}/${CMAKE_CXX_COMPILER_ID}")

# Include and link
include_directories(${TCLTK_FULL_DIR}/include)
include_directories(../)
link_directories(${TCLTK_FULL_DIR}/lib)

include(FetchContent)

# include hhh
FetchContent_Declare(
    hhh
    GIT_REPOSITORY https://github.com/hanslhansl/hhh.git
    GIT_TAG "main"
    GIT_SHALLOW TRUE
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(hhh)

add_library(cpptkinter)
target_sources(cpptkinter PUBLIC FILE_SET CXX_MODULES FILES cpptkinter.ixx _cpptkinter.ixx utility.ixx constants.ixx)
target_link_libraries(cpptkinter hhh)
target_compile_features(cpptkinter PUBLIC cxx_std_23)

add_executable(test test.cpp)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/bigobj)
    target_link_libraries(test cpptkinter hhh tcl90 tcl9tk90)
else()
    target_link_libraries(test cpptkinter hhh libtcl90 libtcl9tk90)
endif()

add_custom_command(TARGET test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TCLTK_FULL_DIR}/bin/tcl90.dll"
        $<TARGET_FILE_DIR:test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TCLTK_FULL_DIR}/bin/tcl9tk90.dll"
        $<TARGET_FILE_DIR:test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TCLTK_FULL_DIR}/bin/libtommath.dll"
        $<TARGET_FILE_DIR:test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TCLTK_FULL_DIR}/bin/zlib1.dll"
        $<TARGET_FILE_DIR:test>
)

# copy libc++
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_custom_command(TARGET test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBXX_DIR}/bin/libc++.dll"
            $<TARGET_FILE_DIR:test>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBXX_DIR}/bin/libunwind.dll"
            $<TARGET_FILE_DIR:test>
    )
endif()
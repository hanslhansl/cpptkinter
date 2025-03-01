# Adjust this!
# libc++ Path
set(LIBXX_DIR "${CMAKE_SOURCE_DIR}/../llvm-mingw-20250212-ucrt-x86_64/x86_64-w64-mingw32")
# Tcl/Tk Path
set(TCLTK_DIR "${CMAKE_SOURCE_DIR}/../tktcl")

cmake_minimum_required(VERSION 3.29.5 FATAL_ERROR)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)
set(SRC_DIR "src")
set(TEST_DIR "tests")
set(TCLTK_FULL_DIR "${TCLTK_DIR}/${CMAKE_CXX_COMPILER_ID}")

# dont know why, vs generator complains about this
if (NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_CXX_MODULE_STD ON)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/bigobj)
endif()

# Include and link
include_directories(${TCLTK_FULL_DIR}/include)
include_directories(../)    # for reflect

# add hhh
add_subdirectory("${CMAKE_SOURCE_DIR}/../hhh" hhh)

# add tcl
add_library(tcl SHARED IMPORTED)
set_target_properties(tcl PROPERTIES
    IMPORTED_LOCATION "${TCLTK_FULL_DIR}/bin/tcl90.dll"
    IMPORTED_IMPLIB "${TCLTK_FULL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tcl90${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# add tk
add_library(tk SHARED IMPORTED)
set_target_properties(tk PROPERTIES
    IMPORTED_LOCATION "${TCLTK_FULL_DIR}/bin/tcl9tk90.dll"
    IMPORTED_IMPLIB "${TCLTK_FULL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tcl9tk90${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# add tommath
add_library(tommath SHARED IMPORTED)
set_target_properties(tommath PROPERTIES
    IMPORTED_LOCATION "${TCLTK_FULL_DIR}/bin/libtommath.dll"
    IMPORTED_IMPLIB "${TCLTK_FULL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}tommath${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# add zlib
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(ZLIB_LIB_NAME zdll)
else()
    set(ZLIB_LIB_NAME libz)
endif()
add_library(zlib SHARED IMPORTED)
set_target_properties(zlib PROPERTIES
    IMPORTED_LOCATION "${TCLTK_FULL_DIR}/bin/zlib1.dll"
    IMPORTED_IMPLIB "${TCLTK_FULL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${ZLIB_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# create cpptkinter lib
add_library(cpptkinter)
target_sources(cpptkinter 
    PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${SRC_DIR}
        FILES ${SRC_DIR}/cpptkinter.ixx ${SRC_DIR}/_cpptkinter.ixx ${SRC_DIR}/utility.ixx ${SRC_DIR}/constants.ixx
)
target_link_libraries(cpptkinter PUBLIC hhh tcl tk tommath zlib)

# create test exe
add_executable(test ${TEST_DIR}/test.cpp)
target_link_libraries(test PUBLIC cpptkinter hhh)

# copy dlls
if(CMAKE_IMPORT_LIBRARY_SUFFIX)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
  )
endif()

# copy libc++
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_custom_command(TARGET cpptkinter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBXX_DIR}/bin/libc++.dll"
            $<TARGET_FILE_DIR:cpptkinter>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LIBXX_DIR}/bin/libunwind.dll"
            $<TARGET_FILE_DIR:cpptkinter>
    )
endif()
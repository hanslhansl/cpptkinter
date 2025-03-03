# Adjust this!
# libc++ Path
set(LIBXX_DIR "${CMAKE_SOURCE_DIR}/../llvm-mingw-20250212-ucrt-x86_64/x86_64-w64-mingw32")
# Tcl/Tk Path
set(TCLTK_DIR "${CMAKE_SOURCE_DIR}/../tktcl")

cmake_minimum_required(VERSION 4.0.0 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
include(my_cmake_commands)
ENABLE_IMPORT_STD_MACRO()

project(test LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FETCHCONTENT_QUIET FALSE)
set(SRC_DIR "src")
set(TEST_DIR "tests")
set(TCLTK_FULL_DIR "${TCLTK_DIR}/${CMAKE_CXX_COMPILER_ID}")


# Include and link
include_directories(${TCLTK_FULL_DIR}/include)
include_directories(../)    # for reflect

#[[ Fetch range-v3
include(FetchContent)
FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG master  # Use the latest stable release or specify a commit hash
)
FetchContent_MakeAvailable(range-v3)]]

# add range-v3
add_subdirectory("${CMAKE_SOURCE_DIR}/../range-v3" range-v3)

# add hhh
add_subdirectory("${CMAKE_SOURCE_DIR}/../hhh" hhh)

# add tcl/tk
set(IMPORTED_SHARED_LIBRARY_NAMES "tcl90" "tcl9tk90" "libtommath" "zlib1")
set(IMPORTED_SHARED_LIBRARIES "")
foreach(lib IN LISTS IMPORTED_SHARED_LIBRARY_NAMES)
    add_library(${lib} SHARED IMPORTED)
    set_target_properties(${lib} PROPERTIES
        IMPORTED_LOCATION "${TCLTK_FULL_DIR}/bin/${lib}.dll"
        IMPORTED_IMPLIB "${TCLTK_FULL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    list(APPEND IMPORTED_SHARED_LIBRARIES ${lib})   # Store the target name for later use
endforeach()

# create cpptkinter lib
add_library(cpptkinter)
target_sources(cpptkinter 
    PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${SRC_DIR}
        FILES ${SRC_DIR}/cpptkinter.ixx ${SRC_DIR}/_cpptkinter.ixx ${SRC_DIR}/utility.ixx ${SRC_DIR}/constants.ixx ${SRC_DIR}/tk.ixx
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(cpptkinter PUBLIC "/translateInclude")
endif()
target_link_libraries(cpptkinter PUBLIC hhh range-v3 ${IMPORTED_SHARED_LIBRARIES})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/bigobj)
endif()

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
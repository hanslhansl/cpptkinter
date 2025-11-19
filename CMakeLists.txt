# Adjust this!
# Tcl/Tk Path
set(TCLTK_DIR "${CMAKE_SOURCE_DIR}/../tktcl")


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")
include(my_cmake_commands)
ENABLE_IMPORT_STD_MACRO()

cmake_minimum_required(VERSION 4.1.0 FATAL_ERROR)
project(cpptkinter LANGUAGES CXX)

set(SRC_DIR "src")
set(TEST_DIR "tests")
set(TCLTK_FULL_DIR "${TCLTK_DIR}/${CMAKE_CXX_COMPILER_ID}")

# Include
include_directories(${TCLTK_FULL_DIR}/include)

# fetch range-v3
include(FetchContent)
FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_TAG master
    GIT_PROGRESS ON
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(range-v3)

# fetch hhh
FetchContent_Declare(
    hhh
    GIT_REPOSITORY https://github.com/hanslhansl/hhh.git
    GIT_TAG main
    GIT_PROGRESS ON
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(hhh)

# add tcl/tk
set(IMPORTED_SHARED_LIBRARY_NAMES "tcl90" "tcl9tk90" "libtommath" "zlib1")
set(IMPORTED_SHARED_LIBRARIES "")
foreach(lib IN LISTS IMPORTED_SHARED_LIBRARY_NAMES)
    add_library(${lib} SHARED IMPORTED)
    set_target_properties(${lib} PROPERTIES
        IMPORTED_LOCATION "${TCLTK_FULL_DIR}/bin/${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}"
        IMPORTED_IMPLIB "${TCLTK_FULL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    list(APPEND IMPORTED_SHARED_LIBRARIES ${lib})   # Store the target name for later use
endforeach()

# create cpptkinter lib
add_library(cpptkinter)
target_sources(cpptkinter 
    PUBLIC FILE_SET CXX_MODULES
        BASE_DIRS ${SRC_DIR}
        FILES ${SRC_DIR}/constants.ixx ${SRC_DIR}/tk.ixx
        ${SRC_DIR}/utility/utility.detail.ixx ${SRC_DIR}/utility/utility.ixx
        ${SRC_DIR}/_cpptkinter/_cpptkinter.functions.ixx ${SRC_DIR}/_cpptkinter/_cpptkinter.tcl_obj.ixx ${SRC_DIR}/_cpptkinter/_cpptkinter.templates.ixx
        ${SRC_DIR}/_cpptkinter/_cpptkinter.tkappobj.ixx ${SRC_DIR}/_cpptkinter/_cpptkinter.ixx
        ${SRC_DIR}/cpptkinter/cpptkinter.detail.ixx ${SRC_DIR}/cpptkinter/cpptkinter.wm.ixx ${SRC_DIR}/cpptkinter/cpptkinter.misc.ixx ${SRC_DIR}/cpptkinter/cpptkinter1.ixx
        ${SRC_DIR}/cpptkinter/cpptkinter.cnfs.ixx ${SRC_DIR}/cpptkinter/cpptkinter.widget.base.ixx ${SRC_DIR}/cpptkinter/cpptkinter.widget1.ixx ${SRC_DIR}/cpptkinter/cpptkinter.widget2.ixx
        ${SRC_DIR}/ttk/ttk.cnfs.ixx ${SRC_DIR}/ttk/ttk.ixx
        ${SRC_DIR}/cpptkinter/cpptkinter.ixx 
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(cpptkinter PUBLIC "/translateInclude" "/bigobj")
endif()

target_link_libraries(cpptkinter PUBLIC hhh range-v3 ${IMPORTED_SHARED_LIBRARIES})

# create test exe
add_executable(test1 ${TEST_DIR}/test1.cpp)
target_link_libraries(test1 PUBLIC cpptkinter hhh)

# copy dlls
if(CMAKE_IMPORT_LIBRARY_SUFFIX)
    add_custom_command(TARGET test1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:test1> $<TARGET_FILE_DIR:test1>
        COMMAND_EXPAND_LISTS
  )
endif()
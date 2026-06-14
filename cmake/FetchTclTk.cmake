enable_language(C)

include(FetchContent)
include(ExternalProject)

option(USE_SYSTEM_TCL "Use system-installed Tcl/Tk instead of building" OFF)

# Build options
option(TCL_ENABLE_THREADS "Enable Tcl threads" ON)
option(TCL_ENABLE_SHARED  "Build Tcl/Tk as shared libraries" ON)


function(FetchTclTk)
    # --------------------------------------------------------------------------
    # 1. Use system Tcl/Tk
    # --------------------------------------------------------------------------
    if (USE_SYSTEM_TCL)
        find_package(TCL REQUIRED)
        find_package(Tk REQUIRED)
        return()
    endif()

    # --------------------------------------------------------------------------
    # 2. Fetch Tcl/Tk source (cached)
    #[[ --------------------------------------------------------------------------
    FetchContent_Declare(
        tcl_source
        GIT_REPOSITORY  https://github.com/tcltk/tcl.git
        GIT_TAG         main
        GIT_PROGRESS    ON
        GIT_SHALLOW     ON
    )

    FetchContent_Declare(
        tk_source
        GIT_REPOSITORY  https://github.com/tcltk/tk.git
        GIT_TAG         main
        GIT_PROGRESS    ON
        GIT_SHALLOW     ON
    )

    FetchContent_MakeAvailable(tcl_source tk_source)

    set(TCL_SRC "${tcl_source_SOURCE_DIR}")
    set(TK_SRC  "${tk_source_SOURCE_DIR}")

    set(TCL_INSTALL_DIR "${CMAKE_BINARY_DIR}/tcl-install")
    file(MAKE_DIRECTORY "${TCL_INSTALL_DIR}")]]

    # --------------------------------------------------------------------------
    # 3. Detect toolchain
    # --------------------------------------------------------------------------
    if (MSVC)
        set(TCL_MODE MSVC)
    elseif (MINGW)
        set(TCL_MODE MINGW)
    else()
        message(STATUS "Unsupported setup for building Tcl/Tk:")
    endif()

    message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
    message(STATUS "C Compiler ID: ${CMAKE_C_COMPILER_ID}")
    message(STATUS "C Compiler Version: ${CMAKE_C_COMPILER_VERSION}")
    message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
    message(STATUS "C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")

    message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")
    message(STATUS "Generator Platform: ${CMAKE_GENERATOR_PLATFORM}")
    message(STATUS "Generator Toolset: ${CMAKE_GENERATOR_TOOLSET}")

    message(STATUS "CYGWIN: ${CYGWIN}")
    message(STATUS "LINUX: ${LINUX}")
    message(STATUS "MSVC: ${MSVC}")
    message(STATUS "MINGW: ${MINGW}")
    message(STATUS "WIN32: ${WIN32}")
    message(STATUS "UNIX: ${UNIX}")
    message(STATUS "MSYS: ${MSYS}")
    message(FATAL_ERROR "end")

    # --------------------------------------------------------------------------
    # 4. Build Tcl/Tk using ExternalProject (cached)
    # --------------------------------------------------------------------------

    if (TCL_MODE STREQUAL "MSVC")

        ExternalProject_Add(
            tcl_project
            SOURCE_DIR "${TCL_SRC}/win"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND nmake -f makefile.vc
                OPTS=threads=${TCL_ENABLE_THREADS} shared=${TCL_ENABLE_SHARED}
            INSTALL_COMMAND ""
        )

        ExternalProject_Add(
            tk_project
            SOURCE_DIR "${TK_SRC}/win"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND nmake -f makefile.vc TCLDIR=${TCL_SRC}
                OPTS=threads=${TCL_ENABLE_THREADS} shared=${TCL_ENABLE_SHARED}
            INSTALL_COMMAND ""
            DEPENDS tcl_project
        )

        set(TCL_LIB "${TCL_SRC}/win/tcl90.lib")
        set(TK_LIB  "${TK_SRC}/win/tk90.lib")

        # DLLs on Windows (MSVC)
        set(TCL_RUNTIME_DLLS
            "${TCL_SRC}/win/tcl90.dll"
        )
        list(APPEND TCL_RUNTIME_DLLS "${TK_SRC}/win/tk90.dll")
        
        # Include paths (MSVC)
        set(TCL_INCLUDE_DIRS
            "${TCL_SRC}/generic"
            "${TCL_SRC}/win"
        )

    else()
        # Autoconf args
        set(AUTOCONF_OPTS "")
        if (TCL_ENABLE_THREADS)
            list(APPEND AUTOCONF_OPTS "--enable-threads")
        else()
            list(APPEND AUTOCONF_OPTS "--disable-threads")
        endif()

        if (TCL_ENABLE_SHARED)
            list(APPEND AUTOCONF_OPTS "--enable-shared")
        else()
            list(APPEND AUTOCONF_OPTS "--disable-shared")
        endif()

        ExternalProject_Add(
            tcl_project
            SOURCE_DIR "${TCL_SRC}/unix"
            CONFIGURE_COMMAND ./configure --prefix=${TCL_INSTALL_DIR} ${AUTOCONF_OPTS}
            BUILD_COMMAND make -j
            INSTALL_COMMAND make install
        )

        ExternalProject_Add(
            tk_project
            SOURCE_DIR "${TK_SRC}/unix"
            CONFIGURE_COMMAND ./configure --prefix=${TCL_INSTALL_DIR} --with-tcl=${TCL_INSTALL_DIR}/lib ${AUTOCONF_OPTS}
            BUILD_COMMAND make -j
            INSTALL_COMMAND make install
            DEPENDS tcl_project
        )

        # Libraries
        if (WIN32)
            set(TCL_LIB "${TCL_INSTALL_DIR}/lib/libtcl9.0.dll.a")
            set(TK_LIB  "${TCL_INSTALL_DIR}/lib/libtk9.0.dll.a")

            # Runtime DLLs (MinGW)
            file(GLOB TCL_DLLS "${TCL_INSTALL_DIR}/bin/*.dll")
            set(TCL_RUNTIME_DLLS ${TCL_DLLS})
        else()
            set(TCL_LIB "${TCL_INSTALL_DIR}/lib/libtcl9.0.so")
            set(TK_LIB  "${TCL_INSTALL_DIR}/lib/libtk9.0.so")
        endif()

        # Include dir for autoconf builds
        set(TCL_INCLUDE_DIRS "${TCL_INSTALL_DIR}/include")
    endif()

    # --------------------------------------------------------------------------
    # 5. Imported targets
    # --------------------------------------------------------------------------

    add_library(TCL::Tcl UNKNOWN IMPORTED)
    set_target_properties(TCL::Tcl PROPERTIES
        IMPORTED_LOCATION "${TCL_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${TCL_INCLUDE_DIRS}"
    )
    add_dependencies(TCL::Tcl tcl_project)

    add_library(TCL::Tk UNKNOWN IMPORTED)
    set_target_properties(TCL::Tk PROPERTIES
        IMPORTED_LOCATION "${TK_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${TCL_INCLUDE_DIRS}"
    )
    add_dependencies(TCL::Tk tk_project)

    # --------------------------------------------------------------------------
    # 6. Export runtime DLL list for Windows
    # --------------------------------------------------------------------------
    set(TCL_RUNTIME_DLLS ${TCL_RUNTIME_DLLS} PARENT_SCOPE)
    set(TCL_INCLUDE_DIRS ${TCL_INCLUDE_DIRS} PARENT_SCOPE)

endfunction()

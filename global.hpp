#pragma once


//#include <reflect/reflect.hpp>
//#include <rfl.hpp>

//#include "hhh/meta.hpp"
//#include "hhh/misc.hpp"
//using namespace hhh;

//import std;
#if 1	// dynamic
#pragma comment(lib, "tcl9tk90.lib")
#pragma comment(lib, "tcl90.lib")
#pragma comment(lib, "tommath.lib")
#pragma comment(lib, "zdll.lib")
#else	// static
#pragma comment(lib, "tcl90s.lib")
#pragma comment(lib, "tcl9tk90s.lib")
#endif

//#pragma comment(lib, "tclstub.lib")
//#pragma comment(lib, "tkstub.lib")


#pragma warning(disable : 4996)

#ifndef TCL_CORE_LIBRARY_IS_EMBEDDED
/// true/false
/// whether the Tcl core library scripts are embedded into the executable (dll or static lib).
/// if false the path to the Tcl core library must be specified with cpptkinter::init.
#define TCL_CORE_LIBRARY_IS_EMBEDDED true
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define CPPTKINTER_WARNING(msg) __pragma(warning(msg))
#else
#define CPPTKINTER_WARNING(msg) _Pragma(msg)
#endif

#define ANNOTATION_WARNING(msg) CPPTKINTER_WARNING(msg)
#define DEVIATING_IMPLEMENTATION_WARNING(msg) //CPPTKINTER_WARNING(msg)

#if !true
#define NOT_IMPLEMENTED_ERROR static_assert(false, "not implemented")
#define ANNOTATION_ERROR(msg) static_assert(false, msg)
#else
#define NOT_IMPLEMENTED_ERROR throw std::exception("not implemented")
#define ANNOTATION_ERROR(msg) throw std::exception(msg)
#endif

#ifdef _WIN32
#define MS_WINDOWS	// defined in pyconfig.h
#endif
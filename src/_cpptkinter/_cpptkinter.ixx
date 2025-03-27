/// @file _cpptkinter.ixx
/// @brief Implements _tkinter.c, _tkinter.c.h and tkappinit.c.
module;
#include <tk.h>
export module cpptkinter:_cpptkinter;
import :utility;
export import :_cpptkinter.functions;
export import :_cpptkinter.tcl_obj;
export import :_cpptkinter.templates;
export import :_cpptkinter.tkappobj;
//import :tk;


static_assert(std::same_as<Tcl_WideInt, long long>);


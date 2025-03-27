/// @file cpptkinter.ixx
/// @brief Implements __init__.py.
module;
#include "../global.hpp"
export module cpptkinter;
export import :constants;
export import :utility;
export import :_cpptkinter;
export import :cpptkinter.detail;
export import :cpptkinter.wm;
export import :cpptkinter.misc;
export import :cpptkinter.cnfs;
export import :cpptkinter1;
export import :cpptkinter.widget.base;
export import :cpptkinter.widget1;
export import :cpptkinter.widget2;
export import :ttk;
import std;
import hhh;

using namespace std::literals;


/// @brief Implementation of the Python module tkinter in C++.
export namespace cpptkinter
{
    using namespace constants;

    const auto TkVersion = std::atof(_cpptkinter::TK_VERSION.data());
    const auto TclVersion = std::atof(_cpptkinter::TCL_VERSION.data());

    //constexpr auto wantobjects = 1; deprecated, always true
    using _cpptkinter::READABLE;
    using _cpptkinter::WRITABLE;
    using _cpptkinter::EXCEPTION;

    /// @brief Inhibit setting of default root window.
    /// 
    /// Call this function to inhibit that the first instance of Tk is used for windows without an explicit parent window.
	void NoDefaultRoot()
	{
		detail::_support_default_root = false;
        detail::_default_root = nullptr;
	}

    /// @brief Initialize cpptkinter.
    /// 
    /// This function must be called before doing anything else.
    /// @param executable_path: usually arg[0].
    /// @param tcl_library: Path to the Tcl library. Necessary only used if TCL_CORE_LIBRARY_IS_EMBEDDED is false.
    void init(const std::string& executable_path, const std::string& tcl_library = {})
    {
        _cpptkinter::init(executable_path, tcl_library);
    }

    void mainloop(int n = 0)
    {
        detail::_get_default_root("call mainloop").tk->mainloop(n);
    }

    std::vector<std::string> image_names()
    {
        auto tk = detail::_get_default_root("use image_names()").tk;
        return tk->call<std::vector<std::string>>("image", "names");
    }

    std::vector<std::string> image_types()
    {
        auto tk = detail::_get_default_root("use image_types()").tk;
        return tk->call<std::vector<std::string>>("image", "types");
    }
}
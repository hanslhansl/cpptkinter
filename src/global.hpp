

/// @mainpage cpptkinter
///
/// _Cpptkinter_ is a binding for _tk_ inspired by _python's_ _tkinter_.
///
/// - @subpage philosophy
/// - @subpage terminology
/// - @subpage design
/// - @subpage threads
/// - @subpage example
/// - @subpage building
/// - @subpage state
/// - @subpage todo

/// @page philosophy philosophy
/// The goal is to provide a library which mirrors _tkinter's_ api as closely as possible.
/// This requires a plethora of meta programming and other shenanigans which makes this project a rather academic approach to gui programming.

/// @page terminology terminology
/// To prevent misunderstandings the following terms are defined as
/// - _tcl_: the [scripting language](https://en.wikipedia.org/wiki/Tcl)
/// - _tk_: the [gui toolkit](https://en.wikipedia.org/wiki/Tk_(software)) available in _tcl_
/// - _tkinter_: the [Python binding](https://en.wikipedia.org/wiki/Tkinter) for _tk_
/// - __tkinter_: _tkinter's_ backend written in _c_
/// - _cpptkinter_: [this library](https://github.com/hanslhansl/cpptkinter)
/// - __cpptkinter_: the implementation of __tkinter_ in modern day _c++_ (also part of this project)
/// - _ttk_: may refer to the [tk themed widget set](https://www.tcl-lang.org/man/tcl/TkCmd/ttk_intro.htm), or tkinter's or cpptkinter's implementation thereof (disambiguation is rarely necessary)
/// 
/// The terminology applies to this file, the documentation and source code annotations.
/// 
/// It does **not** apply to the naming of code entities.
/// E.g. `tkinter.Tk` and `cpptkinter::Tk` are classes that represent a root window in _python_ and _c++_ respectively.
/// They don't refer to the gui toolkit _tk_ itself.

/// @page design design decisions
/// _Python_ provides syntax and language features which can't easily be translated to _c++_. This section explains how _cpptkinter_ tries to implement them.
/// 
/// @section kwargs keyword arguments
/// _Tkinter_ makes heavy usage of _python's_ keyword argument syntax
/// @code {Python}
/// def func(**kwargs):
/// 	pass
/// 
/// func(foo = 2, bar = 3.14, baz = "bla")
/// @endcode
/// which allows for
/// - reordering of arguments in arbitrary order: `func(bar = 3.14, foo = 2, baz = "bla")`.
/// - omitting of arguments: `func(foo = 2, baz = "bla")`.
///
/// The first feature isn't (feasibly) reproducible in _c++_.
/// The second feature is implementable using _c++20_ [designated initializers](https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers).
/// Instead of passing the arguments directly they are passed as part of a _cnf_ struct:
/// @code {C++}
/// struct func_struct { // cnf struct
/// 	std::optional<int> foo;
/// 	std::optional<float> bar;
/// 	std::optional<std::string> baz;
/// };
/// template<typename CNF = func_struct>
/// void func(CNF&& args) {}
///
/// func({ .foo = 2, .bar = 3.14, .baz = "bla" });	// all arguments with keywords
/// func({ 2, 3.14, "bla" });						// all arguments without keywords
/// func({ .foo = 2, .baz = "bla" });				// some arguments with keywords
/// func({ 2, "bla" });								// some arguments without keywords
/// @endcode
/// _Cpptkinter_ makes use of this technique for widget constructors and many widget methods.
/// Because these functions take the _cnf_ struct by universal reference it is possible to pass any struct as argument (as long as it is an [aggregate](https://en.cppreference.com/w/cpp/language/aggregate_initialization)).
///
/// The _cnf_ structs usually have the same name as the function they are meant for and are located in `namespace cpptkinter::cnfs`.
/// 
/// @section refcount reference counting
/// _Python_ objects are reference counted: They get destroyed once no reference remains. Reference cycles are (in theory) broken by the garbage collector.
///
/// In _c++_ reference counting is usually done using [std::shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr). This solution has three major drawbacks:
/// - The contained object is accessed using the pointer syntax `ptr->member` instead of `obj.member`.
/// - A shared_ptr can point to null. Dereferencing such an empty shared_ptr invokes undefined behaviour.
/// - shared_ptr doesn't provide an algorithm for breaking reference cycles. If e.g. a master has a reference to its slave and vice versa these two objects will never be destructed because their reference counts never reach 0.
///
/// _Cpptkinter_ solves problem 1 and 2 with wrapper classes.
/// These classes are essentially a combination of the [pimpl idiom](https://en.cppreference.com/w/cpp/language/pimpl) and `std::shared_ptr`.
/// They hold an owning reference to an implementation struct which contains the members (and potentially virtual member functions).
/// The member functions are implemented inside the wrapper class instead of inside the implementation struct (solving problem 1).
/// Because the shared_ptr isn't exposed to the outside the library can guarantee that no reference ever points to null (solving problem 2).
///
/// Problem 3 isn't solved as of yet.
/// See [this issue](/../../issues/1) for further information.
/// `cpptkinter::utility::weak` provides a weak reference mechanism much like `std::weak_ptr` which can be used to break reference cycles.
/// 
/// @section converting converting objects from and to tcl
/// Some _cpptkinter_ functions take arguments which are passed on to _tcl_ and some return values from _tcl_ back to _c++_.
/// The conversion from and to _tcl_ is done by `cpptkinter::_cpptkinter::FromObj()` and `cpptkinter::_cpptkinter::AsObj()` respectively.
/// To allow for maximum flexibility these functions are heavily templated and guarded by concepts.
///
/// `cpptkinter::detail::FromObjImpl()` and `cpptkinter::detail::AsObjImpl()` can be overloaded to add support for additional types.
/// However, if added overloads change the outcome of argument-dependent lookup at existing call sites the library might break.
/// 
/// @section callbacks registering callbacks
/// In _tkinter_ it is often necessary to register member functions of widgets as callbacks:
/// @code {Python}
/// text = tk.Text()
/// sb = tk.Scrollbar(command = text.yview)
/// @endcode
/// Because member function pointers aren't bound to an instance in _c++_ the most obvious way to implement this would be:
/// @code {C++}
/// auto text = tk::Text();
/// auto sb = tk::Scrollbar({ .command = [](/*whatever args text.yview takes*/){ text.yview(/*args*/); } });
/// @endcode
/// This of course isn't very elegant and violates this library's philosophy.
/// 
/// The workaround is to implement the relevant functions as member functors i.e. member objects which overload `operator()`:
/// @code {C++}
/// struct Text
/// {
///		/*...*/
/// 
/// 	struct {
///			/*...*/
/// 		void operator()(/*args*/);
///		} yview;
/// };
/// auto text = tk::Text();
/// auto sb = tk::Scrollbar({ .command = text.yview });
/// @endcode
/// Because a member functor has to hold a reference to the impl struct it belongs to every member functor increases the size of the widget class by 8 bytes.
/// Therefor, only functions which are actually used as callbacks are implemented like this.
/// 
/// Currently, the functors hold a simple reference to the impl struct. If this should ever lead to dangling references the functors can easily be adjusted to hold weak references instead.

/// @page threads thread safety
/// In _tcl_ execution revolves around _interpreters_. _Tkinter_ adheres to this structure and so does _cpptkinter_.
/// In _cpptkinter_ a _tcl_ interpreter is represented by an instance of `cpptkinter::Tk`.
/// Every instance has its own _tcl_ interpreter.
///
/// If _tcl_ was compiled with threads disabled _cpptkinter_ isn't thread-safe.
/// Only the thread that created a _tcl_ interpreter can use that interpreter.
/// Only the thread that created a particular instance of `cpptkinter::Tk` can use it or any of its children.
///
/// If the _tcl_ interpreter was compiled with threads enabled _cpptkinter_ is somewhat thread-safe.
/// An instance of `cpptkinter::Tk` and its children can be used across multiple threads.
/// This isn't actual multithreading though as calls into _tcl_ are serialized and executed on the thread which created the _tcl_ interpreter.
/// They are not executed in parallel.
/// The thread safety only applies to _tcl_.
/// Data races inside the _c++_ part of the library may still occur if e.g. two threads modify a widget's member variable simultaniously.

/// @page example a simple example
/// @code {Python}
/// import tkinter as tk
///
/// root = tk.Tk()
/// root.mainloop()
/// @endcode
/// translates to
/// @code {C++}
/// import cpptkinter;
/// namespace tk = cpptkinter;
///
/// int main()
/// {
/// 	auto root = tk::Tk();
/// 	root.mainloop();
/// }
/// @endcode
/// See [examples](https://github.com/hanslhansl/cpptkinter/tree/main/examples) for more elaborate examples.

/// @page building
/// _Cpptkinter_ is implemented as a _c++ module_ and can be consumed via `import cpptkinter`. It requires _c++23_.
///
/// The dependencies are
/// - [tcl](https://github.com/tcltk/tcl)
/// - [tk](https://github.com/tcltk/tk)
/// - [qlibs/reflect](https://github.com/qlibs/reflect) (header only, will be replaced by _c++26 reflection_)
///
/// _Tcl/tk_ can be [built from source](https://www.tcl-lang.org/doc/howto/compile.html) but [third-party binaries](https://www.tcl-lang.org/software/tcltk/bindist.html) exist as well.
/// Make sure to get the _tcl/tk_ library binaries (e.g. .dll, .lib, .so, .a). The _tcl_ executable isn't required.
/// Both static as well as dynamic linking can be used (though I haven't gotten static linking to work on my machine yet).
///
/// I am developing on _windows_ using _msvc_ and _clang_. Once _gcc_ supports `import std` via _cmake_ I will test with this compiler as well.

/// @page state current state of the project
/// - `_tkinter`: Mostly implemented and available in `namespace cpptkinter::_cpptkinter`
/// - `tkinter`: Fully implemented except for `cpptkinter::Canvas` and `cpptkinter::Misc`.
/// - `constants`: Fully implemented.
/// - `colorchooser`: Not implemented.
/// - `commondialog`: Not implemented.
/// - `dialog`: Not implemented.
/// - `dnd`: Not implemented.
/// - `filedialog`: Not implemented.
/// - `font`: Not implemented.
/// - `messagebox`: Not implemented.
/// - `scrolledtext`: Not implemented.
/// - `simpledialog`: Not implemented.
/// - `tix`: Not implemented.
/// - `ttk`: Not implemented.

/// @page todo to-do's
/// - Implementing `cpptkinter::Canvas`.
/// - Fully implementing `cpptkinter::Misc`.
/// - Adding tests.
/// - Simplifying `cpptkinter::_cpptkinter`.
/// - Implementing `cpptkinter::ttk`.
/// - ...
/// - Implementing some of the widget member functions as calls to the c tk api instead of as tcl scripts.



//#pragma warning(disable : 4996)

namespace cpptkinter
{
	/// @brief true/false, whether the Tcl core library scripts are embedded into the executable (dll or static lib).
    /// if false the path to the Tcl core library must be specified with cpptkinter::init.
	constexpr auto TCL_CORE_LIBRARY_IS_EMBEDDED = true;
}


#if defined(_MSC_VER) && !defined(__clang__)
#define CPPTKINTER_WARNING(msg) __pragma(warning(msg))
#else
#define CPPTKINTER_WARNING(msg) _Pragma(msg)
#endif

#define ANNOTATION_WARNING(msg) CPPTKINTER_WARNING(msg)
#define DEVIATING_IMPLEMENTATION_WARNING(msg) CPPTKINTER_WARNING(msg)

#if !true
#define NOT_IMPLEMENTED_ERROR static_assert(false, "not implemented")
#define ANNOTATION_ERROR(msg) static_assert(false, msg)
#else
#define NOT_IMPLEMENTED_ERROR throw utility::construct_exception<std::exception>("not implemented")
#define ANNOTATION_ERROR(msg) throw utility::construct_exception<std::exception>(msg)
#endif

/// If Tcl is compiled for threads, we must also define TCL_THREAD. We define it always; if Tcl is not threaded, the thread functions in Tcl are empty.
#define TCL_THREADS

#define ENTER_TCL				{ auto _opt_mutex_adapter = utility::optional_mutex_adaptor(tcl_lock); auto _temp_tcl_lock = std::scoped_lock(_opt_mutex_adapter)
#define LEAVE_TCL				}
#define ENTER_OVERLAP			// nothing
#define LEAVE_OVERLAP_TCL		}

#define ENTER_PYTHON			{ auto _opt_inv_mutex_adapter = utility::optional_inverse_mutex_adaptor(tcl_lock); auto _temp_tcl_inv_lock = std::scoped_lock(_opt_inv_mutex_adapter)
#define LEAVE_PYTHON			}

#define Py_BuildValue(fmt_str, ...) __VA_ARGS__
#define TRACE(_self, ARGS) do {                 \
        if ((_self)->trace) {  \
            Tkapp_Trace((_self), Py_BuildValue ARGS);   \
        }   \
    } while (0)

#define REF_TO_IMPL(member) decltype(impl::member)& member = static_cast<impl*>(this->pimpl.get())->member

#define DEFINE_ASSIGNMENT_OPERATOR(cl) \
    cl& operator=(const cl& other) \
    { \
        std::destroy_at(this); \
        return *std::construct_at(this, other); \
    }

#define IMPL_CTOR(cl, base) friend detail::widget_friend; \
    template<std::derived_from<impl> I> cl(const std::shared_ptr<I>& pimpl) : base(pimpl) { }
#define CNF_CONSTRUCTOR(cl, cnf_type, str) \
    using constructor_cnf = cnf_type;   \
    cl() : cl(constructor_cnf{}) { }    \
    template<cnfs::is_cnf CNF = constructor_cnf> \
    cl(CNF&& cnf = {}) : cl(std::make_shared<impl>()) \
    {   \
        this->_init_(str, std::forward<CNF>(cnf));  \
    }

#define CONSTRUCTORS_AND_ASSIGNMENT(cl, cnf_type, str, base) \
    protected:  \
    IMPL_CTOR(cl, base) \
    public: \
    CNF_CONSTRUCTOR(cl, cnf_type, str)   \
    DEFINE_ASSIGNMENT_OPERATOR(cl)
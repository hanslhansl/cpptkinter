Have you ever wanted to use _python's_ tkinter in _c++_? No? Well, here you go anyways.
- [terminology](#terminology)
- [philosophy](#philosophy)
- [design decisions](#design-decisions)
  - [keyword arguments](#keyword-arguments)
  - [reference counting](#reference-counting)
  - [converting objects from and to _tcl_](#converting-objects-from-and-to-tcl)
- [thread safety](#thread-safety)
- [current state of the project](#current-state-of-the-project)
- [examples](#examples)
- [building](#building)
## terminology
To prevent misunderstandings the following terms are defined as
- _tcl_: the [scripting language](https://en.wikipedia.org/wiki/Tcl)
- _tk_: the [gui toolkit](https://en.wikipedia.org/wiki/Tk_(software)) available in _tcl_
- _tkinter_: the [Python binding](https://en.wikipedia.org/wiki/Tkinter) for _tk_
- __tkinter_: _tkinter's_ backend written in _c_
- _cpptkinter_: [this library](https://github.com/hanslhansl/cpptkinter)
- __cpptkinter_: the implementation of __tkinter_ in modern day _c++_ (also part of this project)

The terminology applies to this file, the documentation and source code annotations.

It does **not** apply to the naming of code entities. E.g. `tkinter.Tk` and `cpptkinter::Tk` are classes that represent a root window in _python_ and _c++_ respectively. They don't refer to the gui toolkit _tk_ itself.
## philosophy
The goal is to provide a library which mirrors _tkinter_ as closely as possible. This requires a plethora of meta programming and other shenanigans which makes this project a rather academic approach to gui programming.

Nevertheless, it is definitely usable in real life code. See [examples](#examples) and [more elaborate examples](examples).
## design decisions
_Python_ provides syntax and language features which can't easily be translated to _c++_. This section explains how _cpptkinter_ tries to implement them.
### keyword arguments
_Tkinter_ makes heavy usage of _python's_ keyword argument syntax
```Python
def func(**kwargs):
    pass

func(foo = 2, bar = 3.14, baz = "bla")
```
which allows for
- reordering of arguments in arbitrary order: `func(bar = 3.14, foo = 2, baz = "bla")`.
- omitting of arguments: `func(foo = 2, baz = "bla")`.

The first feature isn't (feasibly) reproducible in _c++_. The second feature is implementable using _c++20_ [designated initializers](https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers):
```C++
struct func_struct { // cnf struct
    std::optional<int> foo;
    std::optional<float> bar;
    std::optional<std::string> baz;
};
void func(func_struct args) {}

func({ .foo = 2, .bar = 3.14, .baz = "bla" }); // all arguments with keywords
func({ 2, 3.14, "bla" }); // all arguments without keywords
func({ .foo = 2, .baz = "bla" }); // some arguments with keywords
func({ 2, "bla" }); // some arguments without keywords
```
_Cpptkinter_ makes use of this technique for widget constructors and many widget methods. The _cnf_ structs have the same name as the function they are meant for and are located in `namespace cpptkinter::cnfs`.
### reference counting
_Python_ objects are reference counted: They get destroyed once no reference remains. Reference cycles are (in theory) broken by the garbage collector.

In _c++_ reference counting is usually done using [std::shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr). This solution has three major drawbacks:
- The contained object is accessed using the pointer syntax `ptr->member` instead of `obj.member`.
- A shared_ptr can point to null. Dereferencing such an empty shared_ptr invokes undefined behaviour.
- shared_ptr doesn't provide an algorithm for breaking reference cycles. If e.g. a master has a reference to its slave and vice versa these two objects will never be destructed because their reference count never reaches 0.

_Cpptkinter_ solves problem 1 and 2 with wrapper classes. These classes are essentially a combination of the [pimpl idiom](https://en.cppreference.com/w/cpp/language/pimpl) and `std::shared_ptr`. They hold an owning reference to an implementation struct which contains the members (and potentially virtual member functions). The member functions are implemented inside the wrapper class instead of inside the implementation struct (solving problem 1). Because the shared_ptr isn't exposed to the outside the library can guarantee that no reference ever points to null (solving problem 2).

Problem 3 isn't solved as of yet. See [this issue](/../../issues/1) for further information. `cpptkinter::utility::weak` provides a weak reference mechanism much like `std::weak_ptr` which can be used to break reference cycles.
### converting objects from and to _tcl_
Some _cpptkinter_ functions take arguments which are passed on to _tcl_ and some return values from _tcl_ back to _c++_. The conversion from and to _tcl_ is done by `cpptkinter::_cpptkinter::FromObj()` and `cpptkinter::_cpptkinter::AsObj()` respectively. To allow for maximum flexibility these functions are heavily templated and guarded by concepts.

`cpptkinter::_cpptkinter::detail::FromObjImpl()` and `cpptkinter::_cpptkinter::detail::AsObjImpl()` can be overloaded to add support for additional types. However, if added overloads change the outcome of argument-dependent lookup at existing call sites the library might break.
### thread safety
In _tcl_ execution revolves around _interpreters_. _Tkinter_ adheres to this this structure and so does _cpptkinter_. In _cpptkinter_ a _tcl_ interpreter is represented by an instance of `cpptkinter::Tk`. Every instance has its own _tcl_ interpreter.

If _tcl_ was compiled with threads disabled _cpptkinter_ isn't thread-safe. Only the thread that created a _tcl_ interpreter can use that interpreter. Only the thread that created a particular instance of `cpptkinter::Tk` can use it or any of its children.

If the _tcl_ interpreter was compiled with threads enabled _cpptkinter_ is somewhat thread-safe. An instance of `cpptkinter::Tk` and its children can be used across multiple threads. This isn't actual multithreading though as calls into _tcl_ are serialized and executed on the thread which created the _tcl_ interpreter. They are not executed in parallel. The thread safety only applies to _tcl_. Data races inside the _c++_ part of the library may still occur if e.g. two threads modify a widget's member variable simultaniously.
## current state of the project
- The three geometry managers, `pack`, `place` and `grid`, are fully implemented for all available widget classes.
- `Variable`, `StringVar`, `IntVar`, `DoubleVar` and `BooleanVar` are fully implemtented.
- The window manager class `Wm`, which is base for some widget classes, is partially implemeted.
- `Misc`, which is base for all widget classes, is partially implemtented.
- `BaseWidget` and `Widget`, which are base for many widget classes, are fully implemtented.
- `Tk`, `Toplevel`, `Button`, `Frame`, `Label`, `Scale` and `LabelFrame` are fully implemented. However, a lot of their functionality is inherited from `Misc` and therefor not implemented as of yet.
- `_tkinter`is implemented for the most part and available in `namespace cpptkinter::_cpptkinter`
## examples
See [examples](examples) for more elaborate examples.
## building
_Cpptkinter_ requires _c++23_. It is tested with _msvc_ and _clang_ on _windows_.

The dependencies are
- [tcl](https://github.com/tcltk/tcl)
- [tk](https://github.com/tcltk/tk)
- [qlibs/reflect](https://github.com/qlibs/reflect) (until _c++26_ reflection)
- [getml/reflect-cpp](https://github.com/getml/reflect-cpp) (because _qlibs/reflect_ is missing features)

_Tcl/tk_ can be built from source but third-party binaries exist as well. Make sure that you get the _tcl/tk_ library binaries (e.g. .dll, .lib, .so, .a). The _tcl_ executable isn't required. Both static as well as dynamic linking can be used (though I haven't gotten static linking to work on my machine yet).

The other dependencies and _cpptkinter_ itself need to be compiled as part of your project. Most of the functionality is templated so precompiling these wouldn't be useful anyways.

Add `#include cpptkinter.hpp` to your source files to use the library.






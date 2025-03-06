# cpptkinter
 Have you ever wanted to use _python's_ tkinter in _c++_? No? Well, here you go anyways.
- [terminology](#terminology)
- [philosophy](#philosophy)
- [design decisions](#design-decisions)
  - [keyword arguments](#keyword-arguments)
  - [reference counting](#reference-counting)
  - [converting objects from and to _tcl_](#converting-objects-from-and-to-tcl)
- [thread safety](#thread-safety)
- [simple example](#simple-example)
- [documentation](#documentation)
- [building](#building)
- [current state of the project](#current-state-of-the-project)
## philosophy
The goal is to provide a library which mirrors _tkinter's_ api as closely as possible.
This requires a plethora of meta programming and other shenanigans which makes this project a rather academic approach to gui programming.

Nevertheless, it is definitely usable in real life code. See [simple example](#simple-example) and [more elaborate examples](examples).
## terminology
To prevent misunderstandings the following terms are defined as
- _tcl_: the [scripting language](https://en.wikipedia.org/wiki/Tcl)
- _tk_: the [gui toolkit](https://en.wikipedia.org/wiki/Tk_(software)) available in _tcl_
- _tkinter_: the [Python binding](https://en.wikipedia.org/wiki/Tkinter) for _tk_
- __tkinter_: _tkinter's_ backend written in _c_
- _cpptkinter_: [this library](https://github.com/hanslhansl/cpptkinter)
- __cpptkinter_: the implementation of __tkinter_ in modern day _c++_ (also part of this project)
- _ttk_: may refer to the [tk themed widget set](https://www.tcl-lang.org/man/tcl/TkCmd/ttk_intro.htm), or tkinter's or cpptkinter's implementation thereof (disambiguation is rarely necessary)

The terminology applies to this file, the documentation and source code annotations.

It does **not** apply to the naming of code entities.
E.g. `tkinter.Tk` and `cpptkinter::Tk` are classes that represent a root window in _python_ and _c++_ respectively.
They don't refer to the gui toolkit _tk_ itself.
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

The first feature isn't (feasibly) reproducible in _c++_.
The second feature is implementable using _c++20_ [designated initializers](https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers).
Instead of passing the arguments directly they are passed as part of a _cnf_ struct:
```C++
struct func_struct { // cnf struct
    std::optional<int> foo;
    std::optional<float> bar;
    std::optional<std::string> baz;
};
template<typename CNF = func_struct>
void func(CNF&& args) {}

func({ .foo = 2, .bar = 3.14, .baz = "bla" }); // all arguments with keywords
func({ 2, 3.14, "bla" }); // all arguments without keywords
func({ .foo = 2, .baz = "bla" }); // some arguments with keywords
func({ 2, "bla" }); // some arguments without keywords
```
_Cpptkinter_ makes use of this technique for widget constructors and many widget methods.
Because these functions take the _cnf_ struct by universal reference it is possible to pass any struct as argument (as long as it is an [aggregate](https://en.cppreference.com/w/cpp/language/aggregate_initialization)).

The _cnf_ structs usually have the same name as the function they are meant for and are located in `namespace cpptkinter::cnfs`.
### reference counting
_Python_ objects are reference counted: They get destroyed once no reference remains. Reference cycles are (in theory) broken by the garbage collector.

In _c++_ reference counting is usually done using [std::shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr). This solution has three major drawbacks:
- The contained object is accessed using the pointer syntax `ptr->member` instead of `obj.member`.
- A shared_ptr can point to null. Dereferencing such an empty shared_ptr invokes undefined behaviour.
- shared_ptr doesn't provide an algorithm for breaking reference cycles. If e.g. a master has a reference to its slave and vice versa these two objects will never be destructed because their reference counts never reach 0.

_Cpptkinter_ solves problem 1 and 2 with wrapper classes.
These classes are essentially a combination of the [pimpl idiom](https://en.cppreference.com/w/cpp/language/pimpl) and `std::shared_ptr`.
They hold an owning reference to an implementation struct which contains the members (and potentially virtual member functions).
The member functions are implemented inside the wrapper class instead of inside the implementation struct (solving problem 1).
Because the shared_ptr isn't exposed to the outside the library can guarantee that no reference ever points to null (solving problem 2).

Problem 3 isn't solved as of yet.
See [this issue](/../../issues/1) for further information.
`cpptkinter::utility::weak` provides a weak reference mechanism much like `std::weak_ptr` which can be used to break reference cycles.
### converting objects from and to _tcl_
Some _cpptkinter_ functions take arguments which are passed on to _tcl_ and some return values from _tcl_ back to _c++_.
The conversion from and to _tcl_ is done by `cpptkinter::_cpptkinter::FromObj()` and `cpptkinter::_cpptkinter::AsObj()` respectively.
To allow for maximum flexibility these functions are heavily templated and guarded by concepts.

`cpptkinter::_cpptkinter::detail::FromObjImpl()` and `cpptkinter::_cpptkinter::detail::AsObjImpl()` can be overloaded to add support for additional types.
However, if added overloads change the outcome of argument-dependent lookup at existing call sites the library might break.
### thread safety
In _tcl_ execution revolves around _interpreters_. _Tkinter_ adheres to this structure and so does _cpptkinter_.
In _cpptkinter_ a _tcl_ interpreter is represented by an instance of `cpptkinter::Tk`.
Every instance has its own _tcl_ interpreter.

If _tcl_ was compiled with threads disabled _cpptkinter_ isn't thread-safe.
Only the thread that created a _tcl_ interpreter can use that interpreter.
Only the thread that created a particular instance of `cpptkinter::Tk` can use it or any of its children.

If the _tcl_ interpreter was compiled with threads enabled _cpptkinter_ is somewhat thread-safe.
An instance of `cpptkinter::Tk` and its children can be used across multiple threads.
This isn't actual multithreading though as calls into _tcl_ are serialized and executed on the thread which created the _tcl_ interpreter.
They are not executed in parallel.
The thread safety only applies to _tcl_.
Data races inside the _c++_ part of the library may still occur if e.g. two threads modify a widget's member variable simultaniously.
## simple example
```Python
import tkinter as tk

root = tk.Tk()
root.mainloop()
```
translates to
```C++
#include "cpptkinter.hpp"
namespace tk = cpptkinter;

int main() {
    auto root = tk::Tk();
    root.mainloop();
}
```
See [examples](examples) for more elaborate examples.
## documentation
The documentation is generated using [doxygen](https://www.doxygen.nl/index.html).
The documentation is available online on [github pages](https://hanslhansl.github.io/cpptkinter/).
## building
_Cpptkinter_ is implemented as a _c++ module_ and can be consumed via `import cpptkinter`. It requires _c++23_.

The dependencies are
- [tcl](https://github.com/tcltk/tcl)
- [tk](https://github.com/tcltk/tk)
- [qlibs/reflect](https://github.com/qlibs/reflect) (header only, will be replaced by _c++26 reflection_)

_Tcl/tk_ can be [built from source](https://www.tcl-lang.org/doc/howto/compile.html) but [third-party binaries](https://www.tcl-lang.org/software/tcltk/bindist.html) exist as well.
Make sure to get the _tcl/tk_ library binaries (e.g. .dll, .lib, .so, .a). The _tcl_ executable isn't required.
Both static as well as dynamic linking can be used (though I haven't gotten static linking to work on my machine yet).

_Cpptkinter_ is tested with _msvc_ on _windows_. Once _clang_ supports the necessary _c++23_ library features and _gcc_ supports `import std` via _cmake_ I will test with these compilers as well.
## current state of the project
- The three geometry managers, `pack`, `place` and `grid`, are implemented for all available widget classes.
- `Variable`, `StringVar`, `IntVar`, `DoubleVar` and `BooleanVar` are implemtented.
- The window manager class `Wm`, which is base for some widget classes, is mostly implemeted.
- `Misc`, which is base for all widget classes, is partially implemtented.
- `BaseWidget` and `Widget`, which are base for many widget classes, are implemtented.
- The only widget not implemented is `Canvas`. However, a lot of the functionality of all widgets is inherited from `Misc` and therefor not implemented as of yet.
- `_tkinter`is implemented for the most part and available in `namespace cpptkinter::_cpptkinter`

The next step is to fully implement `Misc` and `ttk`.



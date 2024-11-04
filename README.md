# cpptkinter
Have you ever wanted to use python's tkinter in c++? No? Well, here you go anyways.
## terminology
To prevent misunderstandings the following terms are defined as
- _Tcl_: the [scripting language](https://en.wikipedia.org/wiki/Tcl)
- _Tk_: the [gui toolkit](https://en.wikipedia.org/wiki/Tk_(software)) available in _tcl_
- _Tkinter_: the [Python binding](https://en.wikipedia.org/wiki/Tkinter) for _tk_
- _Cpptkinter_: [this library](https://github.com/hanslhansl/cpptkinter)

The terminology applies to this file, the documentation and source code annotations.

It does **not** apply to the naming of code entities. E.g. `tkinter.Tk` and `cpptkinter::Tk` are classes that represent a root window in python and c++ respectively. They don't refer to the gui toolkit _tk_ itself.
## philosophy
The goal is to provide a library which mirrors _tkinter_ as closely as possible. This requires a plethora of meta programming shenanigans which makes this project a rather academic approach to gui programming.

Nevertheless, it is definitely usable in real life code. See [examples](#examples) and [more elaborate examples](examples).
## design decisions
Python provides syntax and language features which can't easily be translated to c++. This section explains how _cpptkinter_ tries to implement them.
### keyword arguments
_Tkinter_ makes heavy usage of python's keyword argument syntax
```Python
def func(**kwargs):
    pass

func(foo = 2, bar = 3.14, baz = "bla")
```
This feature has two possible effects:
- Reordering of arguments in arbitrary order: `func(bar = 3.14, foo = 2, baz = "bla")`.
- Omitting arguments: `func(foo = 2, baz = "bla")`.

Both are useful when working with _tkinter_. The first effect isn't (feasibly) reproducible in c++. The second effect is implementable using c++20 [designated initializers](https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers):
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
_Cpptkinter_ makes use of this technique for widget constructors and many widget methods. The _cnf_ structs have the same name as the function they are meant for and are located in namespace `cpptkinter::cnfs`.
### reference counting
Python objects are reference counted: They get destroyed once no reference remains. Reference cycles are (in theory) broken by the garbage collector.

In c++ reference counting is usually implemented using [std::shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr). This solution has three major drawbacks:
- shared_ptr doesn't provide an algorithm for breaking reference cycles. If e.g. a master has a reference to its slave and vice versa these two objects will never be destructed because their reference count never reaches 0.
- A shared_ptr can point to null. Dereferencing such an empty shared_ptr invokes undefined behaviour.
- The contained object is accessed using the pointer syntax `ptr->member` instead of `obj.member`.

_Cpptkinter_ solves problem 2 and 3 with wrapper classes. These classes are essentially a combination of the [pimpl idiom](https://en.cppreference.com/w/cpp/language/pimpl) and `std::shared_ptr`. They hold an owning reference to an implementation struct which contains the member objects (and potentially virtual functions). Member functions are implemented inside the wrapper class instead of inside the implementation class (solving problem )
### converting objects from c++ to tcl
### converting objects from tcl to c++
## thread safety
## examples

# cpptkinter
Have you ever wanted something like Python's Tkinter in C++? No? Well, here you go anyways.
## terminology
To prevent misunderstandings the following terms are defined as
- _Tcl_: the [scripting language](https://en.wikipedia.org/wiki/Tcl)
- _Tk_: the [gui toolkit](https://en.wikipedia.org/wiki/Tk_(software)) available in _Tcl_
- _Tkinter_: the [Python binding](https://en.wikipedia.org/wiki/Tkinter) for _Tk_
- _Cpptkinter_: [this library](https://github.com/hanslhansl/cpptkinter)

The terminology applies to the documentation and source code annotations.

It does **not** apply to the naming of code entities. E.g. `tkinter.Tk` and `cpptkinter::Tk` are classes that represent a root window in Python and C++ respectively. They don't refer to the gui toolkit _Tk_ itself.
## philosophy
Goal of this project is to provide a library which mirrors _Tkinter_ as closely as possible while still being a viable C++ gui toolkit.
## synopsis
Python provides syntax and language features which can't easily be translated to C++. This section explains how _Cpptkinter_ tries to implement them nevertheless.
### default arguments
### keyword arguments
### reference counting
### dynamic typing
## examples


#include "__init__.hpp"
#include "vld.h"

using namespace hhh;
namespace tk = cpptkinter;
namespace _tk = tk::_cpptkinter;

int main(int argc, char* argv[])
{
    //tk::detail::_debug = true;

    /*
    to-do
    - widget container 2 options
        - shared_ptr to widget
            - pro:
                - nullable
            - con:
                - obj->func() and obj->member
                - non-owning not possible
                - weak reference with weak_ptr
        - wrapper class containing shared_ptr to impl
            - pro:
                - obj.func() and potentially obj.member (as reference to impl)
                - obj[idx]
                - non-owning possible with custom dtor
                - weak reference with custom class
            - con:
                - not nullable, master will be std::optional or special "empty" value
    - make intellisense run in msvc mode even when using clang
    */

	tk::utility::weak<tk::Tk> wroot;

    try {

        tk::init(argc, argv);

        auto root = tk::Tk();
		wroot = root;

        auto toplvl = tk::Toplevel({ root });

        tk::StringVar var({}, "Hello, World!");

        auto b1 = tk::Button({
            .master = root,
            .command = [&var]() { var.set("foo"); },
            .textvariable = var });
        b1.grid({ .column = 0, .row = 0 });

        auto b2 = tk::Button({
            .master = root,
            .text = "other b" });
        b2["command"] = [&]() { b2["text"] = "bar"; };
        b2.grid({ .column = 1, .row = 1 });
        b2.grid_info();


        tk::mainloop();

    } catch (const std::exception& ex) { misc::printl(typeid(ex).name()); misc::printl(ex.what()); }


    return 0;
}

#include "cpptkinter.hpp"
#include "vld.h"

using namespace hhh;
namespace tk = cpptkinter;
namespace _tk = tk::_cpptkinter;

int main(int argc, char* argv[])
{
    //tk::detail::_debug = true;

    /*
    to-do

    - make intellisense run in msvc mode even when using clang
    */

	tk::utility::weak<tk::Tk> wroot;

    try {

        tk::init(argc, argv);

        auto root = tk::Tk();
		wroot = root;

        root.wm_attributes();

        root.forget(root);

        auto toplvl = tk::Toplevel({ root });

        tk::StringVar var({}, "Hello, World!");

        auto b1 = tk::Button({
            .master = root,
            .command = [&var]() { var.set("foo"); },
            .textvariable = var });
        b1.grid({ .column = 0, .row = 0 });

		auto frame = tk::Frame({ .master = root, .bg = "red", .padx = 5, });
		frame.grid({ .column = 1, .row = 1 });

        auto b2 = tk::Button({ .master = frame, .text = "other b" });
        b2["command"] = [&]() { b2["text"] = "bar"; };
        b2.grid();

        auto scale = tk::Scale({ .master = frame});
        scale.grid();


        tk::mainloop();

    } catch (const std::exception& ex) { misc::printl(typeid(ex).name()); misc::printl(ex.what()); }


    return 0;
}
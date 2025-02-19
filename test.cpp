
#include "cpptkinter.hpp"
#include "vld.h"

using namespace hhh;
namespace tk = cpptkinter;
namespace _tk = tk::_cpptkinter;


void donothing()
{
    misc::printl("donothing was called");
}

int main(int argc, char* argv[])
{
    tk::init(argc, argv);
    //tk::detail::_debug = true;

    /*
    to-do

    - make intellisense run in msvc mode even when using clang
    */


	tk::utility::weak<tk::Tk> wroot;

    try {

        auto root = tk::Tk();
		wroot = root;

        misc::printl(tk::utility::container_or_tuple_to_string(root.wm_attributes()));

        auto menubar = tk::Menu({ root });
        auto filemenu = tk::Menu({ .master = menubar, .tearoff = 0 });
        filemenu.add_checkbutton({ .label = "1"});
        filemenu.add_checkbutton({ .label = "cb" });
        filemenu.add_checkbutton({ .label = "3" });
        filemenu.add_command({ .command = donothing, .label = "New" });
        filemenu.add_command({ .command = donothing, .label = "Open" });
        filemenu.add_command({ .command = donothing, .label = "Save" });
        filemenu.add_command({ .command = donothing, .label = "Save as..." });
        filemenu.add_command({ .command = donothing, .label = "Close" });

        filemenu.add_separator();
        filemenu.add_command({ .command = [&]() { root.quit(); }, .label = "Exit" });
        menubar.add_cascade({ .label = "File", .menu = filemenu });
        auto editmenu = tk::Menu({ .master = menubar, .tearoff = 0 });
        editmenu.add_command({ .command = donothing, .label = "Undo" });
        editmenu.add_separator();
        editmenu.add_command({ .command = donothing, .label = "Cut" });
        editmenu.add_command({ .command = donothing, .label = "Copy" });
        editmenu.add_command({ .command = donothing, .label = "Paste" });
        editmenu.add_command({ .command = donothing, .label = "Delete" });
        editmenu.add_command({ .command = donothing, .label = "Select All",});

        menubar.add_cascade({ .label = "Edit", .menu = editmenu });
        auto helpmenu = tk::Menu({ .master = menubar, .tearoff = 0 });
        helpmenu.add_command({ .command = donothing, .label = "Help Index" });
        helpmenu.add_command({ .command = donothing, .label = "About..." });
        menubar.add_cascade({ .label = "Help", .menu = helpmenu });

        root.config("menu", menubar);

        filemenu.delete_(1);

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

    misc::printl("wroot.use_count() ", wroot.use_count());

    return 0;
}

#include "cpptkinter.hpp"
#include "vld.h"

using namespace hhh;
namespace tk = cpptkinter;
namespace _tk = tk::_cpptkinter;




int main(int argc, char* argv[])
{
    tk::init(argc, argv);
    tk::detail::_debug = false;

    tk::detail::to_index("");
    tk::detail::to_index(1);
    tk::detail::to_index(1ll);
    tk::detail::to_index(std::string());


    /*
    to-do

    */


	tk::utility::weak<tk::Tk> wroot;

    try
    {
        auto root = tk::Tk();
        root.title("Welcome to GeeksForGeeks");
        root.geometry("700x500");

		wroot = root;

		auto donothing = [&]() { misc::printl("do_nothing was called"); };

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

        tk::StringVar var({ .value = "Hello, World!" });

        auto b1 = tk::Button({
            .master = root,
            .command = [&var]() { var.set("foo"); return 1; },
            .textvariable = var });
        b1.grid({ .column = 0, .row = 0 });

		auto frame = tk::Frame({ .master = root, .bg = "red", .padx = 5, });
		frame.grid({ .column = 1, .row = 1 });

        auto b2 = tk::Button({ .master = frame, .text = "other b" });
        b2["command"] = [&]() { b2["text"] = "bar"; };
        b2.grid();

        auto scale = tk::Scale({ .master = frame }});
        scale.grid();

        auto options_list = { "Option 1", "Option 2", "Option 3", "Option 4" };
        auto value_inside = tk::StringVar();
        value_inside.set("Select an Option");
        auto question_menu = tk::OptionMenu(root, value_inside, options_list);
        question_menu.grid();
        auto print_answers = [&]() { misc::printl("Selected Option: ", value_inside.get()); };
        auto submit_button = tk::Button({ .master = root, .command = print_answers, .text = "Submit" });
        submit_button.grid();

		auto check1 = tk::Checkbutton({ .master = root, .text = "Check 1" });
        check1.grid();


        auto listbox = tk::Listbox();
        listbox.insert(0, 1, 2, 3, 4);
        listbox.grid();

        tk::mainloop();

    } catch (const std::exception& ex) { std::println("exception type: {}\n{}", typeid(ex).name(), ex.what()); }

    misc::printl("wroot.use_count() ", wroot.use_count());

    return 0;
}
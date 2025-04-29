#ifndef NDEBUG
//#include "vld.h"
#endif
import std;
import hhh;
import cpptkinter;
import aggregate_formatter;
import variant_formatter;

using namespace hhh;
namespace tk = cpptkinter;
namespace ttk = tk::ttk;



int main(int argc, char* argv[])
{
    tk::detail::_debug = false;
	tk::utility::weak<tk::Tk> wroot;


    try
    {
        tk::init(argv[0]);

        auto root = tk::Tk();
        wroot = root;

        tk::Misc root2 = root;

        root.title("Welcome");

        auto donothing = [&]() { misc::printl("do_nothing was called"); };

        auto menubar = tk::Menu({ root });
        auto filemenu = tk::Menu({ .master = menubar, .tearoff = 0 });
        filemenu.add_checkbutton({ .label = "1" });
        filemenu.add_checkbutton({ .label = "xfgj" });
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
        editmenu.add_command({ .command = donothing, .label = "Select All", });

        menubar.add_cascade({ .label = "Edit", .menu = editmenu });
        auto helpmenu = tk::Menu({ .master = menubar, .tearoff = 0 });
        helpmenu.add_command({ .command = donothing, .label = "Help Index" });
        helpmenu.add_command({ .command = donothing, .label = "About..." });
        menubar.add_cascade({ .label = "Help", .menu = helpmenu });

        root.config("menu", menubar);

        filemenu.delete_(1);

        tk::StringVar var({ .value = "Hello, World!" });
        auto b1 = tk::Button({
            .master = root,
            .command = [&var]() { var.set("foo"); return 1; },
            .textvariable = var });
        b1.grid({ .column = 3, .row = 2 });

        auto frame = tk::Frame({ .master = root, .bg = "red", .padx = 5, });
        frame.grid({ .column = 1, .row = 1 });

        auto b2 = tk::Button({ .master = frame, .text = "other b" });
        b2["command"] = [&]() { b2["text"] = "bar"; };
        b2.grid();

        auto scale = tk::Scale({ .master = frame });
        scale.grid();

        std::vector options_list = { "Opt 1", "Opt 2", "Opt 3", "Opt 4" };
        auto value_inside = tk::StringVar();
        value_inside.set("Select an Option");
        auto question_menu = tk::OptionMenu(root, value_inside, options_list);
        question_menu.grid({ .column = 2, .row = 1 });
        auto print_answers = [&]() { misc::printl("Selected Option: ", value_inside.get()); };
        auto submit_button = tk::Button({ .master = root, .command = print_answers, .text = "Submit" });
        submit_button.grid({ .column = 2, .row = 2 });

        auto check1 = tk::Checkbutton({ .master = root, .offvalue = 0, .onvalue = 1, .text = "Check 1", .tristatevalue = -1 });
        auto toggle_checkbutton = [](tk::Event event) {
            auto&& checkbutton = event.widget;
            auto varname = checkbutton.cget<std::string>("variable");
            auto current_value = checkbutton.tk->getvar<long long>(varname);
            long long new_value;
            if (current_value == 1)
                new_value = 0;
            else if (current_value == 0)
                new_value = -1;
            else
                new_value = 1;
            checkbutton.tk->setvar(varname, new_value);
            return std::string("break");
            };
        check1.bind("<1>", toggle_checkbutton);
        check1.grid({ .column = 2, .row = 0 });


        auto listbox = tk::Listbox();
        listbox.insert(0, 1, 2, 3, 4);
        listbox.grid({ .column = 3, .row = 0 });

        auto pw = tk::PanedWindow();
        pw.grid({ .column = 3, .row = 1 });
        auto pwb = tk::Button({ .text = "pwb" });
        pw.add({ .child = pwb, .minsize = 100 });

        auto ys = tk::Scrollbar({ .master = root, .orient = "vertical" });
        ys.grid({ .column = 1, .row = 0, .sticky = "ns" });
        auto text = tk::Text({.master= root, .height = 10, .width=40, .wrap="none"});
        text["yscrollcommand"] = ys.set;
        ys["command"] = text.yview;
        text.grid({ .column = 0, .row = 0, .sticky = "nwes" });
        for (int i = 0; i<105;i++)
        {
            auto b = tk::Button({ .master = text, .command = [i]() { misc::printl("button ", i); }, .text = std::to_string(i) });
            text.window_create({ .index="end", .window = b });
            text.insert("end", "\n");
        }

        auto nb = ttk::Notebook({ .master = root });
        nb.add({ .child = tk::Button({ .text = "fxjcghcgkh" }), .text = "1" });
        nb.add({ .child = tk::Button({ .text = "cc bnkvhjl" }), .text = "2" });
        nb.grid({ .column = 0, .columnspan = 1, .row = 2, .sticky = "nswe" });

        auto tree = ttk::Treeview({ .master = root, .columns = std::vector<std::string>{ "Name", "Age" }, .selectmode = "none", .show = "headings" });

        tree.heading({ .column = "Name", .text = "Name" });
        tree.heading({ .column = "Age", .text = "Age" });

        std::println("{}", tree.heading(0));

        tree.column({ .column = "Name", .width = 100 });
        tree.column({ .column = "Age", .width = 50 });

        std::vector<std::vector<std::string>> data = { { "Alice", "25" }, { "Bob", "30" }, { "Charlie", "22" }, { "David", "35" } };
		for (auto& item : data)
            tree.insert({ .parent = "", .index = "end", .values = item });

        auto toggle_selection = [&](tk::Event event) {
            auto item = tree.identify_row(event.y);
			if (std::ranges::contains(tree.selection(), item))
				tree.selection_remove(item);
			else
				tree.selection_add(item);
            };
        tree.bind("<Button-1>", toggle_selection);

        tree.grid({ .column = 1, .row = 2, .sticky = "nswe" });

        auto get_selected_items = [&]() {
            auto selected_items = tree.selection();
			for (auto& item : selected_items)
                std::println("{}", tree.item(item).values);
            };
        tk::Button({ .master = root, .command = get_selected_items, .text = "Get Selected" }).grid({ .column = 1, .row = 3, .sticky = "nswe" });

        root.after(3000, [&]() { misc::printl("after 3000ms"); });

        root.mainloop();
    }
    catch (const std::exception& ex)
    //catch (const std::logic_error& ex)
    { std::println("exception type: {}\n{}", typeid(ex).name(), ex.what()); }

    misc::printl("wroot.use_count() ", wroot.use_count());

    return 0;
}
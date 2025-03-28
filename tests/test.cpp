#ifndef NDEBUG
//#include "vld.h"
#endif
import std;
import hhh;
import cpptkinter;
import aggregate_formatter;

using namespace hhh;
namespace tk = cpptkinter;
namespace ttk = tk::ttk;


class MainFrame : public tk::Frame
{
    static tk::Canvas create_canvas(const tk::Misc& master)
    {
        // Create outer frame to hold canvas + scrollbars
        auto outer_frame = tk::Frame({ master });
        outer_frame.pack({ .expand = true, .fill = "both" });

        // Create a canvas
        auto canvas = tk::Canvas({ outer_frame });

        // Add a vertical scrollbar to the canvas
        auto v_scrollbar = tk::Scrollbar({ .master = outer_frame, .command = canvas.yview, .orient = "vertical" });

        // Add a horizontal scrollbar to the canvas
        auto h_scrollbar = tk::Scrollbar({ .master = outer_frame, .command = canvas.xview, .orient = "horizontal" });

        h_scrollbar.pack({ .fill = "x", .side = "bottom" });
        canvas.pack({ .expand = true, .fill = "both", .side = "left" });
        v_scrollbar.pack({ .fill = "y", .side = "right" });

        // Configure the canvas to work with the scrollbars
        canvas["yscrollcommand"] = v_scrollbar.set;
        canvas["xscrollcommand"] = h_scrollbar.set;

        return canvas;
    }

    MainFrame(tk::Canvas canvas, int) : tk::Frame({ canvas })
    {
        auto&& scrollable_frame = *this;

        // Add the frame to the canvas's window
        auto canvas_window = canvas.create_window(0, 0, { .anchor = "nw", .window = scrollable_frame });

        // Configure scrolling
        auto on_frame_configure = [=](tk::Event event) {
            canvas["scrollregion"] = canvas.bbox("all");
            //canvas.itemconfig(canvas_window, width=scrollable_frame.winfo_reqwidth())
            };
        scrollable_frame.bind("<Configure>", on_frame_configure);

        auto on_canvas_resize = [](tk::Event event) {
            //canvas.itemconfig(canvas_window, width = event.width);
            };
        canvas.bind("<Configure>", on_canvas_resize);

        // Enable scrolling with the mouse wheel (Windows/Linux)
        auto _on_mouse_wheel = [=](tk::Event event) {
            canvas.yview_scroll(-1 * (event.delta / 120), "units");
            };

        auto _on_shift_mouse_wheel = [=](tk::Event event) {
			canvas.xview_scroll(-1 * (event.delta / 120), "units");
            };

        canvas.bind_all("<MouseWheel>", _on_mouse_wheel);
        canvas.bind_all("<Shift-MouseWheel>", _on_shift_mouse_wheel);
    }

public:
    MainFrame(const tk::Misc& master) : MainFrame(create_canvas(master), 0)
    {

    }
};

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
        question_menu.grid({ .column = 2, .row = 3 });
        auto print_answers = [&]() { misc::printl("Selected Option: ", value_inside.get()); };
        auto submit_button = tk::Button({ .master = root, .command = print_answers, .text = "Submit" });
        submit_button.grid({ .column = 2, .row = 4 });

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

        auto tempframe = tk::Frame({ root });
        tempframe.grid({ .column = 0, .columnspan = 1, .row = 1, .sticky = "ns" });
        auto scrollable_frame = MainFrame(tempframe);
        for (auto i = 0; i < 50; i++)
            tk::Label({ .master = scrollable_frame, .text = std::format("Label {}", i + 1) }).pack({ .anchor = "w" });
        

        auto nb = ttk::Notebook({ .master = root });
        nb.add({ .child = tk::Button({ .text = "fxjcghcgkh" }), .text = "1" });
        nb.add({ .child = tk::Button({ .text = "cc bnkvhjl" }), .text = "2" });
        nb.grid({ .column = 0, .columnspan = 1, .row = 2, .sticky = "nswe" });

        root.after(3000, []() { misc::printl("after 3000ms"); });

        root.mainloop();
    }
    catch (const std::exception& ex)
    //catch (const std::logic_error& ex)
    { std::println("exception type: {}\n{}", typeid(ex).name(), ex.what()); }

    misc::printl("wroot.use_count() ", wroot.use_count());

    return 0;
}
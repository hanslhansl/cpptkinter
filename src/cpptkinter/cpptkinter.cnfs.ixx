module;
#include "../global.hpp"
export module cpptkinter:cpptkinter.cnfs;
import :utility;
import :_cpptkinter;
import :cpptkinter.detail;
import :cpptkinter.misc;
import :cpptkinter1;
import std;


export namespace cpptkinter::detail
{
    using pad_type = utility::extend_variants<detail::ScreenUnits, std::array<detail::ScreenUnits, 2>>::type;
    using visual_type = std::variant<std::string, std::tuple<std::string, long long>>;
}

/// @brief Contains structs to be passed to many of cpptkinter's functions.
///
/// Replaces Python's **kwargs.
export namespace cpptkinter::cnfs
{
    using opt_bool = opt<bool>;
    using opt_pad_type = opt<detail::pad_type>;
    using opt_visual_type = opt<detail::visual_type>;
    using opt_anchor = opt<detail::Anchor>;
    using opt_font_description = opt<detail::FontDescription>;
    using opt_cursor = opt<detail::Cursor>;
    using opt_image_spec = opt<detail::ImageSpec>;
    using opt_compound = opt<detail::Compound>;
    using opt_relief = opt<detail::Relief>;
    using opt_take_focus_value = opt<detail::TakeFocusValue>;
    using opt_text = opt<std::variant<double, std::string>>;
    using opt_xy_scrollcommand = opt<detail::XYScrollCommand>;
    using opt_variable = opt<cpptkinter::Variable>;
    using opt_entry_validate_command = opt<detail::EntryValidateCommand>;


    /// @brief Argument for Pack::pack_configure().
    struct pack_configure
    {
        /// widget - pack it after you have packed widget
        opt_master after;
        /// NSEW (or subset) - position widget according to given direction
        opt_string anchor;
        /// widget - pack it before you will pack widget
        opt_master before;
        /// bool - expand widget if parent size grows
        opt<std::size_t> expand;
        /// NONE or X or Y or BOTH - fill widget if widget grows
        opt_string fill;
        /// master - use master to contain this widget
        opt_master in;
        /// amount - add internal padding in x direction
        opt_screenunits ipadx;
        /// amount - add internal padding in y direction
        opt_screenunits ipady;
        /// amount - add padding in x direction
        opt_pad_type padx;
        /// amount - add padding in y direction
        opt_pad_type pady;
        /// TOP or BOTTOM or LEFT or RIGHT -  where to add this widget.
        opt_string side;
    };

    /// @brief Argument for Place::place_configure().
    struct place_configure
    {
        /// NSEW (or subset) - position anchor according to given direction
        opt_anchor anchor;
        /// "inside", "outside" or "ignore" - whether to take border width of master widget into account
        opt_string bordermode;
        /// master relative to which the widget is placed
        opt_master in;
        /// locate anchor of this widget at position x of master
        opt_screenunits x;
        /// locate anchor of this widget at position y of master
        opt_screenunits y;
        /// locate anchor of this widget between 0.0 and 1.0 relative to width of master (1.0 is right edge)
        opt<std::variant<std::string, double>> relx;
        /// locate anchor of this widget between 0.0 and 1.0 relative to height of master (1.0 is bottom edge)
        opt<std::variant<std::string, double>> rely;
        /// height of this widget in pixel
        opt_screenunits height;
        /// width of this widget in pixel
        opt_screenunits width;
        /// height of this widget between 0.0 and 1.0 relative to height of master (1.0 is the same height as the master)
        opt<std::variant<std::string, double>> relheight;
        /// width of this widget between 0.0 and 1.0 relative to width of master (1.0 is the same width as the master)
        opt<std::variant<std::string, double>> relwidth;
    };

    /// @brief Argument for Grid::grid_configure().
    struct grid_configure
    {
        /// number - use cell identified with given column (starting with 0)
        opt<std::size_t> column;
        /// number - this widget will span several columns
        opt<std::size_t> columnspan;
        /// master - use master to contain this widget
        opt_master in;
        /// number - use cell identified with given row (starting with 0)
        opt<std::size_t> row;
        /// number - this widget will span several rows
        opt<std::size_t> rowspan;
        /// amount - add internal padding in x direction
        opt_screenunits ipadx;
        /// amount - add internal padding in y direction
        opt_screenunits ipady;
        /// amount - add padding in x direction
        opt_pad_type padx;
        /// amount - add padding in y direction
        opt_pad_type pady;
        /// NSEW - if cell is larger on which sides will this widget stick to the cell boundary
        opt_string sticky;
    };

    /// @brief Argument for Menu::Menu().
    struct Menu
    {
        opt_master master;
        opt_string activebackground;
        opt_screenunits activeborderwidth;
        opt_string activeforeground;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_cursor cursor;
        opt_string disabledforeground;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_string name;
        opt<std::variant<std::string, std::function<void()>>> postcommand;
        opt_relief relief;
        opt_string selectcolor;
        opt_take_focus_value takefocus;
        opt_bool tearoff;
        opt<std::variant<std::string, std::function<void(const std::string&, const std::string&)>>> tearoffcommand;
        opt_string title;
        opt_string type;
    };

    /// @brief Argument for Menu::add_command().
    struct Menu_add_cascade;

    /// @brief Argument for Menu::add_radiobutton().
    template<typename T>
    struct Menu_add_checkbutton
    {
        opt_string accelerator;
        opt_string activebackground;
        opt_string activeforeground;
        opt_string background;
        opt_string bitmap;
        opt<int> columnbreak;
        opt<std::variant<std::string, std::function<void()>>> command;
        opt_compound compound;
        opt_font_description font;
        opt_string foreground;
        opt_bool hidemargin;
        opt_image_spec image;
        opt_bool indicatoron;
        opt_string label;
        T offvalue;
        T onvalue;
        opt_string selectcolor;
        opt_image_spec selectimage;
        opt_string state;
        opt<int> underline;
        opt_variable variable;
    };

    /// @brief Argument for Menu::add_command().
    struct Menu_add_command
    {
        opt_string accelerator;
        opt_string activebackground;
        opt_string activeforeground;
        opt_string background;
        opt_string bitmap;
        opt<int> columnbreak;
        opt<std::variant<std::string, std::function<void()>>> command;
        opt_compound compound;
        opt_font_description font;
        opt_string foreground;
        opt_bool hidemargin;
        opt_image_spec image;
        opt_string label;
        opt_string state;
        opt<int> underline;
    };

    /// @brief Argument for Menu::add_radiobutton().
    template<typename T>
    struct Menu_add_radiobutton
    {
        opt_string accelerator;
        opt_string activebackground;
        opt_string activeforeground;
        opt_string background;
        opt_string bitmap;
        opt<int> columnbreak;
        opt<std::variant<std::string, std::function<void()>>> command;
        opt_compound compound;
        opt_font_description font;
        opt_string foreground;
        opt_bool hidemargin;
        opt_image_spec image;
        opt_bool indicatoron;
        opt_string label;
        opt_string selectcolor;
        opt_image_spec selectimage;
        opt_string state;
        opt<int> underline;
        T value;
        opt_variable variable;
    };

    /// @brief Argument for Menu::add_separator().
    struct Menu_add_separator
    {
        opt_string background;
    };

    /// @brief Argument for Button::Button() and TypedButton::TypedButton().
    struct Button
    {
        opt_master master;
        opt_string activebackground;
        opt_string activeforeground;
        opt_anchor anchor;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_string bitmap;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt<detail::ButtonCommand> command;
        opt_compound compound;
        opt_cursor cursor;
        opt_string default_;
        opt_string disabledforeground;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_image_spec image;
        opt_string justify;
        opt_string name;
        opt_relief overrelief;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt<std::size_t> repeatdelay;
        opt<std::size_t> repeatinterval;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_text text;
        opt_variable textvariable;
        opt<std::size_t> underline;
        opt_screenunits width;
        opt_screenunits wraplength;
    };

    /// @brief Argument for Canvas::Canvas().
    struct Canvas
    {
        opt_master master;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt<double> closeenough;
        opt_bool confine;
        opt_cursor cursor;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string insertbackground;
        opt_screenunits insertborderwidth;
        opt<std::size_t> insertofftime;
        opt<std::size_t> insertontime;
        opt_screenunits insertwidth;
        opt_string name;
        //void offset;
        opt_relief relief;
        opt<std::array<detail::ScreenUnits, 4>> scrollregion;
        opt_string selectbackground;
        opt_screenunits selectborderwidth;
        opt_string selectforeground;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_screenunits width;
        opt_xy_scrollcommand xscrollcommand;
        opt_screenunits xscrollincrement;
        opt_xy_scrollcommand yscrollcommand;
        opt_screenunits yscrollincrement;
    };

    /// @brief Argument for Canvas::create_arc().
    struct Canvas_create_arc
    {

    };

    /// @brief Argument for Canvas::create_bitmap().
    struct Canvas_create_bitmap
    {

    };

    /// @brief Argument for Canvas::create_image().
    struct Canvas_create_image
    {

    };

    /// @brief Argument for Canvas::create_line().
    struct Canvas_create_line
    {

    };

    /// @brief Argument for Canvas::create_oval().
    struct Canvas_create_oval
    {

    };

    /// @brief Argument for Canvas::create_polygon().
    struct Canvas_create_polygon
    {

    };

    /// @brief Argument for Canvas::create_rectangle().
    struct Canvas_create_rectangle
    {

    };

    /// @brief Argument for Canvas::create_text().
    struct Canvas_create_text
    {

    };

    /// @brief Argument for Canvas::create_window().
    struct Canvas_create_window
    {
        opt_anchor anchor;
        opt_screenunits height;
        opt_string state;
        opt<std::variant<std::string, std::vector<std::string>>> tags;
        opt_screenunits width;
        opt<Misc> window;
    };

    /// @brief Argument for Checkbutton::Checkbutton() and TypedCheckbutton::TypedCheckbutton().
    template<typename T>
    struct Checkbutton
    {
        opt_master master;
        opt_string activebackground;
        opt_string activeforeground;
        opt_anchor anchor;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_string bitmap;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt<detail::ButtonCommand> command;
        opt_compound compound;
        opt_cursor cursor;
        opt_string disabledforeground;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_image_spec image;
        opt_bool indicatoron;
        opt_string justify;
        opt_string name;
        opt_relief offrelief;
        opt<T> offvalue;
        opt<T> onvalue;
        opt_relief overrelief;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt_string selectcolor;
        opt_image_spec selectimage;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_text text;
        opt_variable textvariable;
        opt_image_spec tristateimage;
        opt<T> tristatevalue;
        opt<std::size_t> underline;
        opt_variable variable;
        opt_screenunits width;
        opt_screenunits wraplength;
    };

    /// @brief Argument for Entry::Entry().
    struct Entry
    {
        opt_master master;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_cursor cursor;
        opt_string disabledbackground;
        opt_string disabledforeground;
        opt_bool exportselection;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string insertbackground;
        opt_screenunits insertborderwidth;
        opt<std::size_t> insertofftime;
        opt<std::size_t> insertontime;
        opt_screenunits insertwidth;
        opt_entry_validate_command invalidcommand;
        opt_entry_validate_command invcmd;
        opt_string justify;
        opt_string name;
        opt_string readonlybackground;
        opt_relief relief;
        opt_string selectbackground;
        opt_screenunits selectborderwidth;
        opt_string selectforeground;
        opt_string show;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_variable textvariable;
        opt_string validate;
        opt_entry_validate_command validatecommand;
        opt_entry_validate_command vcmd;
        opt_screenunits width;
        opt_xy_scrollcommand xscrollcommand;
    };

    /// @brief Argument for Frame::Frame().
    struct Frame
    {
        opt_master master;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_string class_;
        opt<std::variant<std::string, Misc>> colormap;
        opt_bool container;
        opt_cursor cursor;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string name;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt_take_focus_value takefocus;
        opt_visual_type visual;
        opt_screenunits width;
    };

    /// @brief Argument for Label::Label().
    struct Label
    {
        opt_master master;
        opt_string activebackground;
        opt_string activeforeground;
        opt_anchor anchor;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_string bitmap;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_compound compound;
        opt_cursor cursor;
        opt_string disabledforeground;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_image_spec image;
        opt_string justify;
        opt_string name;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_text text;
        opt_variable textvariable;
        opt<std::size_t> underline;
        opt_screenunits width;
        opt_screenunits wraplength;
    };

    /// @brief Argument for Listbox::Listbox().
    struct Listbox
    {
        opt_master master;
        opt_string activestyle;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_cursor cursor;
        opt_string disabledforeground;
        opt_bool exportselection;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string justify;
        opt_variable listvariable;
        opt_string name;
        opt_relief relief;
        opt_string selectbackground;
        opt_screenunits selectborderwidth;
        opt_string selectforeground;
        opt_string selectmode;
        opt_bool setgrid;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_screenunits width;
        opt_xy_scrollcommand xscrollincrement;
        opt_xy_scrollcommand yscrollincrement;
    };

    /// @brief Argument for Listbox::itemconfigure(long long, CNF&&).
    struct Listbox_itemconfigure
    {
        opt_string background;
        opt_string bg;
        opt_string foreground;
        opt_string fg;
        opt_string selectbackground;
        opt_string selectforeground;
    };
    /// @brief Argument for Radiobutton::Radiobutton() and TypedRadioButton::TypedRadioButton().
    template<typename T>
    struct Radiobutton
    {
        opt_master master;
        opt_string activebackground;
        opt_string activeforeground;
        opt_anchor anchor;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_string bitmap;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt<detail::ButtonCommand> command;
        opt_compound compound;
        opt_cursor cursor;
        opt_string disabledforeground;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_image_spec image;
        opt_string indicatoron;
        opt_string justify;
        opt_string name;
        opt_relief offrelief;
        opt_relief overrelief;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt_string selectcolor;
        opt_image_spec selectimage;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_text text;
        opt_variable textvariable;
        opt_image_spec tristateimage;
        opt<T> tristatevalue;
        opt<std::size_t> underline;
        opt<T> value;
        opt_variable variable;
        opt_screenunits width;
        opt_screenunits wraplength;
    };

    /// @brief Argument for Scale::Scale().
    struct Scale
    {
        opt_master master;
        opt_string activebackground;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt<double> bigincrement;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt<std::variant<std::string, std::function<void(std::string)>>> command;
        opt_cursor cursor;
        opt<long long> digits;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt<double> from;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string label;
        opt_screenunits length;
        opt_string name;
        opt_string orient;
        opt_relief relief;
        opt<long long> repeatdelay;
        opt<long long> repeatinterval;
        opt<double> resolution;
        opt_bool showvalue;
        opt_screenunits sliderlength;
        opt_relief sliderrelief;
        opt_string state;
        opt_take_focus_value takefocus;
        opt<double> tickinterval;
        opt<double> to;
        opt_string troughcolor;
        opt<std::variant<IntVar, DoubleVar>> variable;
        opt_screenunits width;
    };

    /// @brief Argument for Scrollbar::Scrollbar().
    struct Scrollbar
    {
        opt_master master;
        opt_string activebackground;
        opt_relief activerelief;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt<std::variant<std::string, std::function<void(std::vector<Tcl_Obj>)>>> command;
        opt_cursor cursor;
        opt_screenunits elementborderwidth;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_bool jump;
        opt_string name;
        opt_string orient;
        opt_relief relief;
        opt<std::size_t> repeatdelay;
        opt<std::size_t> repeatinterval;
        opt_take_focus_value takefocus;
        opt_string troughcolor;
        opt_screenunits width;
    };

    using text_index = std::variant<std::string, double, Misc>;

    /// @brief Argument for Text::Text().
    struct Text
    {
        opt_master master;
        opt_bool autoseparators;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_bool blockcursor;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_cursor cursor;
        opt<long long> endline;
        opt_bool exportselection;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string inactiveselectbackground;
        opt_string insertbackground;
        opt_screenunits insertborderwidth;
        opt<std::size_t> insertofftime;
        opt<std::size_t> insertontime;
        opt_string insertunfocussed;
        opt_bool insertwidth;
        opt<long long> maxundo;
        opt_string name;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt_string selectbackground;
        opt_screenunits selectborderwidth;
        opt_string selectforeground;
        opt_bool setgrid;
        opt_screenunits spacing1;
        opt_screenunits spacing2;
        opt_screenunits spacing3;
        opt<long long> startline;
        opt_string state;
        opt<std::variant<detail::ScreenUnits, std::vector<detail::ScreenUnits>>> tabs;
        opt_string tabstyle;
        opt_take_focus_value takefocus;
        opt_bool undo;
        opt<long long> width;
        opt_string wrap;
        opt_xy_scrollcommand xscrollcommand;
        opt_xy_scrollcommand yscrollcommand;
    };

    /// @brief Argument for Text::dump().
    struct Text_dump
    {
        text_index index1;
        opt<text_index> index2;
        bool all;
        bool image;
        bool mark;
        bool tag;
        bool text;
        bool window;
    };

    /// @brief Argument for Text::image_create().
    struct Text_image_create
    {
        text_index index;
        opt_string align;
        opt_image_spec image;
        opt_string name;
        opt_screenunits padx;
        opt_screenunits pady;
    };

    /// @brief Argument for Text::peer_create().
    struct Text_peer_create
    {
        std::string newPathName;
        opt_master master;
        opt_bool autoseparators;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_bool blockcursor;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_cursor cursor;
        opt<long long> endline;
        opt_bool exportselection;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string inactiveselectbackground;
        opt_string insertbackground;
        opt_screenunits insertborderwidth;
        opt<std::size_t> insertofftime;
        opt<std::size_t> insertontime;
        opt_string insertunfocussed;
        opt_bool insertwidth;
        opt<long long> maxundo;
        opt_string name;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt_string selectbackground;
        opt_screenunits selectborderwidth;
        opt_string selectforeground;
        opt_bool setgrid;
        opt_screenunits spacing1;
        opt_screenunits spacing2;
        opt_screenunits spacing3;
        opt<long long> startline;
        opt_string state;
        opt<std::variant<detail::ScreenUnits, std::vector<detail::ScreenUnits>>> tabs;
        opt_string tabstyle;
        opt_take_focus_value takefocus;
        opt_bool undo;
        opt<long long> width;
        opt_string wrap;
        opt_xy_scrollcommand xscrollcommand;
        opt_xy_scrollcommand yscrollcommand;
    };

    /// @brief Argument for Text::tag_configure().
    struct Text_tag_configure
    {
        std::string tagName;
        opt_string background;
        opt_string bgstipple;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_bool elide;
        opt_string fgstipple;
        opt_font_description font;
        opt_string foreground;
        opt_string justify;
        opt_screenunits lmargin1;
        opt_screenunits lmargin2;
        opt_string lmargincolor;
        opt_screenunits offset;
        opt_bool overstrike;
        opt_string overstrikefg;
        opt_relief relief;
        opt_screenunits rmargin;
        opt_string rmargincolor;
        opt_string selectbackground;
        opt_string selectforeground;
        opt_screenunits spacing1;
        opt_screenunits spacing2;
        opt_screenunits spacing3;
        opt<std::variant<detail::ScreenUnits, std::vector<detail::ScreenUnits>>> tabs;
        opt_string tabstyle;
        opt_bool underline;
        opt_string underlinefg;
        opt_string wrap;
    };

    /// @brief Argument for Text::search().
    struct Text_search
    {
        std::string pattern;
        text_index index;
        opt<text_index> stopindex;
        bool forwards;
        bool backwards;
        bool exact;
        bool regexp;
        bool nocase;
        opt_variable count;
        bool elide;
    };

    /// @brief Argument for Text::window_create().
    struct Text_window_create
    {
        text_index index;
        opt_string align;
        opt_string create;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_bool stretch;
        opt<std::variant<std::string, Misc>> window;
    };

    /// @brief Argument for PhotoImage::PhotoImage().
    struct PhotoImage
    {
        std::string name;
        opt_string data;
        opt_string format;
        opt_string file;
        opt<double> gamma;
        opt<std::size_t> height;
        opt<std::variant<long long, std::string>> palette;
        opt<std::size_t> width;
        opt<std::variant<Misc, std::shared_ptr<_cpptkinter::TkappObject>>> master;
    };

    /// @brief Argument for BitmapImage::BitmapImage().
    struct BitmapImage
    {
        std::string name;
        opt_string background;
        opt_string data;
        opt_string file;
        opt_string foreground;
        opt_string maskdata;
        opt_string maskfile;
        opt<std::variant<Misc, std::shared_ptr<_cpptkinter::TkappObject>>> master;
    };

    /// @brief Argument for Spinbox::Spinbox().
    struct Spinbox
    {
        opt_master master;
        opt_string activebackground;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_string buttonbackground;
        opt_cursor buttoncursor;
        opt_relief buttondownrelief;
        opt_string buttonuprelief;
        opt<std::variant<std::string, std::function<void()>>> command;  //
        opt_cursor cursor;
        opt_string disabledbackground;
        opt_string disabledforeground;
        opt_bool exportselection;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_string format;
        opt<double> from;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt<double> increment;
        opt_string insertbackground;
        opt_screenunits insertborderwidth;
        opt<std::size_t> insertofftime;
        opt<std::size_t> insertontime;
        opt_screenunits insertwidth;
        opt_entry_validate_command invalidcommand;
        opt_entry_validate_command invcmd;
        opt_string justify;
        opt_string name;
        opt_string readonlybackground;
        opt_relief relief;
        opt<std::size_t> repeatdelay;
        opt<std::size_t> repeatinterval;
        opt_string selectbackground;
        opt_screenunits selectborderwidth;
        opt_string selectforeground;
        opt_string state;
        opt_take_focus_value takefocus;
        opt_variable textvariable;
        opt<double> to;
        opt_string validate;
        opt_entry_validate_command validatecommand;
        opt_entry_validate_command vcmd;
        opt<std::vector<std::string>> values;
        opt_screenunits width;
        opt_bool wrap;
        opt_xy_scrollcommand xscrollcommand;
    };

    /// @brief Argument for LabelFrame::LabelFrame().
    struct LabelFrame
    {
        opt_master master;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_string class_;
        opt<std::variant<std::string, Misc>> colormap;
        opt_bool container;
        opt_cursor cursor;
        opt_string fg;
        opt_font_description font;
        opt_string foreground;
        opt_screenunits height;
        opt_string highlightbackground;
        opt_string highlightcolor;
        opt_screenunits highlightthickness;
        opt_string labelanchor;
        opt_master labelwidget;
        opt_string name;
        opt_screenunits padx;
        opt_screenunits pady;
        opt_relief relief;
        opt_take_focus_value takefocus;
        opt_text text;
        opt_visual_type visual;
        opt_screenunits width;
    };

    /// @brief Argument for PanedWindow::PanedWindow().
    struct PanedWindow
    {
        opt_master master;
        opt_string background;
        opt_screenunits bd;
        opt_string bg;
        opt_screenunits border;
        opt_screenunits borderwidth;
        opt_cursor cursor;
        opt_screenunits handlepad;
        opt_screenunits handlesize;
        opt_screenunits height;
        opt_string name;
        opt_bool opaqueresize;
        opt_string orient;
        opt_string proxybackground;
        opt_screenunits proxyborderwidth;
        opt_relief proxyrelief;
        opt_relief relief;
        opt_cursor sashcursor;
        opt_screenunits sashpad;
        opt_relief sashrelief;
        opt_screenunits sashwidth;
        opt_bool showhandle;
        opt_string width;
    };
}
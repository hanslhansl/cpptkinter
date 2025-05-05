module;
export module cpptkinter:ttk.cnfs;
import :cpptkinter.cnfs;
import :cpptkinter.widget.base;
import std;

export namespace cpptkinter::detail
{
    using Padding = utility::extend_variants<ScreenUnits, std::array<ScreenUnits, 2>, std::array<ScreenUnits, 3>, std::array<ScreenUnits, 4>>::type;
}

export namespace cpptkinter::cnfs
{
    using opt_padding = opt<detail::Padding>;

	/// @brief Argument for ttk::Notebook::Notebook().
    struct Notebook
    {
        opt_master master;
        opt_string class_;
        opt_cursor cursor;
        opt<std::size_t> height;
        opt_string name;
        opt_padding padding;
        opt_string style;
		opt_take_focus_value takefocus;
		opt<std::size_t> width;
    };

	/// @brief Argument for ttk::Notebook::add().
    struct Notebook_add
    {
        cpptkinter::Widget child;
        opt_string state;
        opt_string sticky;
        opt<std::vector<std::size_t>> padding;
        opt_string text;
        opt_string image;
        opt_string compound;
        opt<long long> underline;
    };

	/// @brief Argument for ttk::Notebook::insert().
    struct Notebook_insert
    {
        std::variant<std::size_t, std::string> pos;
        cpptkinter::Widget child;
        opt_string state;
        opt_string sticky;
        opt<std::vector<std::size_t>> padding;
        opt_string text;
        opt_string image;
        opt_string compound;
        opt<long long> underline;
    };

	/// @brief Argument for ttk::Notebook::tab().
    struct Notebook_tab
    {
        std::variant<std::size_t, std::string> tab_id;
        opt_string state;
        opt_string sticky;
        opt<std::vector<std::size_t>> padding;
        opt_string text;
        opt_string image;
        opt_string compound;
        opt_string underline;
    };

	/// @brief Argument for ttk::Treeview::Treeview().
	struct Treeview
	{
		opt_master master;
        opt_string class_;
		opt<std::variant<long long, std::string, std::vector<std::string>, std::vector<long long>, std::vector<std::variant<long long, std::string>>>> columns;
		opt_cursor cursor;
        opt<std::variant<long long, std::string, std::vector<std::string>, std::vector<long long>>> displaycolumns;
		opt<std::size_t> height;
        opt_string name;
        opt_padding padding;
        opt_string selectmode;
        opt_string selecttype;
        opt<std::variant<std::string, std::vector<std::string>>> show;
        opt_bool striped;
        opt_string style;
        opt_take_focus_value takefocus;
        opt_xy_scrollcommand xscrollcommand;
        opt_xy_scrollcommand yscrollcommand;
	};

    /// @brief Argument for ttk::Treeview::column().
    struct Treeview_column
    {
        std::variant<std::string, long long> column;
		/// Specifies how the text in this column should be aligned with respect to the cell. One of the standard Tk anchor values.
        opt_anchor anchor;
        /// The minimum width of the column in pixels.
        /// The treeview widget will not make the column any smaller than -minwidth when the widget is resized or the user drags a heading column separator.
        /// Default is 20 pixels. 
        opt<long long> minwidth;
        /// Specifies whether or not a column separator should be drawn to the right of the column. Default is false. 
        opt_bool separator;
        /// Specifies whether or not the column width should be adjusted when the widget is resized or the user drags a heading column separator.
        /// By default columns are stretchable.
        opt_bool stretch;
        /// The width of the column in pixels. Default is 200 pixels.
        /// The specified column width may be changed by Tk in order to honor -stretch and/or -minwidth, or when the widget is resized or the user drags a heading column separator. 
        opt<long long> width;
    };

	/// @brief Argument for ttk::Treeview::heading().
    struct Treeview_heading
    {
		std::variant<std::string, long long> column;
        /// Specifies how the heading text should be aligned. One of the standard Tk anchor values. 
		opt_anchor anchor;
        /// A script to evaluate when the heading label is pressed. 
        opt<std::variant<std::string, std::function<void()>>> command;
        /// Specifies an image to display to the right of the column heading.
		opt_image_spec image;
        /// The text to display in the column heading
		opt_string text;
    };

	/// @brief Argument for ttk::Treeview::insert().
    struct Treeview_insert
    {
         std::variant<std::string, long long> parent;
         std::variant<std::string, long long> index;
         opt<std::variant<std::string, long long>> id;
         opt<std::size_t> height;
         opt_bool hidden;
         opt_image_spec image;
         opt_anchor imageanchor;
         opt_bool open;
         opt<std::vector<std::string>> tags;
         opt_string text;
         opt<std::variant<long long, std::string, std::vector<std::string>, std::vector<long long>, std::vector<std::variant<long long, std::string>>>> values;
    };
}
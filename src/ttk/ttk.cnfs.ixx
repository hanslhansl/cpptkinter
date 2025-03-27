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

	/// @brief Argument for Notebook::Notebook().
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

	/// @brief Argument for Notebook::add().
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

	/// @brief Argument for Notebook::insert().
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

	/// @brief Argument for Notebook::tab().
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
}
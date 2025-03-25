module;
#include "../global.hpp"
export module cpptkinter:cpptkinter.detail;
import :utility;
import :_cpptkinter;
import std;


export namespace cpptkinter
{
    using _cpptkinter::Tcl_Obj;
    using _cpptkinter::TclError;

    struct Image;
    class Tk;

    enum class EventType
    {
        KeyPress = 2,
        Key = KeyPress,
        KeyRelease = 3,
        ButtonPress = 4,
        Button = ButtonPress,
        ButtonRelease = 5,
        Motion = 6,
        Enter = 7,
        Leave = 8,
        FocusIn = 9,
        FocusOut = 10,
        Keymap = 11,
        Expose = 12,
        GraphicsExpose = 13,
        NoExpose = 14,
        Visibility = 15,
        Create = 16,
        Destroy = 17,
        Unmap = 18,
        Map = 19,
        MapRequest = 20,
        Reparent = 21,
        Configure = 22,
        ConfigureRequest = 23,
        Gravity = 24,
        ResizeRequest = 25,
        Circulate = 26,
        CirculateRequest = 27,
        Property = 28,
        SelectionClear = 29,
        SelectionRequest = 30,
        Selection = 31,
        Colormap = 32,
        ClientMessage = 33,
        Mapping = 34,
        VirtualEvent = 35,
        Activate = 36,
        Deactivate = 37,
        MouseWheel = 38
    };
}

/// @brief Implementation details of cpptkinter.
/// 
/// This namespace contains implementation details of cpptkinter as well as the implementation of tkinter entities prefixed with an underscore (e.g. tk._print_command()).
export namespace cpptkinter::detail
{
    struct Tk_impl;
    template<typename W>
    struct set_get_proxy
    {
        W widget;
        std::string key;

        template<typename T>
            requires requires { widget._setitem_(key, std::declval<T>()); }
        void set(T&& value)
        {
            this->widget._setitem_(this->key, std::forward<T>(value));
        }
        template<typename T>
            requires requires { widget._setitem_(key, std::declval<T>()); }
        void operator=(T&& value)
        {
            this->set(std::forward<T>(value));
        }

        template<typename R>
            requires requires { widget._getitem_(key, std::type_identity<R>{}); }
        R get()
        {
            return this->widget._getitem_(this->key, std::type_identity<R>{});
        }
        template<typename R>
            requires requires { widget._getitem_(key, std::type_identity<R>{}); }
        operator R()
        {
            return this->get<R>();
        }
    };

    using Anchor = std::string;
    using ButtonCommand = std::variant<std::string, std::function<void()>>;
    using Compound = std::string;
    using Cursor = std::variant<std::string,
        std::tuple<std::string>,
        std::tuple<std::string, std::string>,
        std::tuple<std::string, std::string, std::string>,
        std::tuple<std::string, std::string, std::string, std::string>>;
    using EntryValidateCommand = std::variant<std::string, std::vector<std::string>, std::function<bool()>>;
    using ImageSpec = std::variant<std::string, Image>;
    DEVIATING_IMPLEMENTATION_WARNING("_ImageSpec not done");
    using Relief = std::string;
    using ScreenUnits = std::variant<long long, double, std::string>;
    /// the callback takes 2 strings which contain floats
    using XYScrollCommand = std::variant<std::string, std::function<void(std::string, std::string)>>;
    using TakeFocusValue = std::variant<bool, std::function<bool(const std::string&)>>;
    using FontDescription = std::variant<std::string, /*Font, */
        std::tuple<std::string>,
        std::tuple<std::string, long long>,
        std::tuple<std::string, long long, std::string>,
        std::tuple<std::string, long long, std::vector<std::string>>/*, _tkinter.Tcl_Obj*/>;
    DEVIATING_IMPLEMENTATION_WARNING("_FontDescription not done");

    /// @brief set to true to print executed Tcl / Tk commands
    bool _debug =
#ifdef NDEBUG
        false;
#else
        true;
#endif
    bool _support_default_root = true;
    std::shared_ptr<Tk_impl> _default_root = nullptr;
    long long _varnum = 0;
    long long _checkbutton_count = 0;

    std::size_t tcl_command_name_counter = 0;
    const std::set<char> tcl_forbidden_chars{ ' ', '{', '}', '[', ']', '(', ')', '"', '\\', '$', ';', '|', '&', '*', '~', '<', '>', ':', '\'', '`' , ',' };

    /// @brief Internal class.
    /// 
    /// Stores function to call when some user defined Tcl function is called e.g. after an event occurred.
    template<typename R, typename...Args>
    struct CallWrapper;

    void _print_command(std::vector<std::string> cmd)
    {
        for (auto& c : cmd)
            std::cerr << c << " ";
        std::cerr << std::endl;
    }

    /// Internal function
    void _tkerror(std::string)
    {

    }

    /// Internal function. Calling it will throw std::runtime_error.
    void _exit()
    {
        throw utility::construct_exception<std::runtime_error>("");
    }

    /// @brief Check if a weak_ptr is empty.
    /// 
    /// @tparam T The type of the weak_ptr.
    /// @param weak The weak_ptr to check.
    template<typename T>
    bool weak_ptr_is_empty(const std::weak_ptr<T>& weak)
    {
        return !weak.owner_before(std::weak_ptr<T>{}) && !std::weak_ptr<T>{}.owner_before(weak);
    }

    template<typename A, typename V, typename Conv = const std::nullopt_t&>
    A _splitdict_to_aggregate(std::map<std::string, V>&& v, bool cut_minus = true, Conv&& conv = std::nullopt)
    {
        if (v.size() != reflect::size<A>())
            throw utility::construct_exception<std::invalid_argument>(std::format("map has {} elements but type {} has {} members", v.size(), reflect::type_name<A>(), reflect::size<A>()));

        if (cut_minus)
        {
            std::map<std::string, V> temp_v{};
            while (!v.empty())
            {
                auto&& node = v.extract(v.begin());
                auto&& key = node.key();

                if (key.starts_with('-'))
                    key = key.substr(1);

                temp_v.insert(temp_v.end(), std::move(node));
            }
            v = std::move(temp_v);
        }

        auto inner_visitor = [&v, &conv]<std::size_t I>()->reflect::member_type<I, A> {
            auto key = /*rfl::fields<A>()[I].name()*/reflect::member_name<I, A>();
            auto&& node = v.extract(std::string(key));
            if (node.empty())
                throw utility::construct_exception<std::invalid_argument>(std::format("key '{}' not found", key));
            auto&& mapped = node.mapped();

            if constexpr (std::same_as<Conv, const std::nullopt_t&>)
            {
                if constexpr (hhh::meta::is_template_instance<V, std::variant>)
                    return std::get<reflect::member_type<I, A>>(std::move(mapped));
                else
                    return std::move(mapped);
            }
            else
            {
                return conv.template operator() < reflect::member_type<I, A> > (std::move(mapped));
            }
        };

        auto visitor = [&inner_visitor]<std::size_t...I>(std::integer_sequence<std::size_t, I...>)->A {
            return A{ inner_visitor.template operator() < I > ()... };
        };

        return visitor(std::make_index_sequence<reflect::size<A>()>{});/**/
    }

    template<typename T>
    T pack_grid_info(auto&& self, const std::string& a1, const std::string& a2, const std::string& a3)
    {
        using V = std::variant<long long, std::string, _cpptkinter::tk_window_type>;
        auto map = self.tk->template call<std::map<std::string, V>>(a1, a2, a3);

        if (map.size() != reflect::size<T>())
            throw utility::construct_exception<std::invalid_argument>(std::format("map has {} elements but type {} has {} members", map.size(), reflect::type_name<T>(), reflect::size<T>()));

        auto converter = [&self]<typename T2>(V && v)->T2
        {
            if constexpr (std::same_as<T2, Misc>)
                return self.nametowidget(std::get<_cpptkinter::tk_window_type>(std::move(v)));
            else if constexpr (std::same_as<T2, bool>)
                return std::get<long long>(std::move(v));
            else
                return std::get<T2>(std::move(v));
        };
        return detail::_splitdict_to_aggregate<T>(std::move(map), true, converter);
    }

    /// @brief Concept for ranges of types satisfying AsObjConcept.
    template<typename R>
    concept range_convertible_to_tcl_obj = std::ranges::range<R> && AsObjConcept<std::ranges::range_value_t<R>>;

    /// @brief Satisfied if R is a range of a type convertible to std::string.
    template<typename R>
    concept sized_range_convertible_to_string = std::ranges::sized_range<R> && std::convertible_to<std::ranges::range_value_t<R>, std::string>;

    /// @brief Concept for types allowed as indices to e.g. Entry and Listbox.
    template<typename T>
    concept index = utility::union_arg<T, long long, std::string>;
    constexpr auto to_index = utility::to_union_arg<long long, std::string>;

    /// @brief Concept for types allowed as screenunits
    template<typename T>
    concept screenunits_arg = utility::union_arg<T, long long, double, std::string>;
    constexpr auto to_screenunits_arg = utility::to_union_arg<long long, double, std::string>;

    /// @brief std::string or long long
    template<typename T>
    concept tag_or_id_arg = utility::union_arg<T, long long, std::string>;
    constexpr auto to_tag_or_id = utility::to_union_arg<long long, std::string>;

    /// @brief Concept for types allowed as indices to Text.
    template<typename T>
    concept text_index = utility::union_arg<T, double, std::string>;
	constexpr auto to_text_index = utility::to_union_arg<double, std::string>;

    template<std::derived_from<Misc> T>
    struct Event
    {
        /// serial number of event
        long long serial;
        /// mouse button pressed(ButtonPress, ButtonRelease)
        long long num;
        /// whether the window has the focus (Enter, Leave), if invalid: false
        bool focus;
        /// height of the exposed window (Configure, Expose)
        long long height;
        /// width of the exposed window (Configure, Expose)
        long long width;
        /// keycode of the pressed key (KeyPress, KeyRelease)
        long long keycode;
        /// state of the event as a number (ButtonPress, ButtonRelease, Enter, KeyPress, KeyRelease, Leave, Motion) or as a string (Visibility)
        std::variant<long long, std::string> state;
        /// when the event occurred
        long long time;
        /// x - position of the mouse
        long long x;
        /// y - position of the mouse
        long long y;
        /// x - position of the mouse on the screen (ButtonPress, ButtonRelease, KeyPress, KeyRelease, Motion)
        long long x_root;
        /// y - position of the mouse on the screen (ButtonPress, ButtonRelease, KeyPress, KeyRelease, Motion)
        long long y_root;
        /// pressed character (KeyPress, KeyRelease)
        std::string char_;
        /// see X / Windows documentation, if invalid: false
        bool send_event;
        /// keysym of the event as a string (KeyPress, KeyRelease)
        std::string keysym;
        /// keysym of the event as a number (KeyPress, KeyRelease)
        long long keysym_num;
        /// type of the event as a number
        EventType type;
        /// widget in which the event occurred
        T widget;
        /// delta of wheel movement (MouseWheel)
        long long delta;
    };

    template<typename T>
    concept Variable_mode_concept = std::convertible_to<T, std::string> || (utility::is_vector<T> && std::convertible_to<typename T::value_type, std::string>);

    /// @brief Return type of Misc::grid_columnconfigure() and Misc::grid_rowconfigure().
    struct grid_column_row_configure_return
    {
        long long minsize;
        long long pad;
        std::string uniform;
        long long weight;
    };

    Tk _get_default_root(const std::string& what = {});
}

export namespace cpptkinter
{
    /// @brief Container for the properties of an event.
    /// 
    /// Instances of this type are generated if one of the following events occurs:
    /// 
    /// KeyPress, KeyRelease - for keyboard events\n
    /// ButtonPress, ButtonRelease, Motion, Enter, Leave, MouseWheel - for mouse events\n
    /// Visibility, Unmap, Map, Expose, FocusIn, FocusOut, Circulate, Colormap, Gravity, Reparent, Property, Destroy, Activate, Deactivate - for window events
    /// 
    /// If a callback function for one of these events is registered using bind, bind_all, bind_class, or tag_bind, the callback is called with an Event as first argument.
    using Event = detail::Event<Misc>;
}
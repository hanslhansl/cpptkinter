/// @file cpptkinter.ixx
/// @brief Implements __init__.py.

module;
#include "global.hpp"
#include <reflect/reflect.hpp>
#include <range/v3/all.hpp>
#undef TK_VERSION
#undef TCL_VERSION
export module cpptkinter;
export import :constants;
export import :utility;
export import :_cpptkinter;
import std;
import hhh;

using namespace std::literals;

#define REF_TO_IMPL(member) decltype(impl::member)& member

#define DEFINE_ASSIGNMENT_OPERATOR(cl) \
    cl& operator=(const cl& other) \
    { \
        std::destroy_at(this); \
        return *std::construct_at(this, other); \
    }
// expanded DEFINE_ASSIGNMENT_OPERATOR(cl) macro to make COMMA macro work
#define CNF_CONSTRUCTOR_AND_ASSIGNMENT(cl, cnf_type, str, base) \
    using constructor_cnf = cnf_type;   \
    template<cnfs::is_cnf CNF = constructor_cnf> \
    cl(CNF&& cnf = {}) : cl(std::make_shared<impl>()) \
    {   \
        this->_init_(str, std::forward<CNF>(cnf));  \
    }   \
    cl() : cl(constructor_cnf{}) { }   \
    cl& operator=(const cl& other) \
    {   \
        std::destroy_at(this);  \
        return *std::construct_at(this, other); \
    }   \
    using base::base

using substitute_long_long = const std::variant<long long, std::string>&;
#define MISC_SUBSTITUTE_PARAMETERS  const std::string& nsign, substitute_long_long b, substitute_long_long f, substitute_long_long h, substitute_long_long k, const std::string& s, \
                                    const std::string& t, substitute_long_long w, const std::string& x, const std::string& y, const std::string& A, substitute_long_long E, const std::string& K, \
                                    substitute_long_long N, const std::string& W, substitute_long_long T, const std::string& X, const std::string& Y, substitute_long_long D
#define MISC_SUBSTITUTE_ARGUMENTS nsign, b, f, h, k, s, t, w, x, y, A, E, K, N, W, T, X, Y, D


#if defined(__cpp_lib_ranges_stride) && defined(__cpp_lib_ranges_to_container) && defined(__cpp_lib_ranges) && defined(__cpp_lib_ranges_zip) && defined(__cpp_lib_ranges_join_with)
using std::views::stride;
using std::ranges::to;
using std::views::drop;
using std::views::zip;
using std::ranges::join_with_view;
#else
using ranges::views::stride;
using ranges::to;
using ranges::views::drop;
using ranges::views::zip;
using ranges::join_with_view;
#endif

export namespace cpptkinter
{
    using namespace constants;

    using _cpptkinter::TclError;

    class Variable;
    class Tk;

    const auto TkVersion = std::atof(_cpptkinter::TK_VERSION.data());
    const auto TclVersion = std::atof(_cpptkinter::TCL_VERSION.data());

    //constexpr auto wantobjects = 1; deprecated, always true
    using _cpptkinter::READABLE;
    using _cpptkinter::WRITABLE;
    using _cpptkinter::EXCEPTION;

    /// @brief Implementation details of cpptkinter.
    /// 
    /// This namespace contains implementation details of cpptkinter as well as the implementation of tkinter entities prefixed with an underscore (e.g. tk._print_command()).
    namespace detail
    {
        using namespace _cpptkinter::detail;

        struct Tk_impl;
        template<std::derived_from<Misc> W>
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
        using ImageSpec = std::variant<std::string/*, _Image*/>;
        DEVIATING_IMPLEMENTATION_WARNING("_ImageSpec not done");
        using Relief = std::string;
        using ScreenUnits = std::variant<long long, double, std::string>;
        using XYScrollCommand = std::variant<std::string, std::function<void(double, double)>>;
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
        constexpr bool _support_default_root = true;
        std::shared_ptr<Tk_impl> _default_root = nullptr;
        long long _varnum = 0;
        long long _checkbutton_count = 0;

        size_t tcl_command_name_counter = 0;
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
        void _tkerror()
        {

        }

        /// Internal function. Calling it will throw std::runtime_error.
        void _exit()
        {
            throw detail::construct_exception<std::runtime_error>("");
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
                throw detail::construct_exception<std::invalid_argument>(std::format("map has {} elements but type {} has {} members", v.size(), reflect::type_name<A>(), reflect::size<A>()));

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

            auto inner_visitor = [&v, &conv]<size_t I>()->reflect::member_type<I, A> {
                auto key = /*rfl::fields<A>()[I].name()*/reflect::member_name<I, A>();
                auto&& node = v.extract(std::string(key));
                if (node.empty())
                    throw detail::construct_exception<std::invalid_argument>(std::format("key '{}' not found", key));
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
                    return conv.template operator()<reflect::member_type<I, A>>(std::move(mapped));
                }
                };

            auto visitor = [&inner_visitor]<size_t...I>(std::integer_sequence<size_t, I...>)->A {
                return A{ inner_visitor.template operator()<I>()... };
            };

            return visitor(std::make_index_sequence<reflect::size<A>()>{});/**/
        }

        Tk _get_default_root(const std::string& what = {});

        /// @brief Concept for ranges of types satisfying AsObjConcept.
        template<typename R>
        concept range_of_AsObj = std::ranges::range<R> && AsObjConcept<std::ranges::range_value_t<R>>;

        /// @brief Concept for types allowed as indices to e.g. Entry and Listbox.
        template<typename T>
        concept index = AsObjConcept<T> && (std::convertible_to<T, long long> || std::convertible_to<T, std::string>);

        long long to_index_impl(long long);
        std::string to_index_impl(std::string);

        template<index T>
        decltype(auto) to_index(const T& t)
        {
            if constexpr (std::same_as<T, long long>)
                return t;
            else if constexpr (std::same_as<T, std::string>)
                return t;
            else
                return static_cast<decltype(to_index_impl(t))>(t);
        }
    }

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

    /// @brief Container for the properties of an event.
    /// 
    /// Instances of this type are generated if one of the following events occurs:
    /// 
    /// KeyPress, KeyRelease - for keyboard events\n
    /// ButtonPress, ButtonRelease, Motion, Enter, Leave, MouseWheel - for mouse events\n
    /// Visibility, Unmap, Map, Expose, FocusIn, FocusOut, Circulate, Colormap, Gravity, Reparent, Property, Destroy, Activate, Deactivate - for window events
    /// 
    /// If a callback function for one of these events is registered using bind, bind_all, bind_class, or tag_bind, the callback is called with an Event as first argument.
    template<typename T>
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

    /// @brief Initialize cpptkinter.
    /// 
    /// This function must be called before doing anything else.
    /// @param argc: the first argument of startup function main().
    /// @param argv: the second argument of startup function main().
    /// @param tcl_library: Path to the Tcl library. Only used if TCL_CORE_LIBRARY_IS_EMBEDDED is false.
    void init(int argc, char* argv[], const std::string& tcl_library = {})
    {
        _cpptkinter::detail::argc = argc;
        _cpptkinter::detail::argv = argv;

        _cpptkinter::init(tcl_library);
    }

    /// @brief Provides functions for the communication with the window manager.
    struct Wm
    {
        /// @brief Instruct the window manager to set the aspect ratio (width/height) of this widget to be between MINNUMER / MINDENOM and MAXNUMER / MAXDENOM.
        void wm_aspect(this auto&& self, long long minNumer, long long minDenom, long long maxNumer, long long maxDenom)
        {
            self.tk->call("wm", "aspect", self._w, minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @brief Removes any aspect ratio restrictions.
        ///
        /// Should only be called with 4 empty strings.
        void wm_aspect(this auto&& self, const std::string& minNumer, const std::string& minDenom, const std::string& maxNumer, const std::string& maxDenom)
        {
            self.tk->call("wm", "aspect", self._w, minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @brief Returns the current aspect ratio restriction (if any).
        std::optional<std::array<long long, 4>> wm_aspect(this auto&& self)
        {
            auto res = self.tk->template call<std::variant<std::array<long long, 4>, std::string>>("wm", "aspect", self._w);
            if (std::holds_alternative<std::string>(res))
                return {};
            return std::get<std::array<long long, 4>>(res);
        }
        /// @copydoc wm_aspect(this auto&&, long long, long long, long long, long long)
        void aspect(this auto&& self, long long minNumer, long long minDenom, long long maxNumer, long long maxDenom)
        {
            self.wm_aspect(minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @copydoc wm_aspect(this auto&&, const std::string&, const std::string&, const std::string&, const std::string&)
        void aspect(this auto&& self, const std::string& minNumer, const std::string& minDenom, const std::string& maxNumer, const std::string& maxDenom)
        {
            self.wm_aspect(minNumer, minDenom, maxNumer, maxDenom);
        }
        /// @copydoc wm_aspect(this auto&&)
        std::optional<std::array<long long, 4>> aspect(this auto&& self)
        {
            return self.wm_aspect();
        }

        /// @brief This subcommand returns or sets platform specific attributes
        ///
        /// The first form returns a list of the platform specific flags and their values.
        /// The second form returns the value for the specific option.
        /// The third form sets one or more of the values. The values are as follows: 
        /// 
        /// On Windows,
        /// - disabled gets or sets whether the window is in a disabled state.
        /// - toolwindow gets or sets the style of the window to toolwindow (as defined in the MSDN).
        /// - topmost gets or sets whether this is a topmost window (displays above all other windows).
        ///
        /// On Macintosh, XXXXX
        ///
        /// On Unix, there are currently no special attribute values.
        std::map<std::string, std::variant<std::string, double, long long>> wm_attributes(this auto&& self)
        {
            using V = std::variant<std::string, double, long long>;
            auto data = self.tk->template call<std::vector<V>>("wm", "attributes", self._w);

            auto lambda = [](V& var) { 
                auto&& key = std::get<std::string>(std::move(var));
                if (key.starts_with('-'))
                    key = key.substr(1);
                return key;
                };

            return std::map<std::string, V>(std::from_range, std::views::zip(
                data | /*std::views::*/stride(2) | std::views::transform(lambda),
                data | std::views::drop(1) | /*std::views::*/stride(2)
            ));
        }
        /// @copydoc wm_attributes(this auto&&)
        std::map<std::string, std::variant<std::string, double, long long>> attributes(this auto&& self)
        {
            return self.wm_attributes();
        }

        /// Store NAME in WM_CLIENT_MACHINE property of this widget. Return current value.
        void wm_client(this auto&& self, const std::string& name)
        {
            self.tk->call("wm", "client", self._w, name);
        }
        /// Get the last name set in a wm client command
        std::string wm_client(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "client", self._w);
        }
        /// @copydoc wm_client(this auto&&, const std::string&)
        void client(this auto&& self, const std::string& name)
        {
            self.wm_client(name);
        }
        /// @copydoc wm_client(this auto&&)
        std::string client(this auto&& self)
        {
            return self.wm_client();
        }

        /// @brief Store list of window names (WLIST) into WM_COLORMAPWINDOWS property of this widget.
        /// 
        /// This list contains windows whose colormaps differ from their parents. Return current list of widgets if WLIST is empty.
        void wm_colormapwindows(this auto&& self);
        /// @copydoc wm_colormapwindows
        void colormapwindows(this auto&& self);

        /// @brief Store VALUE in WM_COMMAND property.
        /// 
        /// It is the command which shall be used to invoke the application.Return current command if VALUE is None.
        void wm_command();
        /// @copydoc wm_command
        void command();

        /// @brief Deiconify this widget.
        /// 
        /// If it was never mapped it will not be mapped. On Windows it will raise this widget and give it the focus.
        void wm_deiconify(this auto&& self)
        {
            self.tk->call("wm", "deiconify", self._w);
        }
        /// @copydoc wm_deiconify
        void deiconify(this auto&& self)
        {
            self.wm_deiconify();
        }

        /// @brief Set focus model.
        /// 
        /// "active" means that this widget will claim the focus itself, "passive" means that the window manager shall give the focus.
        void wm_focusmodel(this auto&& self, const std::string& model)
        {
            self.tk->call("wm", "focusmodel", self._w, model);
        }
        /// @brief Return current focus model.
        std::string wm_focusmodel(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "focusmodel", self._w);
        }
        /// @copydoc wm_focusmodel(this auto&&, const std::string&)
        void focusmodel(this auto&& self, const std::string& model)
        {
            return self.wm_focusmodel(model);
        }
        /// @copydoc wm_focusmodel(this auto&&)
        std::string focusmodel(this auto&& self)
        {
            return self.wm_focusmodel();
        }

        /// @brief The window will be unmapped from the screen and will no longer be managed by wm.
        /// 
        /// Toplevel windows will be treated like frame windows once they are no longer managed by wm, however,
        /// the menu option configuration will be remembered and the menus will return once the widget is managed again.
        void wm_forget(this auto&& self, const std::derived_from<Misc> auto& window)
        {
            self.tk->call("wm", "forget", window);
        }
        /// @copydoc wm_forget
        void forget(this auto&& self, const std::derived_from<Misc> auto& window)
        {
            self.wm_forget(window);
        }

        /// Return identifier for decorative frame of this widget if present.
        void wm_frame(this auto&& self)
        {
            self.tk->call("wm", "frame", self._w);
        }
        /// @copydoc wm_frame
        void frame(this auto&& self)
        {
            self.wm_frame();
        }

        /// @brief Set geometry to NEWGEOMETRY of the form =widthxheight+x+y.
        void wm_geometry(this auto&& self, const std::string& newGeometry)
        {
            self.tk->call("wm", "geometry", self._w, newGeometry);
        }
        /// @brief Get current geometry.
        std::string wm_geometry(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "geometry", self._w);
        }
        /// @copydoc wm_geometry(this auto&&, const std::string&)
        void geometry(this auto&& self, const std::string& newGeometry)
        {
            return self.wm_geometry(newGeometry);
        }
        /// @copydoc wm_geometry(this auto&&)
        std::string geometry(this auto&& self)
        {
            return self.wm_geometry();
        }

        /// @brief Manage window as a gridded window.
        /// 
        /// WIDTHINC and HEIGHTINC are the width and height of a grid unit in pixels. BASEWIDTH and BASEHEIGHT are the number of grid units requested in Tk_GeometryRequest.
        void wm_grid(this auto&& self, long long baseWidth, long long baseHeight, long long widthInc, long long heightInc)
        {
            self.tk->call("wm", "grid", self._w, baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @brief Window will no longer be managed as a gridded window
        ///
        /// Should only be called with 4 empty strings.
        void wm_grid(this auto&& self, const std::string& baseWidth, const std::string& baseHeight, const std::string& widthInc, const std::string& heightInc)
        {
            self.tk->call("wm", "grid", self._w, baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @brief Return grid information for this widget.
        std::optional<std::array<long long, 4>> wm_grid(this auto&& self)
        {
            auto res = self.tk->template call<std::variant<std::array<long long, 4>, std::string>>("wm", "grid", self._w);
            if (std::holds_alternative<std::string>(res))
                return {};
            return std::get<std::array<long long, 4>>(res);
        }
        /// @copydoc wm_grid(this auto&&, long long, long long, long long, long long)
        void grid(this auto&& self, long long baseWidth, long long baseHeight, long long widthInc, long long heightInc)
        {
            self.wm_grid(baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @copydoc wm_grid(this auto&&, const std::string&, const std::string&, const std::string&, const std::string&)
        void grid(this auto&& self, const std::string& baseWidth, const std::string& baseHeight, const std::string& widthInc, const std::string& heightInc)
        {
            self.wm_grid(baseWidth, baseHeight, widthInc, heightInc);
        }
        /// @copydoc wm_grid(this auto&&)
        std::optional<std::array<long long, 4>> grid(this auto&& self)
        {
            return self.wm_grid();
        }

        /// @brief Set the group leader widgets for related widgets to PATHNAME.
        void wm_group(this auto&& self, const std::string& pathName)
        {
            self.tk->call("wm", "group", self._w, pathName);
        }
        /// @brief Get the current group leader.
        std::string wm_group(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "group", self._w);
        }
        /// @copydoc wm_group(this auto&&, const std::string&)
        void group(this auto&& self, const std::string& pathName)
        {
            self.wm_group(pathName);
        }
        /// @copydoc wm_group(this auto&&)
        std::string group(this auto&& self)
        {
            return self.wm_group();
        }

        /// @brief Set bitmap for the iconified widget to BITMAP.
        ///
        /// Under Windows, the DEFAULT parameter can be used to set the icon for the widget and any descendants that don't have an icon set explicitly.
        /// See Tk documentation for more information.
        void wm_iconbitmap(this auto&& self, const std::string& bitmap, bool default_)
        {
            if (default_)
                self.tk->call("wm", "iconmask", self._w, "-default", bitmap);
            else
                self.tk->call("wm", "iconmask", self._w, bitmap);
        }
        /// @brief Get name of the current icon bitmap associated with window
        std::string wm_iconbitmap(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "iconmask", self._w);
        }
        /// @copydoc wm_iconbitmap(this auto&&, const std::string&, bool)
        void iconbitmap(this auto&& self, const std::string& bitmap, bool default_)
        {
            self.wm_iconbitmap(bitmap, default_);
        }
        /// @copydoc wm_iconbitmap(this auto&&)
        std::string iconbitmap(this auto&& self)
        {
            return self.wm_iconbitmap();
        }

        /// @brief Display widget as icon.
        void wm_iconify(this auto&& self)
        {
            self.tk->call("wm", "iconify", self._w);
        }
        /// @copydoc wm_iconify
        void iconify(this auto&& self)
        {
            self.wm_iconify();
        }

        /// Set mask for the icon bitmap of this widget.
        void wm_iconmask(this auto&& self, const std::string& bitmap)
        {
            self.tk->call("wm", "iconmask", self._w, bitmap);
        }
        /// Get the current mask for the icon bitmap.
        std::string wm_iconmask(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "iconmask", self._w);
        }
        /// @copydoc wm_iconmask(this auto&&, const std::string&)
        void iconmask(this auto&& self, const std::string& bitmap)
        {
            return self.wm_iconmask(bitmap);
        }
        /// @copydoc wm_iconmask(this auto&&)
        std::string iconmask(this auto&& self)
        {
            return self.wm_iconmask();
        }

        /// @brief Set the name of the icon for this widget.
        void wm_iconname(this auto&& self, const std::string& newName)
        {
            self.tk->call("wm", "iconname", self._w, newName);
        }
        /// @brief Return the name of the icon for this widget.
        std::string wm_iconname(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "iconname", self._w);
        }
        /// @copydoc wm_iconname(this auto&&, const std::string&)
        void iconname(this auto&& self, const std::string& newName)
        {
            return self.wm_iconname(newName);
        }
        /// @copydoc wm_iconname(this auto&&)
        std::string iconname(this auto&& self)
        {
            return self.wm_iconname();
        }

        /// @brief
        void wm_iconphoto();
        void iconphoto();

        /// @brief Set the position of the icon of this widget to X and Y. 
        void wm_iconposition(this auto&& self, long long x, long long y)
        {
            self.tk->call("wm", "iconposition", self._w, x, y);
        }
        /// @brief Return the current position of the icon of this widget.
        std::array<long long, 2> wm_iconposition(this auto&& self)
        {
            return self.tk->template call<std::array<long long, 2>>("wm", "iconposition", self._w);
        }
        /// @copydoc wm_iconposition(this auto&&, long long, long long)
        void iconposition(this auto&& self, long long x, long long y)
        {
            return self.wm_iconposition(x, y);
        }
        /// @copydoc wm_iconposition(this auto&&)
        std::array<long long, 2> iconposition(this auto&& self)
        {
            return self.wm_iconposition();
        }

        /// @brief Set widget PATHNAME to be displayed instead of icon.
        void wm_iconwindow(this auto&& self, const std::string& pathName)
        {
            self.tk->call("wm", "wm_iconwindow", self._w, pathName);
        }
        /// @brief Return the current value of the icon window.
        Misc wm_iconwindow(this auto&& self);
        /// @copydoc wm_iconwindow(this auto&&, const std::string&)
        void iconwindow(this auto&& self, const std::string& pathName)
        {
            return self.wm_iconwindow(pathName);
        }
        /// @copydoc wm_iconwindow(this auto&&)
        Misc iconwindow(this auto&& self);

        /// @brief The widget specified will become a stand alone top-level window. 
        /// 
        /// The window will be decorated with the window managers title bar, etc.
        void wm_manage(this auto&& self, const std::derived_from<Misc> auto& widget)
        {
            self.tk->call("wm", "manage", widget);
        }
        /// @copydoc wm_manage
        void manage(this auto&& self, const std::derived_from<Misc> auto& widget)
        {
            return self.wm_manage(widget);
        }

        /// @brief Set max WIDTH and HEIGHT for this widget. If the window is gridded the values are given in grid units.
        void wm_maxsize(this auto&& self, long long width, long long height)
        {
            self.tk->call("wm", "maxsize", self._w, width, height);
        }
        /// @brief Return the current max WIDTH and HEIGHT for this widget.
        std::array<long long, 2> wm_maxsize(this auto&& self)
        {
            return self.tk->template call<std::array<long long, 2>>("wm", "maxsize", self._w);
        }
        /// @copydoc wm_maxsize(this auto&&, long long, long long)
        void maxsize(this auto&& self, long long width, long long height)
        {
            return self.wm_maxsize(width, height);
        }
        /// @copydoc wm_maxsize(this auto&&)
        std::array<long long, 2> maxsize(this auto&& self)
        {
            return self.wm_maxsize();
        }

        /// @brief Set min WIDTH and HEIGHT for this widget. If the window is gridded the values are given in grid units.
        void wm_minsize(this auto&& self, long long width, long long height)
        {
            self.tk->call("wm", "minsize", self._w, width, height);
        }
        /// @brief Return the current min WIDTH and HEIGHT for this widget.
        std::array<long long, 2> wm_minsize(this auto&& self)
        {
            return self.tk->template call<std::array<long long, 2>>("wm", "minsize", self._w);
        }
        /// @copydoc wm_minsize(this auto&&, long long, long long)
        void minsize(this auto&& self, long long width, long long height)
        {
            return self.wm_minsize(width, height);
        }
        /// @copydoc wm_minsize(this auto&&)
        std::array<long long, 2> minsize(this auto&& self)
        {
            return self.wm_minsize();
        }

        /// @brief Instruct the window manager to ignore this widget if BOOLEAN is true.
        void wm_overrideredirect(this auto&& self, bool boolean)
        {
            self.tk->call("wm", "overrideredirect", self._w, boolean);
        }
        /// @brief Return the current value of the overrideredirect flag.
        bool wm_overrideredirect(this auto&& self)
        {
            return self.tk->template call<bool>("wm", "overrideredirect", self._w);
        }
        /// @copydoc wm_overrideredirect(this auto&&, bool)
        void overrideredirect(this auto&& self, bool boolean)
        {
            return self.wm_overrideredirect(boolean);
        }
        /// @copydoc wm_overrideredirect(this auto&&)
        bool overrideredirect(this auto&& self)
        {
            return self.wm_overrideredirect();
        }

        /// @brief Instruct the window manager that the position of this widget shall be defined by the user if WHO is "user", and by its own policy if WHO is "program".
        void wm_positionfrom(this auto&& self, const std::string& who)
        {
            self.tk->call("wm", "positionfrom", self._w, who);
        }
        /// @brief Return the current positionfrom setting.
        std::string wm_positionfrom(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "positionfrom", self._w);
        }
        /// @copydoc wm_positionfrom(this auto&&, const std::string&)
        void positionfrom(this auto&& self, const std::string& who)
        {
            return self.wm_positionfrom(who);
        }
        /// @copydoc wm_positionfrom(this auto&&)
        std::string positionfrom(this auto&& self)
        {
            return self.wm_positionfrom();
        }

        /// Bind function FUNC to command NAME for this widget.
        /// 
        /// Return the function bound to NAME if None is given. NAME could be e.g. "WM_SAVE_YOURSELF" or "WM_DELETE_WINDOW".
        template<detail::FromObjConcept R = void, typename Func>
            requires detail::createcommand_concept<Func> || detail::AsObjConcept<std::remove_cvref_t<Func>>
        R wm_protocol(this auto && self, const std::string & name, Func && func)
        {
            if constexpr (detail::createcommand_concept<Func>)
                return self.tk->template call<R>("wm", "protocol", self._w, name, self._register(std::forward<Func>(func)));
            else
                return self.tk->template call<R>("wm", "protocol", self._w, name, std::forward<Func>(func));
        }
        /// @copydoc wm_protocol
        template<detail::FromObjConcept R = void, typename Func>
            requires detail::createcommand_concept<Func> || detail::AsObjConcept<std::remove_cvref_t<Func>>
        R protocol(this auto && self, const std::string & name, Func && func)
        {
            return self.template wm_protocol<R>(name, std::forward<Func>(func));
        }

        /// @brief Instruct the window manager whether this width can be resized in WIDTH or HEIGHT.
        void wm_resizable(this auto&& self, bool width, bool height)
        {
            self.tk->call("wm", "resizable", self._w, width, height);
        }
        /// @brief Return the current resizable settings.
        std::array<bool, 2> wm_resizable(this auto&& self)
        {
            return self.tk->template call<std::array<bool, 2>>("wm", "resizable", self._w);
        }
        /// @copydoc wm_resizable(this auto&&, bool, bool)
        void resizable(this auto&& self, bool width, bool height)
        {
            return self.wm_resizable(width, height);
        }
        /// @copydoc wm_resizable(this auto&&)
        std::array<bool, 2> resizable(this auto&& self)
        {
            return self.wm_resizable();
        }

        /// @brief Instruct the window manager that the size of this widget shall be defined by the user if WHO is "user", and by its own policy if WHO is "program".
        void wm_sizefrom(this auto&& self, const std::string& who)
        {
            self.tk->call("wm", "sizefrom", self._w, who);
        }
        /// @brief Return the current sizefrom setting.
        std::string wm_sizefrom(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "sizefrom", self._w);
        }
        /// @copydoc wm_sizefrom(this auto&&, const std::string&)
        void sizefrom(this auto&& self, const std::string& who)
        {
            return self.wm_sizefrom(who);
        }
        /// @copydoc wm_sizefrom(this auto&&)
        std::string sizefrom(this auto&& self)
        {
            return self.wm_sizefrom();
        }

        /// @brief Set the state of this widget as one of normal, icon, iconic (see wm_iconwindow), withdrawn, or zoomed (Windows only).
        void wm_state(this auto&& self, const std::string& newstate)
        {
            self.tk->call("wm", "state", self._w, newstate);
        }
        /// @brief Return the current state of this widget.
        std::string wm_state(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "state", self._w);
        }
        /// @copydoc wm_state(this auto&&, const std::string&)
        void state(this auto&& self, const std::string& newstate)
        {
            return self.wm_state(newstate);
        }
        /// @copydoc wm_state(this auto&&)
        std::string state(this auto&& self)
        {
            return self.wm_state();
        }

        /// @brief Set the title of this widget.
        void wm_title(this auto&& self, const std::string& string)
        {
            return self.tk->call("wm", "title", self._w, string);
        }
        /// @brief Get the title of this widget.
        std::string wm_title(this auto&& self)
        {
            return self.tk->template call<std::string>("wm", "title", self._w);
        }
        /// @copydoc wm_title(this auto&&, const std::string&)
        void title(this auto&& self, const std::string& string)
        {
            return self.wm_title(string);
        }
        /// @copydoc wm_title(this auto&&)
        std::string title(this auto&& self)
        {
            return self.wm_title();
        }

        /// @brief Instruct the window manager that this widget is transient with regard to widget MASTER.
        void wm_transient(this auto&& self, const std::string& master)
        {
            self.tk->call("wm", "transient", self._w, master);
        }
        /// @brief Return the current transient master.
        Misc wm_transient(this auto&& self);
        /// @copydoc wm_transient(this auto&&, const std::string&)
        void transient(this auto&& self, const std::string& master)
        {
            return self.wm_transient(master);
        }
        /// @copydoc wm_transient(this auto&&)
        Misc transient(this auto&& self);

        /// @brief Withdraw this widget from the screen such that it is unmapped and forgotten by the window manager.
        /// 
        /// Re - draw it with wm_deiconify.
        void wm_withdraw(this auto&& self)
        {
            self.tk->call("wm", "withdraw", self._w);
        }
        /// @copydoc wm_withdraw
        void withdraw(this auto&& self)
        {
            return self.wm_withdraw();
        }
    };

    /// @brief Contains structs to be passed to many of cpptkinter's functions.
    ///
    /// Replaces Python's **kwargs.
    namespace cnfs
    {
        template<typename T>
        struct is_cnf_member_trait : std::bool_constant<detail::AsObjConcept<T> || detail::createcommand_concept<T>> {};
        template<typename...Args>
            requires (is_cnf_member_trait<Args>::value && ...)
        struct is_cnf_member_trait<std::variant<Args...>> : std::true_type {};
        template<typename T>
            requires is_cnf_member_trait<T>::value
        struct is_cnf_member_trait<std::optional<T>> : std::true_type {};

        template<typename T>
        concept is_cnf_member = is_cnf_member_trait<T>::value;

        template<typename T, typename IS = std::make_index_sequence<reflect::size<T>()>>
        struct is_cnf_trait : std::false_type {};
        template<typename T, size_t...I>
            requires (!std::is_array_v<std::remove_cvref_t<T>>) && (is_cnf_member<std::remove_cvref_t<reflect::member_type<I, T>>> && ...)
        struct is_cnf_trait<T, std::integer_sequence<size_t, I...>> : std::true_type {};
        /// @brief Satsified if T is a cnf struct.
        /// 
        /// Usually cnf structs are passed to cpptkinter::Misc::_options() internally.
        template<typename T>
        concept is_cnf = is_cnf_trait<std::remove_cvref_t<T>>::value;

        using pad_type = utility::extend_variants<detail::ScreenUnits, std::array<detail::ScreenUnits, 2>>::type;
        using visual_type = std::variant<std::string, std::tuple<std::string, long long>>;

        template<typename T>
        using opt = std::optional<T>;
        using opt_string = opt<std::string>;
        using opt_bool = opt<bool>;
        using opt_screenunits = opt<detail::ScreenUnits>;
        using opt_pad_type = opt<pad_type>;
        using opt_visual_type = opt<visual_type>;
        using opt_anchor = opt<detail::Anchor>;
        using opt_font_description = opt<detail::FontDescription>;
        using opt_cursor = opt<detail::Cursor>;
        using opt_image_spec = opt<detail::ImageSpec>;
        using opt_compound = opt<detail::Compound>;
        using opt_relief = opt<detail::Relief>;
        using opt_take_focus_value = opt<detail::TakeFocusValue>;
        using opt_text = opt<std::variant<double, std::string>>;
        using opt_xy_scroll_command = opt<detail::XYScrollCommand>;

        /// @brief Argument for Misc::grid_columnconfigure() and Misc::grid_rowconfigure().
        struct grid_column_row_configure
        {
            opt_screenunits minsize;
            opt_screenunits pad;
            opt_string uniform;
            opt<size_t> weight;
        };
        /// @brief Return type of Misc::grid_columnconfigure() and Misc::grid_rowconfigure().
        struct grid_column_row_configure_return
        {
            long long minsize;
            long long pad;
            std::string uniform;
            long long weight;
        };

        struct grid_bbox
        {
            opt<long long> column, row, col2, row2;
        };
    }

    /// @brief Internal class.
    /// 
    /// Base class which defines methods common for interior widgets.
    class Misc
    {
        template<typename R, typename...Args>
        friend struct detail::CallWrapper;
        friend Wm;
        friend struct Pack;
        friend struct Grid;
        friend struct Place;
        friend class BaseWidget;
        friend Variable;
        friend detail::Tk_impl;
        template<typename T>
        friend class utility::weak;
        friend Tk detail::_get_default_root(const std::string&);

    protected:
        struct impl;

        std::shared_ptr<impl> pimpl;

        std::set<std::string>& _tclCommands;
        std::map<std::string, int>& _last_child_ids;
        std::string& _w;
    public:
        std::optional<Misc>& master;
        std::shared_ptr<_cpptkinter::TkappObject>& tk;
        std::map<std::string, Misc>& children;
    protected:
        template<std::derived_from<impl> I>
        Misc(const std::shared_ptr<I>& pimpl) :
            pimpl(pimpl),
            _tclCommands(pimpl->_tclCommands),
            _last_child_ids(pimpl->_last_child_ids),
            _w(pimpl->_w),
            master(pimpl->master),
            tk(pimpl->tk),
            children(pimpl->children)
        {

        }
    public:

        DEFINE_ASSIGNMENT_OPERATOR(Misc);

        /// @brief Calls this->pimpl->destroy().
        void destroy();

        /// @brief Internal function.
        /// 
        /// Delete the Tcl command provided in NAME.
        void deletecommand(const std::string& name)
        {
            this->tk->deletecommand(name);
            this->_tclCommands.erase(name);
        }

        /// @brief Set the list of bindtags for this widget.
        /// 
        /// The bindtags determine in which order events are processed(see bind).
        void bindtags(const utility::range_of_convertible_to<std::string> auto& tagList)
        {
			this->tk->call("bindtags", this->_w, tagList | std::views::transform([](auto& val) { return static_cast<std::string>(val); }));
        }
        /// @brief Get the list of bindtags for this widget.
        /// 
        /// The bindtags determine in which order events are processed(see bind).
        /// @returns The list of all bindtags associated with this widget.
        std::vector<std::string> bindtags()
        {
			return this->tk->template call<std::vector<std::string>>("bindtags", this->_w);
        }

        /// @brief Internal function.
        /// 
		/// Implements the first if statement.
        void _bind(std::vector<std::string>&& what, const std::string& sequence, const std::string& func)
        {
            this->tk->call(what | std::views::transform([](const std::string& str) { return _cpptkinter::AsObj(str); }), sequence, func);
        }
    private:
        template<std::invocable<Event<Misc>> Func>
        std::string _bind_if_2(Func&& func, bool needcleanup)
        {
            return this->_register(
                [func = std::forward<Func>(func), self = utility::weak(*this)](MISC_SUBSTITUTE_PARAMETERS) { return func(self.lock()._substitute(MISC_SUBSTITUTE_ARGUMENTS)); },
                needcleanup
            );
        }
    public:
        /// @brief Internal function.
        /// 
		/// Implements the second if statement with sequence != None.
		/// Creates a tcl command with func and binds it to sequence.
        /// @returns An identifier for the created tcl command.
        template<std::invocable<Event<Misc>> Func>
        std::string _bind(std::vector<std::string>&& what, const std::string& sequence, Func&& func, bool add = false, bool needcleanup = true)
        {
			auto funcid = this->_bind_if_2(std::forward<Func>(func), needcleanup);
            auto cmd = std::format("{}if {{\"[{} {}]\" == \"break\"}} break\n", add ? "+" : "", funcid, this->_subst_format_str);
            this->tk->call(what | std::views::transform([](const std::string& str) { return _cpptkinter::AsObj(str); }), sequence, cmd);
            return funcid;
        }
        /// @brief Internal function.
        /// 
		/// Implements the second if statement with sequence == None.
		/// Creates a tcl command with func but doesn't bind it (i.e. does nothing with it).
        /// @returns An identifier for the created tcl command.
        template<std::invocable<Event<Misc>> Func>
        std::string _bind(std::vector<std::string>&& what, Func&& func, bool add = false, bool needcleanup = true)
        {
            auto funcid = this->_bind_if_2(std::forward<Func>(func), needcleanup);
            return funcid;
        }
        /// @brief Internal function
        /// 
		/// Implements the third if statement.
		/// @returns The script currently bound to sequence or an empty string if there is no binding for sequence.
        std::string _bind(std::vector<std::string>&& what, const std::string& sequence)
        {
            return this->tk->call<std::string>(what | std::views::transform([](const std::string& str) { return _cpptkinter::AsObj(str); }), sequence);
        }
        /// @brief Internal function.
        /// 
		/// Implements the fourth if statement.
		/// @returns A list of all bound events associated with this widget.
        std::vector<std::string> _bind(std::vector<std::string>&& what)
        {
            return this->tk->call<std::vector<std::string>>(what | std::views::transform([](const std::string& str) { return _cpptkinter::AsObj(str); }));
        }

        /// @brief Bind to this widget at event SEQUENCE a call to function FUNC.
		///
        /// @param sequence is a string of concatenated event patterns.
        /// An event pattern is of the form <MODIFIER - MODIFIER - TYPE - DETAIL> where MODIFIER is one of Control, Mod2, M2, Shift, Mod3, M3, Lock, Mod4, M4, Button1, B1, Mod5, M5 Button2, B2, Meta, M, Button3, B3, Alt, Button4, B4, Double, Button5, B5 Triple, Mod1, M1.
        /// TYPE is one of Activate, Enter, Map, ButtonPress, %Button, Expose, Motion, ButtonRelease FocusIn, MouseWheel, Circulate, FocusOut, Property, Colormap, Gravity Reparent, Configure, KeyPress, Key, Unmap, Deactivate, KeyRelease Visibility, Destroy, Leave and DETAIL is the button number for ButtonPress, ButtonRelease and DETAIL is the Keysym for KeyPress and KeyRelease.
        /// Examples are <Control - %Button - 1> for pressing Control and mouse button 1 or <Alt - A> for pressing A and the Alt key (KeyPress can be omitted).
        /// An event pattern can also be a virtual event of the form <<AString>> where AString can be arbitrary.
        /// This event can be generated by event_generate. If events are concatenated they must appear shortly after each other.
        /// 
        /// @param func will be called if the event sequence occurs with an instance of Event as argument. If the return value of FUNC is "break" no further bound function is invoked.
        /// 
        /// @param add specifies whether FUNC will be called additionally to the other bound function or whether it will replace the previous function.
        /// 
        /// Bind will return an identifier to allow deletion of the bound function with unbind without memory leak.
        /// 
        /// If FUNC or SEQUENCE is omitted the bound function or list of bound events are returned.
        /// /// @see _bind.
        template<typename...Args>
        auto bind(Args&&...args) requires requires { this->_bind({}, std::forward<Args>(args)...); }
        {
            return this->_bind({ "bind", this->_w }, std::forward<Args>(args)...);
        }

        /// @brief Unbind for this widget the event SEQUENCE.
        /// 
        /// Destroy the current binding for SEQUENCE, leaving SEQUENCE unbound.
        void unbind(const std::string& sequence)
        {
			this->_unbind({ "bind", this->_w, sequence });
        }
        /// @brief Unbind for this widget the event SEQUENCE.
        /// 
        /// Unbind the function identified with FUNCID and also delete the corresponding Tcl command.
        void unbind(const std::string& sequence, const std::string& funcid)
        {
			this->_unbind({ "bind", this->_w, sequence }, funcid);
        }

        /// @brief Internal function.
        /// 
		/// Implements the first if statement.
        void _unbind(std::vector<std::string>&& what)
        {
			this->tk->call(what | std::views::transform([](const std::string& str) { return _cpptkinter::AsObj(str); }), "");
        }
        /// @brief Internal function.
        /// 
        /// Implements the second if statement.
        void _unbind(std::vector<std::string>&& what, const std::string& funcid)
        {
            auto prefix = std::format("if {{\"[{} ", funcid);
            std::string keep{};
            for(auto&& s : this->tk->call<std::string>(what | std::views::transform([](const std::string& str) { return _cpptkinter::AsObj(str); }))
                | std::views::split('\n')
                | std::views::filter([&](const auto& line) { return !std::ranges::starts_with(line, prefix); }))
            {
                keep.append_range(s);
                keep.push_back('\n');
            }
			if (keep.ends_with('\n'))
				keep.pop_back();

            auto temp_keep = keep;
            temp_keep.erase(0, temp_keep.find_first_not_of("\t\n\v\f\r ")); // left trim
            temp_keep.erase(temp_keep.find_last_not_of("\t\n\v\f\r ") + 1); // right trim
            if (temp_keep.empty())
                keep = "";

            this->tk->call(what | std::views::transform([](const std::string& str) { return _cpptkinter::AsObj(str); }), keep);
            this->deletecommand(funcid);
        }

        /// @brief Bind to all widgets at an event SEQUENCE a call to function FUNC.
        /// 
        /// An additional boolean parameter ADD specifies whether FUNC will be called additionally to the other bound function or whether it will replace the previous function.
        /// @see bind for the return value.
        template<typename...Args>
        auto bind_all(Args&&...args) requires requires { this->_bind({}, std::forward<Args>(args)..., true); }
        {
            return this->bind_class("all", std::forward<Args>(args)...);
        }

        /// @brief Unbind for all widgets for event SEQUENCE all functions.
        void unbind_all(const std::string& sequence)
        {
			this->unbind_class("all", sequence);
        }

        /// @brief Bind to widgets with bindtag CLASSNAME at event SEQUENCE a call of function FUNC.
        /// 
        /// An additional boolean parameter ADD specifies whether FUNC will be called additionally to the other bound function or whether it will replace the previous function.
        /// @see bind for the return value.
        template<typename...Args>
        auto bind_class(const std::string& className, Args&&...args) requires requires { this->_bind({}, std::forward<Args>(args)..., true); };

		/// @brief Unbind for all widgets with bindtag CLASSNAME for event SEQUENCE all functions.
        void unbind_class(const std::string& className, const std::string& sequence);

        /// @brief Call the mainloop of Tk.
        void mainloop(int n = 0)
        {
            this->tk->mainloop(n);
        }

        /// @brief Quit the Tcl interpreter. All widgets will be destroyed.
        void quit()
        {
            this->tk->quit();
        }
    protected:
        template<typename T>
            requires (cnfs::is_cnf_member<std::remove_cvref_t<T>> && !hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::optional>)
        _cpptkinter::Tcl_Obj _options_inner_visitor(T&& value)
        {
            auto visitor = [this]<typename T2>(T2 && value) {
                if constexpr (detail::createcommand_concept<T2>)
                    return _cpptkinter::AsObj(this->_register(std::forward<T2>(value)));
                else
                    return _cpptkinter::AsObj(std::forward<T2>(value));
            };

            return utility::visit_or_invoke(visitor, std::forward<T>(value));
        }

        /// @brief Converts a cnf struct to a vector of Tcl_Obj* which can be passed to TkappObject::call().
        template<cnfs::is_cnf CNF>
        std::vector<_cpptkinter::Tcl_Obj> _options(CNF&& cnf, const std::set<std::string>& ignore_fields = {})
        {
            std::vector<_cpptkinter::Tcl_Obj> raii{};

            auto visitor = [&]<typename T>(T&& value, auto I) {
                auto k = /*rfl::fields<CNF>()[I].name()*/reflect::member_name<I, CNF>();
                if (ignore_fields.contains(std::string(k)))
                    return;

                if (k.ends_with('_'))
                    /*k.erase(k.size() - 1)*/k.remove_suffix(1);

                raii.emplace_back(_cpptkinter::AsObj("-" + std::string(k)));
                raii.emplace_back(this->_options_inner_visitor(std::forward<T>(value)));
            };

            reflect::for_each<CNF>([&](auto I) {
                utility::invoke_or_and_then(visitor, reflect::get<I>(std::forward<CNF>(cnf)), I);
                });

            return raii;
        }
    public:
        /// @brief Return the Tkinter instance of a widget identified by its Tcl name NAME.
        Misc nametowidget(std::string_view name);
        /// @brief Return the Tkinter instance of a widget.
        Misc nametowidget(_cpptkinter::tk_window_type window)
        {
            return this->nametowidget(window.to_string());
        }

    protected:
        /// @brief Return a newly created Tcl function.
        ///
        /// If said Tcl function is called, the C++ function func will be executed.
        template<detail::createcommand_concept Func>
        std::string _register(Func&& func, bool needcleanup = true)
        {
            DEVIATING_IMPLEMENTATION_WARNING("original has subst");
            auto f = detail::CallWrapper{ std::function(std::forward<Func>(func)), *this };
            auto name = (std::ostringstream() << detail::tcl_command_name_counter++ << "_" << &func << "_").str();
            if constexpr (requires { std::ostringstream() << func; })
                name += (std::ostringstream() << func).str();
            else
                name += typeid(func).name();

            std::ranges::replace_if(name, [](char c) { return detail::tcl_forbidden_chars.contains(c); }, '_');
            this->tk->createcommand(name, std::move(f));
            if (needcleanup)
                this->_tclCommands.insert(name);
            return name;
        }

        Tk _root() const;

		static constexpr std::array _subst_format = { "%#"sv, "%b"sv, "%f"sv, "%h"sv, "%k"sv, "%s"sv, "%t"sv, "%w"sv, "%x"sv, "%y"sv, "%A"sv,
            "%E"sv, "%K"sv, "%N"sv, "%W"sv, "%T"sv, "%X"sv, "%Y"sv, "%D"sv };
        static const inline std::string _subst_format_str = hhh::misc::join_strings(_subst_format, " ");
        /// @brief Internal function.
        Event<Misc> _substitute(MISC_SUBSTITUTE_PARAMETERS)
        {
            // print args
            
            // [&](auto&...args) { (utility::visit_or_invoke([](auto& a) { hhh::misc::printl(a); }, args), ...); }(MISC_SUBSTITUTE_ARGUMENTS);

            static auto get_long_long = []<typename T>(const T& p, long long def = std::numeric_limits<long long>::min()) {
                if constexpr (std::same_as<T, std::string>)
                    return std::stoll(p);
				else if constexpr (std::same_as<T, long long>)
					return p;
                else    // substitute_long_long
                {
                    if (std::holds_alternative<std::string>(p))
                    {
                        if (std::get<std::string>(p) == "??")
                            return def;
                        throw detail::construct_exception<std::runtime_error>(std::format("expected \"??\" but got {}", std::get<std::string>(p)));
                    }
                    return std::get<long long>(p);
                }
                };
            static auto get_bool = [&]<typename T>(const T& p) {
                auto ll = get_long_long(p, 0);
				if (ll == 0)
					return false;
				else if (ll == 1)
					return true;
				else
					throw detail::construct_exception<std::runtime_error>(std::format("expected 0 or 1 but got {}", ll));
            };

            return {
                get_long_long(nsign),       // serial
                get_long_long(b),           // num
                get_bool(f),                // focus
                get_long_long(h),           // height
                get_long_long(w),           // width
                get_long_long(k),           // keycode
                get_long_long(s),           // state
                get_long_long(t),           // time
                get_long_long(x),           // x
                get_long_long(y),           // y
                get_long_long(X),           // x_root
                get_long_long(Y),           // y_root
                A,                          // char_
                get_bool(E),                // send_event
                K,                          // keysym
                get_long_long(N),           // keysym_num
                EventType(get_long_long(T)),// type
                this->nametowidget(W),      // widget
                get_long_long(D, 0)         // delta
            };
        }

        void _report_exception();

        std::map<std::string, std::array<std::variant<long long, std::string>, 5>> _getconfigure(std::vector<_cpptkinter::Tcl_Obj>&& raii)
        {
            using V = std::variant<long long, std::string, _cpptkinter::Tcl_Obj>;
            using Arr = std::array<V, 5>;

            auto vec = this->tk->call<std::vector<Arr>>(std::move(raii));

            auto key_view = vec | std::views::transform([&](Arr& e) { return std::get<std::string>(std::move(e.at(0))).substr(1); });

            auto v_lambda = []<typename T>(T & e)->std::variant<long long, std::string> {
                if constexpr (std::same_as<T, long long> || std::same_as<T, std::string>)
                    return std::move(e);
                else
                    return e._repr_();
            };
            auto value_view = vec | std::views::transform([&](Arr& arr) {
                std::array<std::variant<long long, std::string>, 5> new_arr{};
                std::ranges::move(arr | std::views::transform([&](V& e) { return std::visit(v_lambda, e); }), new_arr.begin());
                return new_arr; });

            return /*std::views::*/zip(key_view, value_view) | /*std::ranges::*/to<std::map>();
        }
        std::array<std::variant<long long, std::string>, 5> _getconfigure1(std::vector<_cpptkinter::Tcl_Obj>&& raii)
        {
            using V = std::variant<long long, std::string, _cpptkinter::Tcl_Obj>;
            using Arr = std::array<V, 5>;

            auto arr = this->tk->call<Arr>(std::move(raii));
            std::array<std::variant<long long, std::string>, 5> new_arr{};

            auto v_lambda = []<typename T>(T & e)->std::variant<long long, std::string> {
                if constexpr (std::same_as<T, long long> || std::same_as<T, std::string>)
                    return std::move(e);
                else
                    return e._repr_();
            };
            std::ranges::move(arr | std::views::transform([&](V& e) { return std::visit(v_lambda, e); }), new_arr.begin());

            return new_arr;
        }

        auto _configure(const std::vector<std::string>& cmd) -> decltype(_getconfigure({}))
        {
            std::vector<_cpptkinter::Tcl_Obj> raii{ _cpptkinter::AsObj(this->_w) };
            for (auto& c : cmd)
                raii.emplace_back(_cpptkinter::AsObj(c));

            return this->_getconfigure(std::move(raii));
        }
        auto _configure(const std::vector<std::string>& cmd, const std::string& cnf) -> decltype(_getconfigure1({}))
        {
            std::vector<_cpptkinter::Tcl_Obj> raii{ _cpptkinter::AsObj(this->_w) };
            for (auto& c : cmd)
                raii.emplace_back(_cpptkinter::AsObj(c));
            raii.emplace_back(_cpptkinter::AsObj("-" + cnf));

            return this->_getconfigure1(std::move(raii));
        }
        template<cnfs::is_cnf CNF>
        void _configure(const std::vector<std::string>& cmd, CNF&& cnf)
        {
            auto raii = cmd | std::views::transform(_cpptkinter::AsObj<std::string>) | std::ranges::to<std::vector>();
            this->tk->call(this->_w, std::move(raii), this->_options(std::forward<CNF>(cnf)));
        }
    public:
        /// @brief Configure resources of a widget.
        /// 
        /// To get an overview about the allowed keyword arguments call the method keys.
        /// @param cnf A string or cnf struct.
        template<typename T>
        auto configure(T&& v) requires requires { this->_configure({ }, std::declval<T>()); }
        {
            return this->_configure({ "configure" }, std::forward<T>(v));
        }
        /// @brief Configure a resource of a widget.
        ///
        /// @param keyword The keyword of the resource.
        /// @param value The new value of the resource.
        template<typename T>
        void configure(const std::string& key, T&& value) requires requires { this->_options_inner_visitor(std::declval<T>()); }
        {
            this->tk->call(this->_w, "configure", "-" + key, this->_options_inner_visitor(std::forward<T>(value)));
        }

        /// @copydoc configure(T&&)
        template<typename T>
        auto config(T&& v) requires requires { this->configure(std::declval<T>()); }
        {
            return this->configure(std::forward<T>(v));
        }
        /// @copydoc configure(const std::string&, T&&)
        template<typename T>
        void config(const std::string& key, T&& value) requires requires { this->configure(key, std::declval<T>()); }
        {
            this->configure(key, std::forward<T>(value));
        }

        template<detail::FromObjConcept R>
        R cget(const std::string& key)
        {
            return this->tk->call<R>(this->_w, "cget", "-" + key);
        }

		/// @brief Get a resource of a widget.
        /// 
        /// Used by detail::set_get_proxy. Can be overloaded by inheriting widgets.
        template<detail::FromObjConcept R>
        R _getitem_(const std::string& key, std::type_identity<R>)
        {
			return this->cget<R>(key);
        }

        /// @brief Set a resource of a widget.
        /// 
        /// Used by detail::set_get_proxy. Can be overloaded by inheriting widgets.
        template<typename T>
        void _setitem_(const std::string& key, T&& value) requires requires { this->configure(std::string{}, std::declval<T>()); }
        {
            this->configure(key, std::forward<T>(value));
        }

		/// @brief Returns a proxy object which can be used to set/get resources of a widget.
        template<typename Self>
        detail::set_get_proxy<std::remove_cvref_t<Self>> operator[](this Self&& self, const std::string& key)
        {
			return { std::forward<Self>(self), key };
        }

        /// @brief Return a list of all resource names of this widget.
        std::vector<std::string> keys()
        {
            auto vec = this->tk->call<std::vector<std::vector<std::variant<std::string, detail::ignore>>>>(this->_w, "configure");
            std::vector<std::string> res{};
            for (auto& outer : vec)
                res.emplace_back(std::get<std::string>(std::move(outer.at(0))).substr(1));
            return res;
        }

        /// @brief Return the window path name of this widget.
        operator std::string() const
        {
            return this->_w;
        }
        /// @brief Outputs the window path name of this widget to an output stream.
        friend std::ostream& operator<<(std::ostream& os, const Misc& self)
        {
            return os << self._w;
        }

        /// @brief Get the status for propagation of geometry information.
        ///
        /// @returns The current setting.
        bool pack_propagate()
        {
            return this->tk->call<long long>("pack", "propagate", this->_w);
        }
        /// @brief Set the status for propagation of geometry information.
        /// 
        /// @param flag Specifies whether the geometry information of the slaves will determine the size of this widget.
        void pack_propagate(bool flag)
        {
            this->tk->call("pack", "propagate", this->_w, flag);
        }

        /// @copydoc pack_propagate()
        bool propagate()
        {
            return this->pack_propagate();
        }
        /// @copydoc pack_propagate(bool)
        void propagate(bool flag)
        {
            this->pack_propagate(flag);
        }

        /// @brief Return a list of all slaves of this widget in its packing order.
        std::vector<Misc> pack_slaves()
        {
            auto temp = this->tk->call<std::vector<_cpptkinter::tk_window_type>>("pack", "slaves", this->_w);
            std::vector<cpptkinter::Misc> result{};
            for (auto& t : temp)
                result.emplace_back(this->nametowidget(std::move(t)));
            return result;
        }
        /// @copydoc pack_slaves
        std::vector<Misc> slaves()
        {
            return this->pack_slaves();
        }

        /// @brief Return a list of all slaves of this widget in its packing order.
        std::vector<Misc> place_slaves()
        {
            auto temp = this->tk->call<std::vector<_cpptkinter::tk_window_type>>("place", "slaves", this->_w);
            std::vector<Misc> result{};
            for (auto& t : temp)
                result.emplace_back(this->nametowidget(std::move(t)));
            return result;
        }

        /// @brief The anchor value controls how to place the grid within the master when no row / column has any weight.
        ///
        /// The default anchor is nw.
        void grid_anchor(const std::string& anchor = {})
        {
            if (anchor.empty())
                this->tk->call("grid", "anchor", this->_w);
            else
                this->tk->call("grid", "anchor", this->_w, anchor);
        }
        /// @copydoc grid_anchor
        void anchor(const std::string& anchor = {})
        {
            this->grid_anchor(anchor);
        }

        /// @brief Return a tuple of integer coordinates for the bounding box of this widget controlled by the geometry manager grid.
        ///
        /// If COLUMN, ROW is given the bounding box applies from the cell with row and column 0 to the specified cell.
        /// If COL2 and ROW2 are given the bounding box starts at that cell.
        /// The returned integers specify the offset of the upper left corner in the master widget and the width and height.
        std::array<long long, 4> grid_bbox(const cnfs::grid_bbox& cnf = {})
        {
            std::vector<_cpptkinter::Tcl_Obj> args{ };
            args.emplace_back(_cpptkinter::AsObj("grid"));
            args.emplace_back(_cpptkinter::AsObj("bbox"));
            args.emplace_back(_cpptkinter::AsObj(this->_w));
            if (cnf.column.has_value() && cnf.row.has_value())
            {
                args.emplace_back(_cpptkinter::AsObj(cnf.column.value()));
                args.emplace_back(_cpptkinter::AsObj(cnf.row.value()));
            }
            if (cnf.col2.has_value() && cnf.row2.has_value())
            {
                args.emplace_back(_cpptkinter::AsObj(cnf.col2.value()));
                args.emplace_back(_cpptkinter::AsObj(cnf.row2.value()));
            }
            return this->tk->call<std::array<long long, 4>>(std::move(args));
        }
        /// @copydoc grid_bbox
        std::array<long long, 4> bbox(const cnfs::grid_bbox& cnf = {})
        {
            return this->grid_bbox(cnf);
        }

    protected:
        cnfs::grid_column_row_configure_return _grid_configure(const std::string& command, const std::variant<long long, std::string>& index)
        {
            auto temp = this->tk->call<std::tuple<
                std::string, long long,
                std::string, long long,
                std::string, std::string,
                std::string, long long>>("grid", command, this->_w, index);
            if (std::get<0>(temp) != "-minsize"
                || std::get<2>(temp) != "-pad"
                || std::get<4>(temp) != "-uniform"
                || std::get<6>(temp) != "-weight")
                throw detail::construct_exception<TclError>("unexpected return value " + utility::range_or_tuple_to_string(temp));
            return { std::get<1>(temp), std::get<3>(temp), std::get<5>(temp), std::get<7>(temp) };
        }
    public:
        /// @brief Configure column INDEX of a grid.
        cnfs::grid_column_row_configure_return grid_columnconfigure(const std::variant<long long, std::string>& index)
        {
            return this->_grid_configure("columnconfigure", index);
        }
        /// @brief Configure column INDEX of a grid.
        /// 
        /// Valid resources are minsize (minimum size of the column), weight (how much does additional space propagate to this column),
        /// pad (how much space to let additionally) and uniform.
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void grid_columnconfigure(std::variant<size_t, std::vector<size_t>, std::string> index, CNF&& cnf)
        {
            this->tk->call("grid", "columnconfigure", this->_w, index, this->_options(std::forward<CNF>(cnf)));
        }

        /// @copydoc grid_columnconfigure(const std::variant<long long, std::string>&)
        cnfs::grid_column_row_configure_return columnconfigure(const std::variant<long long, std::string>& index)
        {
            return this->grid_columnconfigure(index);
        }
        /// @copydoc grid_columnconfigure()
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void columnconfigure(std::variant<size_t, std::vector<size_t>, std::string> index, CNF&& cnf)
        {
            return this->grid_columnconfigure(index, std::forward<CNF>(cnf));
        }

        /// @brief Return a tuple of column and row which identify the cell at which the pixel at position X and Y inside the master widget is located.
        std::array<long long, 2> grid_location(const detail::ScreenUnits& x, const detail::ScreenUnits& y)
        {
            return this->tk->call<std::array<long long, 2>>("grid", "location", this->_w, x, y);
        }
        /// @copydoc grid_location
        std::array<long long, 2> location(const detail::ScreenUnits& x, const detail::ScreenUnits& y)
        {
            return this->grid_location(x, y);
        }

        /// @brief Get the status for propagation of geometry information.
        ///
        /// A boolean argument specifies whether the geometry information of the slaves will determine the size of this widget.
        bool grid_propagate()
        {
            return this->tk->call<long long>("grid", "propagate", this->_w);
        }
        /// @brief Set the status for propagation of geometry information.
        ///
        /// A boolean argument specifies whether the geometry information of the slaves will determine the size of this widget.
        void grid_propagate(bool flag)
        {
            this->tk->call<std::string>("grid", "propagate", this->_w, flag);
        }

        /// @brief Configure row INDEX of a grid.
        cnfs::grid_column_row_configure_return grid_rowconfigure(const std::variant<long long, std::string>& index)
        {
            return this->_grid_configure("rowconfigure", index);
        }
        /// @brief Configure row INDEX of a grid.
        ///
        /// Valid resources are minsize (minimum size of the row), weight (how much does additional space propagate to this row),
        /// pad (how much space to let additionally) and uniform.
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void grid_rowconfigure(const std::variant<size_t, std::vector<size_t>, std::string>& index, CNF&& cnf)
        {
            this->tk->call("grid", "rowconfigure", this->_w, index, this->_options(std::forward<CNF>(cnf)));
        }

        /// @copydoc grid_rowconfigure(const std::variant<long long, std::string>&)
        cnfs::grid_column_row_configure_return rowconfigure(const std::variant<long long, std::string>& index)
        {
            return this->grid_rowconfigure(index);
        }
        /// @copydoc grid_rowconfigure()
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void rowconfigure(const std::variant<size_t, std::vector<size_t>, std::string>& index, CNF&& cnf)
        {
            return this->grid_rowconfigure(index, std::forward<CNF>(cnf));
        }

        /// @brief Return a tuple of the number of column and rows in the grid.
        std::array<long long, 2> grid_size()
        {
            return this->tk->call<std::array<long long, 2>>("grid", "size", this->_w);
        }
        /// @copydoc grid_size
        std::array<long long, 2> size()
        {
            return this->grid_size();
        }

        /// @brief Return a list of all slaves of this widget in its packing order.
        std::vector<Misc> grid_slaves(std::optional<long long> row = {}, std::optional<long long> column = {})
        {
            std::vector<_cpptkinter::Tcl_Obj> args{ };
            if (row.has_value())
            {
                args.emplace_back(_cpptkinter::AsObj("-row"));
                args.emplace_back(_cpptkinter::AsObj(row.value()));
            }
            if (column.has_value())
            {
                args.emplace_back(_cpptkinter::AsObj("-column"));
                args.emplace_back(_cpptkinter::AsObj(column.value()));
            }

            auto temp = this->tk->call<std::vector<_cpptkinter::tk_window_type>>("grid", "slaves", this->_w, std::move(args));
            std::vector<Misc> result{};
            for (auto& t : temp)
                result.emplace_back(this->nametowidget(std::move(t)));
            return result;
        }
    };

    struct Misc::impl : hhh::misc::extended_enable_shared_from_this
    {
        std::set<std::string> _tclCommands{};
        std::map<std::string, int> _last_child_ids{};
        std::string _w;
        std::optional<Misc> master;
        std::shared_ptr<_cpptkinter::TkappObject> tk;
        std::map<std::string, Misc> children;

        /// @brief Internal function.
        /// 
        /// Deletes all Tcl commands created for this widget in the Tcl interpreter.
        virtual void destroy()
        {
            // keeps this from being destroyed before this function returns
            auto temp = this->shared_from_this();

            for (auto&& name : this->_tclCommands)
                this->tk->deletecommand(name);
        }
    };

    /// @brief Mix-in class for querying and changing the horizontal position of a widget's window.
    struct XView
    {
        /// @brief Query the horizontal position of the view.
        std::array<double, 2> xview(this auto&& self)
        {
            return self.tk->template call<std::array<double, 2>>(self._w, "xview");
        }
        /// @brief Change the horizontal position of the view.
        void xview(this auto&& self, double d1, double d2)
        {
            self.tk->call(self._w, "xview", d1, d2);
        }

        /// @brief Adjusts the view in the window so that FRACTION of the total width of the canvas is off - screen to the left.
        void xview_moveto(this auto&& self, double fraction)
        {
            self.tk->call(self._w, "xview", "moveto", fraction);
        }

        /// @brief Shift the x-view according to NUMBER which is measured in "units" or "pages" (WHAT).
        void xview_scroll(this auto&& self, const detail::ScreenUnits& number, const std::string& what)
        {
            self.tk->call(self._w, "xview", "scroll", number, what);
        }
    };

    /// @brief Mix-in class for querying and changing the vertical position of a widget's window.
    struct YView
    {
        /// @brief Query the vertical position of the view.
        std::array<double, 2> yview(this auto&& self)
        {
            return self.tk->template call<std::array<double, 2>>(self._w, "yview");
        }
        /// @brief Change the vertical position of the view.
        void yview(this auto&& self, double d1, double d2)
        {
            self.tk->call(self._w, "yview", d1, d2);
        }

        /// @brief Adjusts the view in the window so that FRACTION of the total height of the canvas is off - screen to the top.
        void yview_moveto(this auto&& self, double fraction)
        {
            self.tk->call(self._w, "yview", "moveto", fraction);
        }

        /// @brief Shift the y-view according to NUMBER which is measured in "units" or "pages" (WHAT).
        void yview_scroll(this auto&& self, const detail::ScreenUnits& number, const std::string& what)
        {
            self.tk->call(self._w, "yview", "scroll", number, what);
        }
    };

    template<typename R, typename...Args>
    struct detail::CallWrapper
    {
        std::function<R(Args...)> func;
        Misc/*utility::weak<Misc>*/ widget;

        /// Apply FUNC to arguments.
        R operator()(Args...args)
        {
            try
            {
                return this->func(std::forward<Args>(args)...);
            }
            catch (...)
            {
                this->widget/*.lock()*/._report_exception();
                throw;
            }
        }
    };

    struct detail::Tk_impl : Misc::impl
    {
        bool _tkloaded = false;

        /// @brief Destroy this and all descendants widgets.
        /// 
        /// This will end the application of this Tcl interpreter.
        void destroy() override
        {
            // keeps this from being destroyed before this function returns
            auto temp = this->shared_from_this();

            for (auto&& child : std::vector(std::from_range, std::views::values(this->children)))
                child.destroy();
            this->tk->call("destroy", this->_w);
            this->Misc::impl::destroy();
            if (detail::_support_default_root && detail::_default_root.get() == this)
                detail::_default_root.reset();
        }
    };

    /// @brief %Toplevel widget of %Tk which represents mostly the main window of an application. 
    ///
    /// It has an associated Tcl interpreter.
    class Tk : public Misc, public Wm
    {
        friend Wm;
        friend Misc;
        template<typename T>
        friend class utility::weak;
        friend Tk detail::_get_default_root(const std::string&);

    protected:
        using impl = detail::Tk_impl;
        REF_TO_IMPL(_tkloaded);

        void _init_(const std::string& screenName, const std::string& baseName_, const std::string& className, bool useTk, bool sync, const std::string& use)
        {
            this->_w = ".";

            auto baseName = baseName_.empty() ? std::filesystem::path(_cpptkinter::detail::argv[0]).string() : baseName_;
            auto interactive = false;
            this->tk = _cpptkinter::create(screenName, baseName, className, interactive, useTk, sync, use);
            if (detail::_debug)
                this->tk->settrace(detail::_print_command);
            if (useTk)
                this->_loadtk();
            this->readprofile(baseName, className);
        }

        template<std::derived_from<impl> I>
        Tk(const std::shared_ptr<I>& pimpl) :
            Misc(pimpl),
            _tkloaded(pimpl->_tkloaded)
        {

        }
    public:
        DEFINE_ASSIGNMENT_OPERATOR(Tk);

        /// @brief Create a new Tk object.
        ///
        /// A new Tcl interpreter will be created.
        /// @param baseName will be used for the identification of the profile file (see detail::Tk::readprofile()).
        /// It is constructed from @ref detail::argv[0] without extensions if none is given.
        /// @param className is the name of the widget class.
        /// @return A shared pointer to the newly created detail::Tk object.
        Tk(const std::string& screenName = {}, const std::string& baseName = {}, const std::string& className = "Tk", bool useTk = true, bool sync = false, const std::string& use = {}) :
            Tk(std::make_shared<impl>())
        {
            this->_init_(screenName, baseName, className, useTk, sync, use);
        }

        void loadtk()
        {
            if (!this->_tkloaded)
            {
                this->tk->loadtk();
                this->loadtk();
            }
        }
    private:
        void _loadtk()
        {
            this->_tkloaded = true;

            // Version sanity checks
            auto tk_version = this->tk->getvar<std::string>("tk_version");
            if (tk_version != _cpptkinter::TK_VERSION)
                throw detail::construct_exception<std::runtime_error>(std::format("tk.h version {} doesn't match libtk.a version {}", _cpptkinter::TK_VERSION, tk_version));

            // Under unknown circumstances, tcl_version gets coerced to float
            auto tcl_version = this->tk->getvar<std::string>("tcl_version");
            if (tcl_version != _cpptkinter::TCL_VERSION)
                throw detail::construct_exception<std::runtime_error>(std::format("tcl.h version {} doesn't match libtcl.a version {}", _cpptkinter::TCL_VERSION, tcl_version));

            // Create and register the tkerror and exit commands. We need to parts of _register here, _ register would register differently-named commands.
            this->tk->createcommand("tkerror", detail::_tkerror);
            this->tk->createcommand("exit", detail::_exit);
            this->_tclCommands.insert("tkerror");
            this->_tclCommands.insert("exit");
            if (detail::_support_default_root && detail::_default_root.get() == nullptr)
                detail::_default_root = std::static_pointer_cast<impl>(this->pimpl);
            this->protocol("WM_DELETE_WINDOW", std::function<void()>(std::bind_front(&Tk::destroy, *this)));
        }
    public:
        /// @brief This function prints the exception to stderr.
        ///
        /// It is the default callback registered in @ref report_callback_exception.
        static void default_report_callback_exception(Tk&, const std::exception_ptr& exc_ptr)
        {
            DEVIATING_IMPLEMENTATION_WARNING("original catches exceptions and prints them to stderr");
            /*try
            {*/
            std::rethrow_exception(exc_ptr);
            /*}
            catch (const std::exception& ex)
            {
                std::cerr << "Exception in Tk callback: " << ex.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown exception in Tk callback" << std::endl;
            }*/
        }

        /// @brief Internal function.
        /// 
        /// It reads .BASENAME.tcl and .CLASSNAME.tcl into the Tcl Interpreter.
        void readprofile(const std::string& baseName, const std::string& className)
        {
            auto _home = std::getenv("HOME");
            std::string home = _home != nullptr ? _home : ".";
            auto class_tcl = std::filesystem::path(home) / std::format(".{}.tcl", className);
            auto class_py = std::filesystem::path(home) / std::format(".{}.py", className);
            auto base_tcl = std::filesystem::path(home) / std::format(".{}.tcl", className);
            auto base_py = std::filesystem::path(home) / std::format(".{}.py", className);

            if (std::filesystem::is_regular_file(class_tcl))
                this->tk->call("source", "\"" + class_tcl.string() + "\"");
            //if os.path.isfile(class_py):
            //  exec(open(class_py).read(), dir)
            if (std::filesystem::is_regular_file(base_tcl))
                this->tk->call("source", "\"" + base_tcl.string() + "\"");
            //if os.path.isfile(base_py) :
            //  exec(open(base_py).read(), dir)

            DEVIATING_IMPLEMENTATION_WARNING("original executes class_py and base_py with python's exec()");
        }

        /// @brief Report callback exception on stderr.
        ///
        /// Applications may want to override this internal function. Default value is @ref _report_callback_exception-
        std::function<void(Tk&, const std::exception_ptr&)> report_callback_exception = &Tk::default_report_callback_exception;

        /// @brief Not implementable until c++26 when (hopefully) reflection will allow for code gen.
        void __getattr__() = delete;
    };

    Tk Tcl(const std::string& screenName = {}, const std::string& baseName = {}, const std::string& className = "Tk", bool useTk = true)
    {
        return Tk(screenName, baseName, className, useTk);
    }

    void mainloop(int n = 0)
    {
        detail::_get_default_root("call mainloop").tk->mainloop(n);
    }

    namespace detail
    {
        template<typename T>
        concept Variable_mode_concept = std::convertible_to<T, std::string> || (utility::is_vector<T> && std::convertible_to<typename T::value_type, std::string>);

        template<typename T>
        T pack_grid_info(auto&& self, const std::string& a1, const std::string& a2, const std::string& a3)
        {
            using V = std::variant<long long, std::string, _cpptkinter::tk_window_type>;
            auto map = self.tk->template call<std::map<std::string, V>>(a1, a2, a3);

            if (map.size() != reflect::size<T>())
                throw detail::construct_exception<std::invalid_argument>(std::format("map has {} elements but type {} has {} members", map.size(), reflect::type_name<T>(), reflect::size<T>()));

            auto converter = [&self]<typename T2>(V&& v)->T2
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
    }

    namespace cnfs
    {
        using opt_master = opt<Misc>;

        /// @brief Argument for Variable::Variable() and detail::TypedVariable::TypedVariable().
        template<typename T>
        struct Variable
        {
            opt_master master;
            opt<T> value;
            std::string name;
        };
    }

    /// @brief Class to define value holders for e.g. buttons.
    ///
    /// Subclasses StringVar, IntVar, DoubleVar, BooleanVar are specializations that constrain the type of the value returned from get().
    class Variable
    {
    protected:
        struct impl : hhh::misc::extended_enable_shared_from_this
        {
            std::set<std::string> _tclCommands;
            Tk _root;
            std::shared_ptr<_cpptkinter::TkappObject> _tk;
            std::string _name;

            impl(const Tk& root) : _root(root)
            {

            }
            /// @brief Unset the variable in Tcl.
            virtual ~impl()
            {
                if (this->_tk == nullptr)
                    return;
                DEVIATING_IMPLEMENTATION_WARNING("original uses self.tk.getboolean to convert to bool");
                if (this->_tk->call<long long>("info", "exists", this->_name))
                    this->_tk->globalunsetvar(this->_name);
                for (auto&& name : this->_tclCommands)
                    this->_tk->deletecommand(name);
            }
        };

        std::shared_ptr<impl> pimpl;

        REF_TO_IMPL(_tclCommands);
        REF_TO_IMPL(_root);
        REF_TO_IMPL(_tk);
        REF_TO_IMPL(_name);

    protected:
        template<typename T>
            requires detail::AsObjConcept<T> && std::default_initializable<T>
        void _init_(const cnfs::Variable<T>& cnf)
        {
            if (cnf.master.has_value())
                this->_tk = cnf.master->tk;
            else
                this->_tk = this->_root.tk;

            if (!cnf.name.empty())
                this->_name = cnf.name;
            else
                this->_name = std::format("PY_VAR{}", detail::_varnum++);

            if (cnf.value.has_value())
                this->initialize(cnf.value.value());
            else if (this->_tk->call<long long>("info", "exists", this->_name))
                this->initialize(T{});
            DEVIATING_IMPLEMENTATION_WARNING("original uses self.tk.getboolean to convert to bool");
        }

        template<std::derived_from<impl> I>
        Variable(const std::shared_ptr<I>& pimpl) :
            pimpl(pimpl),
            _tclCommands(pimpl->_tclCommands),
            _root(pimpl->_root),
            _tk(pimpl->_tk),
            _name(pimpl->_name)
        {

        }
    public:
        /// @brief Construct a variable
        ///
        /// If NAME matches an existing variable and VALUE is omitted then the existing value is retained.
        template<typename T>
            requires detail::AsObjConcept<T>&& std::default_initializable<T>
        Variable(const cnfs::Variable<T>& cnf = {}) : Variable(std::make_shared<impl>(cnf.master.has_value() ? cnf.master->_root() : detail::_get_default_root("create variable")))
        {
            this->_init_(cnf);
        }

        DEFINE_ASSIGNMENT_OPERATOR(Variable);

        /// @brief Return the name of the variable in Tcl.
        operator std::string() const
        {
            return this->_name;
        }

        /// @brief Set the variable to VALUE.
        void set(const detail::AsObjConcept auto& value)
        {
            this->_tk->globalsetvar(this->_name, value);
        }
        /// @copydoc set
        void initialize(const detail::AsObjConcept auto& value)
        {
            this->set(value);
        }

        /// @brief Return value of variable.
        template<detail::FromObjConcept R>
        R get()
        {
            return this->_tk->globalgetvar<R>(this->_name);
        }

        template<detail::createcommand_concept Func>
        std::string _register(Func&& callback)
        {
            auto f = detail::CallWrapper{ std::function(std::forward<Func>(callback)), this->_root };
            auto cbname = (std::ostringstream() << detail::tcl_command_name_counter++ << "_" << &callback << "_").str();
            if constexpr (requires { std::ostringstream() << callback; })
                cbname += (std::ostringstream() << callback).str();
            else
                cbname += typeid(callback).name();

            std::ranges::replace_if(cbname, [](char c) { return detail::tcl_forbidden_chars.contains(c); }, '_');
            this->_tk->createcommand(cbname, std::move(f));
            this->_tclCommands.insert(cbname);
            return cbname;
        }

        /// @brief Define a trace callback for the variable.
        ///
        /// @param mode One of "read", "write", "unset", or a vector of such strings.
        /// @param callback Must be a function which is called when the variable is read, written or unset. Gets passed 3 std::strings.
        /// @return The name of the callback.
        template<detail::Variable_mode_concept T, detail::createcommand_concept Func>
        std::string trace_add(const T& mode, Func&& callback)
        {
            auto cbname = this->_register(std::forward<Func>(callback));
            this->_tk->call("trace", "add", "variable", this->_name, mode, std::vector{ cbname });
            return cbname;
        }

        /// @brief Delete the trace callback for a variable.
        /// 
        /// @param mode is one of "read", "write", "unset" or a list of such strings. Must be same as were specified in trace_add().
        /// @param cbname is the name of the callback returned from trace_add().
        template<detail::Variable_mode_concept T>
        void trace_remove(const T& mode, const std::string& cbname)
        {
            this->_tk->call("trace", "remove", "variable", this->_name, mode, cbname);

            for (auto&& [m, ca] : this->trace_info())
                if (ca == cbname)
                    return;

            this->_tk->deletecommand(cbname);
            this->_tclCommands.erase(cbname);
        }

        /// @brief Return all trace callback information.
        std::vector<std::tuple<std::vector<std::string>, std::string>> trace_info()
        {
            return this->_tk->call<std::vector<std::tuple<std::vector<std::string>, std::string>>>("trace", "info", "variable", this->_name);
        }

        template<typename Self, std::derived_from<Variable> Other>
        bool operator==(this const Self& self, const Other& other)
        {
            return self->_name == other._name && self->_tk == other._tk;
        }
    };
    
    template<typename T>
        requires detail::AsObjConcept<T> && detail::FromObjConcept<T> && std::default_initializable<T>
    struct TypedVariable : Variable
    {
        using value_type = T;

        /// @copydoc Variable::Variable(const cnfs::Variable<T>&, std::type_identity<T>)
        TypedVariable(const cnfs::Variable<T>& cnf = {}) : Variable(cnf)
        {

        }

        /// @copydoc Variable::set
        void set(const T& value)
        {
            this->Variable::set(value);
        }
        /// @copydoc set
        void initialize(const T& value)
        {
            this->set(value);
        }

        /// @copydoc Variable::get()
        T get()
        {
            return this->Variable::get<T>();
        }
    };
    
    /// @brief Value holder for string variables.
    using StringVar = TypedVariable<std::string>;
    /// @brief Value holder for integer variables.
    using IntVar = TypedVariable<long long>;
    /// @brief Value holder for float variables.
    using DoubleVar = TypedVariable<double>;
    /// @brief Value holder for boolean variables.
    using BooleanVar = TypedVariable<bool>;

    namespace cnfs
    {
        using opt_variable = opt<cpptkinter::Variable>;

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
            opt<size_t> expand;
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
        /// @brief Return type of Pack::pack_info().
        ///
        /// @see cnfs::pack_configure
        struct PackInfo
        {
            std::string anchor;
            bool expand;
            std::string fill;
            Misc in;
            long long ipadx;
            long long ipady;
            long long padx;
            long long pady;
            std::string side;
        };
    }

    /// @brief Geometry manager Pack.
    /// 
    /// Base class to use the methods pack_* in every widget.
    struct Pack
    {
        /// @brief %Pack a widget in the parent widget.
        template<cnfs::is_cnf CNF = cnfs::pack_configure>
        void pack_configure(this auto&& self, CNF&& cnf = {})
        {
            self.tk->call("pack", "configure", self._w, self._options(std::forward<CNF>(cnf)));
        }
        /// @copydoc pack_configure
        template<cnfs::is_cnf CNF = cnfs::pack_configure>
        void pack(this auto&& self, CNF&& cnf = {})
        {
            return self.pack_configure(std::forward<CNF>(cnf));
        }

        /// @brief Unmap this widget and do not use it for the packing order.
        void pack_forget(this auto&& self)
        {
            self.tk->template call<void>("pack", "forget", self._w);
        }

        /// Return information about the packing options for this widget.
        cnfs::PackInfo pack_info(this auto&& self)
        {
            return detail::pack_grid_info<cnfs::PackInfo>(self, "pack", "info", self._w);
        }
    };

    namespace cnfs
    {
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
        /// @brief Return type of Place::place_info().
        ///
        /// @see cnfs::place_configure
        struct PlaceInfo
        {
            Misc in;
            std::string x;
            std::string relx;
            std::string y;
            std::string rely;
            std::string width;
            std::string relwidth;
            std::string height;
            std::string relheight;
            std::string anchor;
            std::string bordermode;
        };
    }

    /// @brief Geometry manager Place.
    ///
    /// Base class to use the methods place_* in every widget.
    struct Place
    {
        /// @brief %Place a widget in the parent widget.
        template<cnfs::is_cnf CNF = cnfs::place_configure>
        void place_configure(this auto&& self, CNF&& cnf = {})
        {
            self.tk->call("place", "configure", self._w, self._options(std::forward<CNF>(cnf)));
        }
        /// @copydoc place_configure
        template<cnfs::is_cnf CNF = cnfs::place_configure>
        void place(this auto&& self, CNF&& cnf = {})
        {
            self.place_configure(std::forward<CNF>(cnf));
        }

        // @brief Unmap this widget.
        void place_forget(this auto&& self)
        {
            self.tk->call("place", "forget", self._w);
        }

        cnfs::PlaceInfo place_info(this auto&& self)
        {
            auto str = self.tk->template call<std::string>("place", "info", self._w);
            std::vector<std::string> vec = self.tk->splitlist(str);
            std::map<std::string, std::string> map(std::from_range, std::views::zip(
                vec | /*std::views::*/stride(2),
                vec | /*std::views::*/drop(1) | /*std::views::*/stride(2)
            ));

            auto converter = [&self]<typename T2>(std::string&& v)->T2
            {
                if constexpr (std::same_as<T2, Misc>)
                    return self.Misc::nametowidget(std::move(v));
                else
                    return std::move(v);
            };

            return detail::_splitdict_to_aggregate<cnfs::PlaceInfo>(std::move(map), true, converter);
        }
    };

    namespace cnfs
    {
        /// @brief Argument for Grid::grid_configure().
        struct grid_configure
        {
            /// number - use cell identified with given column (starting with 0)
            opt<size_t> column;
            /// number - this widget will span several columns
            opt<size_t> columnspan;
            /// master - use master to contain this widget
            opt_master in;
            /// number - use cell identified with given row (starting with 0)
            opt<size_t> row;
            /// number - this widget will span several rows
            opt<size_t> rowspan;
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
        /// @brief Return type of Grid::grid_info().
        ///
        /// @see cnfs::grid_configure
        struct GridInfo
        {
            Misc in;
            long long column;
            long long row;
            long long columnspan;
            long long rowspan;
            long long ipadx;
            long long ipady;
            long long padx;
            long long pady;
            std::string sticky;
        };
    }

    /// @brief Geometry manager Grid.
    /// 
    /// Base class to use the methods grid_* in every widget.
    struct Grid
    {
        /// @brief Position a widget in the parent widget in a grid.
        template<cnfs::is_cnf CNF = cnfs::grid_configure>
        void grid_configure(this auto&& self, CNF&& cnf = {})
        {
            self.tk->call("grid", "configure", self._w, self._options(std::forward<CNF>(cnf)));
        }
        /// @copydoc grid_configure
        template<cnfs::is_cnf CNF = cnfs::grid_configure>
        void grid(this auto&& self, CNF&& cnf = {})
        {
            self.grid_configure(std::forward<CNF>(cnf));
        }

        /// @brief Unmap this widget.
        void grid_forget(this auto&& self)
        {
            self.tk->call("grid", "forget", self._w);
        }

        /// @brief Unmap this widget but remember the grid options.
        void grid_remove(this auto&& self)
        {
            self.tk->call("grid", "remove", self._w);
        }

        /// @brief Return information about the options for positioning this widget in a grid.
        cnfs::GridInfo grid_info(this auto&& self)
        {
            return detail::pack_grid_info<cnfs::GridInfo>(self, "grid", "info", self._w);
        }
    };

    /// @brief Internal class.
    class BaseWidget : public Misc
    {
        template<typename T>
        friend class utility::weak;

    protected:
        struct impl : Misc::impl
        {
            std::string widgetName;
            std::string _name;

            /// @brief Destroy this widget and all descendants.
            void destroy() override
            {
                // keeps this from being destroyed before this function returns
                auto temp = this->shared_from_this();

                for (auto&& child : std::vector(std::from_range, std::views::values(this->children)))
                    child.destroy();
                this->tk->call("destroy", this->_w);
                this->master.value().children.erase(this->_name);
                this->Misc::impl::destroy();
            }
        };

    public:
        REF_TO_IMPL(widgetName);
    protected:
        REF_TO_IMPL(_name);

        /// @brief Internal function. Sets up information about children.
        ///
        /// @param override_name only used by Checkbutton.
        template<typename Self>
        void _setup(this Self&& self, const std::optional<Misc>& master_, auto& cnf, std::set<std::string>& ignore_fields, const std::optional<std::string>& override_name = std::nullopt)
        {
            auto&& master = master_.has_value() ? master_.value() : detail::_get_default_root();
            self.master = master;
            self.tk = master.tk;

            std::string name{};
            if (override_name.has_value())
            {
                name = override_name.value();
                ignore_fields.insert("name");
            }
            else if constexpr (requires { cnf.name; })
            {
                utility::invoke_or_and_then([&ignore_fields, &name](auto& v) {
                    name = v;
                    ignore_fields.insert("name");
                }, cnf.name);
            }
            if (name.empty())
            {
                name = hhh::misc::to_lower(reflect::type_name<Self>());

                auto count = master._last_child_ids[name] += 1;

                if (count == 1)
                    name = std::format("!{}", name);
                else
                    name = std::format("!{}{}", name, count);
            }
            self._name = name;
            if (master._w == ".")
                self._w = "." + name;
            else
                self._w = master._w + "." + name;

            auto&& [it, success] = master.children.emplace(self._name, self);
            if (!success)
            {
                it->second.destroy();
                it->second = self;
            }
        }

        /// @brief Initialize a widget.
        ///
        /// master is passed within cnf instead of as a separate argument.
        /// @param widgetName is the name of the widget.
        /// @param cnf is a cnf structure of options to configure the widget.
        /// @param extra is an optional std::vector of additional options to configure the widget.
        /// @param ignore_fields is an optional std::set of fields to ignore in cnf.
        /// @param pimpl is an optional shared pointer to the implementation. Used by derived classes that extend impl.
        template<cnfs::is_cnf CNF>
        void _init_(this auto&& self, const std::string& widgetName, CNF&& cnf, std::vector<_cpptkinter::Tcl_Obj>&& extra = {}, std::set<std::string> ignore_fields = {})
        {
            std::optional<Misc> master{};
            if constexpr (requires { cnf.master; })
            {
                utility::invoke_or_and_then([&master]<typename T>(T && v) {
                    master = std::forward<T>(v);
                }, std::forward<CNF>(cnf).master);
                ignore_fields.insert("master");
            }

            self.widgetName = widgetName;
            self._setup(master, cnf, ignore_fields);
            self.tk->call(self.widgetName, self._w, std::move(extra), self._options(std::forward<CNF>(cnf), ignore_fields));
            DEVIATING_IMPLEMENTATION_WARNING("something with classes in cnf (?)");
        }

        template<std::derived_from<impl> I>
        BaseWidget(const std::shared_ptr<I>& pimpl) :
            Misc(pimpl),
            widgetName(pimpl->widgetName),
            _name(pimpl->_name)
        {

        }
    };

    /// @brief Internal class.
    /// 
    /// Base class for a widget which can be positioned with the geometry managers Pack, Place or Grid.
    struct Widget : BaseWidget, Pack, Grid, Place
    {
        using BaseWidget::BaseWidget;

        /// @brief Exists only to make reflect work.
        Widget() : BaseWidget(std::make_shared<impl>()) { ANNOTATION_WARNING("Exists only to make reflect work."); }
    };

    namespace cnfs
    {
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
        struct add_cascade;

        /// @brief Argument for Menu::add_radiobutton().
        template<typename T>
        struct add_checkbutton
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
        struct add_command
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
        struct add_radiobutton
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
        struct add_separator
        {
            opt_string background;
        };
    }

    /// @brief %Menu widget which allows displaying menu bars, pull-down menus and pop-up menus.
    struct Menu : Widget
    {
        friend class OptionMenu;

        /// @brief Construct a menu widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Menu, cnfs::Menu, "menu", Widget);

        /// @brief Post the menu at position X,Y.
        void tk_popup(long long x, long long y)
        {
            this->tk->call("tk_popup", this->_w, x, y);
        }
        /// @brief Post the menu at position X,Y with entry ENTRY.
        void tk_popup(long long x, long long y, long long entry)
        {
            this->tk->call("tk_popup", this->_w, x, y, entry);
        }

        /// @brief Activate entry at INDEX.
        void activate(const detail::index auto& index)
        {
            this->tk->call(this->_w, "activate", detail::to_index(index));
        }

        /// @brief Internal function.
        template<cnfs::is_cnf CNF>
        void add(const std::string& itemType, CNF&& cnf)
        {
            this->tk->call(this->_w, "add", itemType, this->_options(std::forward<CNF>(cnf)));
        }

        /// @brief Add hierarchical menu item.
        template<cnfs::is_cnf CNF = cnfs::add_cascade>
        void add_cascade(CNF&& cnf = {})
        {
            this->add("cascade", std::forward<CNF>(cnf));
        }

        /// @brief Add checkbutton menu item.
        template<cnfs::is_cnf CNF = cnfs::add_checkbutton<bool>>
        void add_checkbutton(CNF&& cnf = {})
        {
            this->add("checkbutton", std::forward<CNF>(cnf));
        }

        /// @brief Add command menu item.
        template<cnfs::is_cnf CNF = cnfs::add_command>
        void add_command(CNF&& cnf = {})
        {
            this->add("command", std::forward<CNF>(cnf));
        }

        /// @brief Add radio menu item.
        template<cnfs::is_cnf CNF = cnfs::add_radiobutton<int>>
        void add_radiobutton(CNF&& cnf = {})
        {
            this->add("radiobutton", std::forward<CNF>(cnf));
        }

        /// @brief Add separator menu item.
        template<cnfs::is_cnf CNF = cnfs::add_separator>
        void add_separator(CNF&& cnf = {})
        {
            this->add("separator", std::forward<CNF>(cnf));
        }

        /// @brief Internal function.
        template<cnfs::is_cnf CNF>
        void insert(const detail::index auto& index, const std::string& itemType, CNF&& cnf)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), itemType, this->_options(std::forward<CNF>(cnf)));
        }

        /// @brief Add hierarchical menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::add_cascade>
        void insert_cascade(const detail::index auto& index, CNF&& cnf = {})
        {
            this->insert(index, "cascade", std::forward<CNF>(cnf));
        }

        /// @brief Add checkbutton menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::add_checkbutton<bool>>
        void insert_checkbutton(const detail::index auto& index, CNF&& cnf = {})
        {
            this->insert(index, "checkbutton", std::forward<CNF>(cnf));
        }

        /// @brief Add command menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::add_command>
        void insert_command(const detail::index auto& index, CNF&& cnf = {})
        {
            this->insert(index, "command", std::forward<CNF>(cnf));
        }

        /// @brief Add radio menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::add_radiobutton<int>>
        void insert_radiobutton(const detail::index auto& index, CNF&& cnf = {})
        {
            this->insert(index, "radiobutton", std::forward<CNF>(cnf));
        }

        /// @brief Add separator at INDEX.
        template<cnfs::is_cnf CNF = cnfs::add_separator>
        void insert_separator(const detail::index auto& index, CNF&& cnf = {})
        {
            this->insert(index, "separator", std::forward<CNF>(cnf));
        }

        /// @brief Delete menu items at INDEX.
        void delete_(const detail::index auto& index)
        {
            this->delete_(detail::to_index(index), detail::to_index(index));
        }
        /// @brief Delete menu items between INDEX1 and INDEX2 (included).
        void delete_(const detail::index auto& index1_, const detail::index auto& index2_)
        {
            auto&& index1 = detail::to_index(index1_);
            auto&& index2 = detail::to_index(index2_);

            auto num_index1 = this->index(index1);
            auto num_index2 = this->index(index2);

            //if (num_index1 is None) or (num_index2 is None) :
            //    num_index1, num_index2 = 0, -1

            for (long long i = num_index1; i < num_index2 + 1; i++)
            {
                auto ec = this->entryconfig(i);
                if (ec.contains("command"))
                {
                    auto c = this->entrycget<std::string>(i, "command");
                    if (!c.empty())
                        this->deletecommand(c);
                }
            }
            this->tk->call(this->_w, "delete", index1, index2);
        }

        /// @brief Return the resource value of a menu item for OPTION at INDEX.
        template<detail::FromObjConcept R>
        R entrycget(const detail::index auto& index, const std::string& option)
        {
            return this->tk->call<R>(this->_w, "entrycget", detail::to_index(index), "-" + option);
        }

        /// @brief Configure a menu item at INDEX.
        template<cnfs::is_cnf CNF>
        auto entryconfigure(const detail::index auto& index_, CNF&& cnf)
        {
            auto&& index = detail::to_index(index_);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(index)>, long long>)
                return this->_configure({ "entryconfigure", std::to_string(index) }, std::forward<CNF>(cnf));
            else
                return this->_configure({ "entryconfigure", index }, std::forward<CNF>(cnf));
        }
        /// @brief Configure a menu item at INDEX.
        auto entryconfigure(const detail::index auto& index_) -> decltype(this->_configure({}))
        {
            auto&& index = detail::to_index(index_);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(index)>, long long>)
                return this->_configure({ "entryconfigure", std::to_string(index) });
            else
                return this->_configure({ "entryconfigure", index });
        }

        /// @copydoc entryconfigure(long long, CNF&&)
        template<cnfs::is_cnf CNF>
        auto entryconfig(const detail::index auto& index, CNF&& cnf)
        {
            return this->entryconfigure(detail::to_index(index), std::forward<CNF>(cnf));
        }
        /// @copydoc entryconfigure(long long)
        auto entryconfig(const detail::index auto& index) -> decltype(this->entryconfigure(index))
        {
            return this->entryconfigure(detail::to_index(index));
        }

        /// @brief Return the index of a menu item identified by INDEX.
        long long index(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Invoke a menu item identified by INDEX and execute the associated command.
        template<detail::FromObjConcept R = void>
        R invoke(const detail::index auto& index)
        {
            return this->tk->call<R>(this->_w, "invoke", detail::to_index(index));
        }

        /// @brief Display a menu at position X,Y.
        void post(long long x, long long y)
        {
            this->tk->call(this->_w, "post", x, y);
        }

        /// @brief Return the type of the menu item at INDEX.
        std::string type(const detail::index auto& index)
        {
            return this->tk->call<std::string>(this->_w, "type", detail::to_index(index));
        }

        /// @brief Unmap a menu.
        void unpost()
        {
            this->tk->call(this->_w, "unpost");
        }

        /// @brief Return the x-position of the leftmost pixel of the menu item at INDEX.
        long long xposition(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "xposition", detail::to_index(index));
        }

        /// @brief "Return the y-position of the topmost pixel of the menu item at INDEX.
        long long yposition(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "yposition", detail::to_index(index));
        }
    };

    namespace cnfs
    {
        using opt_menu = opt<cpptkinter::Menu>;

        /// @brief Argument for Menu::add_command().
        struct add_cascade
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
            opt_menu menu;
            opt_string state;
            opt<int> underline;
        };

        /// @brief Argument for Toplevel::Toplevel().
        struct Toplevel
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
            opt_menu menu;
            opt_string name;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_relief relief;
            opt_string screen;
            opt_take_focus_value takefocus;
            opt<size_t> use;
            opt_visual_type visual;
            opt_screenunits width;
        };
    }

    /// @brief %Toplevel widget, e.g. for dialogs.
    struct Toplevel : BaseWidget, Wm
    {
    protected:
        template<cnfs::is_cnf CNF>
        void _init_(const std::string& widgetName, CNF&& cnf)
        {
            std::vector<_cpptkinter::Tcl_Obj> extra{};
            std::set<std::string> ignore_fields{};
            reflect::for_each<CNF>([&extra, &ignore_fields, &cnf](auto I) {
                constexpr std::string_view wmkey = reflect::member_name<I, CNF>();

                if constexpr (wmkey == "screen"
                    || wmkey == "class_"
                    || wmkey == "class"
                    || wmkey == "visual"
                    || wmkey == "colormap")
                {
                    std::string opt = "-";
                    if (wmkey.ends_with('_'))
                        opt += wmkey.substr(0, wmkey.size() - 1);
                    else
                        opt += wmkey;


                    utility::invoke_or_and_then([&extra, &opt]<typename T>(T && v) {
                        extra.emplace_back(_cpptkinter::AsObj(opt));
                        extra.emplace_back(_cpptkinter::AsObj(std::forward<T>(v)));
                    }, reflect::get<I>(std::forward<CNF>(cnf)));

                    ignore_fields.insert(std::string(wmkey));
                }
                });

            this->BaseWidget::_init_(widgetName, std::forward<CNF>(cnf), std::move(extra), std::move(ignore_fields));

            auto root = this->_root();
            this->iconname(root.iconname());
            this->title(root.title());
            this->protocol("WM_DELETE_WINDOW", std::function<void()>(std::bind_front(&Toplevel::destroy, *this)));
        }
    public:
        /// @brief Create a new Toplevel widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Toplevel, cnfs::Toplevel, "toplevel", BaseWidget);
    };

    namespace cnfs
    {
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
            opt<size_t> repeatdelay;
            opt<size_t> repeatinterval;
            opt_string state;
            opt_take_focus_value takefocus;
            opt_text text;
            opt_variable textvariable;
            opt<size_t> underline;
            opt_screenunits width;
            opt_screenunits wraplength;
        };
    }

    /// @brief %Button widget.
    /// 
    /// @see TypedButton, cpptkinter::Button()
    struct Button : Widget
    {
        /// @brief Construct a new Button widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Button, cnfs::Button, "button", Widget);

        /// @brief Flash the button.
        /// 
        /// This is accomplished by redisplaying the button several times, alternating between active and normal colors.
        /// At the end of the flash the button is left in the same normal/active state as when the command was invoked.
        /// This command is ignored if the button's state is disabled.
        void flash()
        {
            this->tk->call(this->_w, "flash");
        }

        /// @brief Invoke the command associated with the button.
        void invoke()
        {
            this->tk->call(this->_w, "invoke");
        }
    };

    /// @brief %Canvas widget to display graphical elements like lines or text.
    struct Canvas;

    namespace cnfs
    {
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
            opt<size_t> underline;
            opt_variable variable;
            opt_screenunits width;
            opt_screenunits wraplength;
        };
    }

    /// @brief %Checkbutton widget which is either in on- or off-state.
    struct Checkbutton : Widget
    {
        friend BaseWidget;

    protected:
        template<typename Self>
        void _setup(this Self& self, const std::optional<Misc>& master_, auto& cnf, std::set<std::string>& ignore_fields)
        {
            //Because Checkbutton defaults to a variable with the same name as the widget, Checkbutton default names must be globally unique, not just unique within the parent widget.

            bool has_name = false;
            if constexpr (requires{ cnf.name; })
                utility::invoke_or_and_then([&](auto& val) { has_name = !val.empty(); }, cnf.name);

            if(!has_name)
            {
                auto name = hhh::misc::to_lower(reflect::type_name<Self>());
                detail::_checkbutton_count += 1;
                // To avoid collisions with ttk.Checkbutton, use the different name template.
                self.BaseWidget::_setup(master_, cnf, ignore_fields, std::format("!{}-{}", name, detail::_checkbutton_count));
            }
            else
                self.BaseWidget::_setup(master_, cnf, ignore_fields);
        }

    public:
        /// @brief Construct a new Button widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Checkbutton, cnfs::Checkbutton<long long>, "checkbutton", Widget);

        /// @brief Put the button in off-state.
        void deselect()
        {
            this->tk->call(this->_w, "deselect");
        }

        /// @brief Flash the button.
        void flash()
        {
            this->tk->call(this->_w, "flash");
        }

        /// @brief Toggle the button and invoke a command if given as resource.
        void invoke()
        {
            return this->tk->call(this->_w, "invoke");
        }

        /// @brief Put the button in on-state.
        void select()
        {
            this->tk->call(this->_w, "select");
        }

        /// @brief Toggle the button.
        void toggle()
        {
            this->tk->call(this->_w, "toggle");
        }
    };

    /// @brief A checkbutton widget with a defined value type.
    ///
    /// Checkbutton has an arbitrary value type. TypedCheckbutton restricts the value to a specific type.
    /// @tparam T The value type.
    /// @see Checkbutton
    template<detail::AsObjConcept T>
    struct TypedCheckbutton : Checkbutton
    {
        using value_type = T;

        /// @brief Construct a new TypedCheckbutton widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(TypedCheckbutton, cnfs::Checkbutton<value_type>, "radiobutton", Checkbutton);
    };

    namespace cnfs
    {
        using opt_entry_validate_command = opt<detail::EntryValidateCommand>;

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
            opt<size_t> insertofftime;
            opt<size_t> insertontime;
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
            opt_xy_scroll_command xscrollcommand;
        };
    }

    /// @brief %Entry widget which allows displaying simple text.
    struct Entry : Widget, XView
    {
        /// @brief Construct a new Entry widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Entry, cnfs::Entry, "entry", Widget);

        /// @brief Delete a character.
        void delete_(const detail::index auto& index)
        {
            this->tk->call(this->_w, "delete", detail::to_index(index));
        }
        /// @brief Delete text from FIRST to LAST (not included).
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return the text.
        std::string get()
        {
            return this->tk->call<std::string>(this->_w, "get");
        }

        /// @brief Insert cursor at INDEX.
        void icursor(const detail::index auto& index)
        {
            this->tk->call(this->_w, "icursor", detail::to_index(index));
        }

        /// @brief Return position of cursor.
        long long index(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Insert STRING at INDEX.
        void insert(const detail::index auto& index, const std::string& string)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), string);
        }

        /// @brief unknown
        void scan_mark(long long x)
        {
            this->tk->call(this->_w, "scan", "mark", x);
        }

        /// @brief unknown
        void scan_dragto(long long x)
        {
            this->tk->call(this->_w, "scan", "dragto", x);
        }

        /// @brief Adjust the end of the selection near the cursor to INDEX.
        void selection_adjust(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "adjust", detail::to_index(index));
        }

        /// @copydoc selection_adjust
        void select_adjust(const detail::index auto& index)
        {
            this->selection_adjust(detail::to_index(index));
        }

        /// @brief Clear the selection if it is in this widget.
        void selection_clear()
        {
            this->tk->call(this->_w, "selection", "clear");
        }

        /// @copydoc selection_clear
        void select_clear()
        {
            this->selection_clear();
        }

        /// @brief Set the fixed end of a selection to INDEX.
        void selection_from(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "from", detail::to_index(index));
        }

        /// @copydoc selection_from
        void select_from(const detail::index auto& index)
        {
            this->selection_from(detail::to_index(index));
        }

        /// @brief Return true if there are characters selected in the entry, false otherwise.
        bool selection_present()
        {
            return this->tk->call<bool>(this->_w, "selection", "present");
        }

        /// @copydoc selection_present
        bool select_present()
        {
            return this->selection_present();
        }

        /// @brief Set the selection from START to END (not included).
        void selection_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->tk->call(this->_w, "selection", "range", detail::to_index(start), detail::to_index(end));
        }

        /// @copydoc selection_range
        void select_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->selection_range(detail::to_index(start), detail::to_index(end));
        }

        /// @brief Set the variable end of a selection to INDEX.
        void selection_to(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "to", detail::to_index(index));
        }

        /// @copydoc selection_to
        void select_to(const detail::index auto& index)
        {
            this->selection_to(detail::to_index(index));
        }
    };

    namespace cnfs
    {
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
    }

    /// @brief %Frame widget which may contain other widgets and can have a 3D border.
    struct Frame : Widget
    {
    protected:
        template<cnfs::is_cnf CNF>
        void _init_(const std::string& widgetName, CNF&& cnf)
        {
            std::vector<_cpptkinter::Tcl_Obj> extra{};
            std::set<std::string> ignore_fields{};
            if constexpr (requires { cnf.class_; })
            {
                utility::invoke_or_and_then([&]<typename T>(T && v) {
                    extra.emplace_back(_cpptkinter::AsObj("-class"));
                    extra.emplace_back(_cpptkinter::AsObj(std::forward<T>(v)));
                    ignore_fields.insert("class_");
                }, cnf.class_);
            }

            this->Widget::_init_(widgetName, std::forward<CNF>(cnf), std::move(extra), std::move(ignore_fields));
        }

    public:
        /// @brief Construct a frame widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Frame, cnfs::Frame, "frame", Widget);
    };

    namespace cnfs
    {
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
            opt<size_t> underline;
            opt_screenunits width;
            opt_screenunits wraplength;
        };
    }

    /// @brief %Label widget which can display text and bitmaps.
    struct Label : Widget
    {
        /// @brief Construct a label widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Label, cnfs::Label, "label", Widget);
    };

    namespace cnfs
    {
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
            opt_xy_scroll_command xscrollincrement;
            opt_xy_scroll_command yscrollincrement;
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
    }

    /// @brief %Listbox widget which can display a list of strings.
    struct Listbox : Widget
    {
        /// @brief Construct a listbox widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Listbox, cnfs::Listbox, "listbox", Widget);

        /// @brief Activate item identified by INDEX.
        void activate(const detail::index auto& index)
        {
            this->tk->call(this->_w, "activate", detail::to_index(index));
        }

        /// @brief Return a tuple of X1,Y1,X2,Y2 coordinates for a rectangle which encloses the item identified by the given index.
        std::array<long long, 4> bbox(const detail::index auto& index)
        {
            return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_index(index));
        }

        /// @brief Return the indices of currently selected item.
        std::vector<long long> curselection()
        {
            return this->tk->call<std::vector<long long>>(this->_w, "curselection");
        }

        /// @brief Delete item at index.
        void delete_(const detail::index auto& index)
        {
            this->tk->call(this->_w, "delete", detail::to_index(index));
        }
        /// @brief Delete items from FIRST to LAST (included).
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Get the item at index.
        template<detail::FromObjConcept R>
        R get(const detail::index auto& index)
        {
            if constexpr (std::same_as<R, std::string>)
                return this->tk->call<R>(this->_w, "get", detail::to_index(index));
            else
            {
                auto res = this->tk->call<std::variant<R, std::string>>(this->_w, "get", detail::to_index(index));
                if (std::holds_alternative<R>(res))
                    return std::get<R>(res);
                else
                {
                    if (std::get<std::string>(res).empty())
                        throw detail::construct_exception<std::invalid_argument>(std::format("index {} was out of bounds", detail::to_index(index)));
                    else
                        throw detail::construct_exception<std::invalid_argument>(std::format("expected type {} but got std::string", reflect::type_name<R>()));
                }
            }
        }
        /// @brief Get list of items from FIRST to LAST (included).
        template<detail::FromObjConcept R>
        std::vector<R> get(const detail::index auto& first, const detail::index auto& last)
        {
            return this->tk->call<std::vector<R>>(this->_w, "get", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return index of item identified with INDEX.
        long long index(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Insert ELEMENTS at INDEX.
        void insert(const detail::index auto& index, const detail::AsObjConcept auto&...elements)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), elements...);
        }
        /// @copydoc insert(const detail::index auto&, const detail::AsObjConcept auto&...)
        void insert(const detail::index auto& index, const detail::range_of_AsObj auto& elements)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), elements | std::views::transform([](auto& val) { return _cpptkinter::AsObj(val); }));
        }

        /// @brief Get index of item which is nearest to y coordinate Y.
        long long nearest(long long y)
        {
            return this->tk->call<long long>(this->_w, "nearest", y);
        }

        /// @brief unknown
        void scan_mark(long long x, long long y)
        {
            this->tk->call(this->_w, "scan", "mark", x, y);
        }

        /// @brief unknown
        void scan_dragto(long long x, long long y)
        {
            this->tk->call(this->_w, "scan", "dragto", x, y);
        }

        /// @brief Scroll such that INDEX is visible.
        void see(const detail::index auto& index)
        {
            this->tk->call(this->_w, "see", detail::to_index(index));
        }

        /// @brief Set the fixed end oft the selection to INDEX.
        void selection_anchor(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "anchor", detail::to_index(index));
        }

        /// @copydoc selection_anchor
        void select_anchor(const detail::index auto& index)
        {
            this->selection_anchor(detail::to_index(index));
        }

        /// @brief Clear the selection at index.
        void selection_clear(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "clear", detail::to_index(index));
        }
        /// @brief Clear the selection from FIRST to LAST (included).
        void selection_clear(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "selection", "clear", detail::to_index(first), detail::to_index(last));
        }

        /// @copydoc selection_clear(const detail::index auto&)
        void select_clear(const detail::index auto& index)
        {
            this->selection_clear(detail::to_index(index));
        }
        /// @copydoc selection_clear(const detail::index auto&, const detail::index auto&)
        void select_clear(const detail::index auto& first, const detail::index auto& last)
        {
            this->selection_clear(detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return True if INDEX is part of the selection.
        bool selection_includes(const detail::index auto& index)
        {
            return this->tk->call<bool>(this->_w, "selection", "includes", detail::to_index(index));
        }

        /// @copydoc selection_includes
        bool select_includes(const detail::index auto& index)
        {
            return this->selection_includes(detail::to_index(index));
        }

        /// @brief Set the selection for index without changing the currently selected elements.
        void selection_set(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "set", detail::to_index(index));
        }
        /// @brief Set the selection from FIRST to LAST (included) without changing the currently selected elements.
        void selection_set(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "selection", "set", detail::to_index(first), detail::to_index(last));
        }

        /// @copydoc selection_set(const detail::index auto&)
        void select_set(const detail::index auto& index)
        {
            this->selection_set(detail::to_index(index));
        }
        /// @copydoc selection_set(const detail::index auto&, const detail::index auto&)
        void select_set(const detail::index auto& first, const detail::index auto& last)
        {
            this->selection_set(detail::to_index(first), detail::to_index(last));
        }

        /// @brief Return the number of elements in the listbox.
        long long size()
        {
            return this->tk->call<long long>(this->_w, "size");
        }

        /// @brief Return the resource value for an ITEM and an OPTION.
        template<detail::FromObjConcept R>
        void itemcget(const detail::index auto& index, const std::string& option)
        {
            return this->tk->call<R>(this->_w, "itemcget", detail::to_index(index), "-" + option);
        }

        /// @brief Get allowed keywords.
        std::map<std::string, std::array<std::string, 5>> itemconfigure(const detail::index auto& index)
        {
            std::string index_{};
            if constexpr (std::same_as<std::remove_cvref_t<decltype(detail::to_index(index))>, std::string>)
                index_ = detail::to_index(index);
            else
                index_ = std::to_string(detail::to_index(index));
            auto map = this->_configure({ "itemconfigure", index_ });

            auto key_view = map | std::views::keys;
            auto value_view = map | std::views::values | std::views::transform([](auto& arr) {
                std::array<std::string, 5> new_arr{};
                std::ranges::move(arr | std::views::transform([](auto& v) { return std::get<std::string>(v); }), new_arr.begin());
                return new_arr;
                });

            return std::views::zip(key_view, value_view) | std::ranges::to<std::map>();
        }
        /// @brief Configure resources of an ITEM.
        template<cnfs::is_cnf CNF = cnfs::Listbox_itemconfigure>
        void itemconfigure(const detail::index auto& index, CNF&& cnf = {})
        {
            auto&& index_ = detail::to_index(index);
            if constexpr (std::same_as<std::remove_cvref_t<decltype(index)>, std::string>)
                this->_configure({ "itemconfigure", index_ }, std::forward<CNF>(cnf));
            else
                this->_configure({ "itemconfigure", std::to_string(index_) }, std::forward<CNF>(cnf));
        }

        /// @copydoc itemconfigure(const detail::index auto&)
        std::map<std::string, std::array<std::string, 5>> itemconfig(const detail::index auto& index)
        {
            return this->itemconfigure(detail::to_index(index));
        }
        /// @copydoc itemconfigure(const detail::index auto&, CNF&&)
        template<cnfs::is_cnf CNF = cnfs::Listbox_itemconfigure>
        void itemconfig(const detail::index auto& index, CNF&& cnf = {})
        {
            this->itemconfigure(detail::to_index(index), std::forward<CNF>(cnf));
        }
    };

    /// @brief %Listbox widget with a defined element type.
    /// 
    /// Listbox has an arbitrary element type. TypedListbox restricts the element type to a specific type.
    /// @tparam T The element type.
    /// @see Listbox
    template<detail::FromObjConcept T>
    struct TypedListbox : Listbox
    {
        using value_type = T;

        /// @brief Construct a new TypedListbox widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(TypedListbox, cnfs::Listbox, "listbox", Listbox);

        /// @copydoc Listbox::get(const detail::index auto&)
        value_type get(const detail::index auto& index)
        {
            return this->Listbox::get<value_type>(detail::to_index(index));
        }
        /// @copydoc Listbox::get(const detail::index auto&, const detail::index auto&)
        std::vector<value_type> get(const detail::index auto& first, const detail::index auto& last)
        {
            return this->Listbox::get<value_type>(detail::to_index(first), detail::to_index(last));
        }

        /// @copydoc Listbox::insert(const detail::index auto&, const detail::AsObjConcept auto&...)
        void insert(const detail::index auto& index, const std::convertible_to<value_type> auto&...elements)
        {
            this->Listbox::insert(detail::to_index(index), static_cast<value_type>(elements)...);
        }
        /// @copydoc insert(const detail::index auto&, const detail::range_of_AsObj auto&)
        void insert(const detail::index auto& index, const utility::range_of_convertible_to<value_type> auto& elements)
        {
            this->Listbox::insert(detail::to_index(index), elements | std::views::transform([](auto& val) { return static_cast<value_type>(val); }));
        }
    };

    namespace cnfs
    {
        /// @brief Argument for Menubutton::Menubutton().
        struct Menubutton
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
            opt_string direction;
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
            opt_menu menu;
            opt_string name;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_relief relief;
            opt_string state;
            opt_take_focus_value takefocus;
            opt_text text;
            opt_variable textvariable;
            opt<size_t> underline;
            opt_screenunits width;
            opt_screenunits wraplength;
        };
    }

    /// @brief %Menubutton widget, obsolete since Tk8.0.
    struct Menubutton : Widget
    {
        /// @brief Construct a menubutton widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Menubutton, cnfs::Menubutton, "menubutton", Widget);
    };

    /// @brief %Message widget to display multiline text. Obsolete since Label does it too.
    struct [[deprecated("according to tkinter")]] Message;

    namespace cnfs
    {
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
            opt<size_t> underline;
            opt<T> value;
            opt_variable variable;
            opt_screenunits width;
            opt_screenunits wraplength;
        };
    }

    /// @brief %Radiobutton widget which shows only one of several buttons in on-state.
    struct Radiobutton : Widget
    {
        /// @brief Construct a radiobutton widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Radiobutton, cnfs::Radiobutton<long long>, "radiobutton", Widget);

        /// @brief Put the button in off-state.
        void deselect()
        {
            this->tk->call(this->_w, "deselect");
        }

        /// @brief Flash the button.
        void flash()
        {
            this->tk->call(this->_w, "flash");
        }

        /// @brief Toggle the button and invoke a command if given as resource.
        void invoke()
        {
            return this->tk->call(this->_w, "invoke");
        }

        /// @brief Put the button in on-state.
        void select()
        {
            this->tk->call(this->_w, "select");
        }
    };

    /// @brief A radiobutton widget with a defined value type.
    ///
    /// Radiobutton has an arbitrary value type. TypedRadiobutton restricts the value to a specific type.
    /// @tparam T The value type.
    /// @see Radiobutton
    template<detail::AsObjConcept T>
    struct TypedRadiobutton : Button
    {
        using value_type = T;

        /// @brief Construct a new TypedRadiobutton widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(TypedRadiobutton, cnfs::Radiobutton<T>, "radiobutton", Button);
    };

    namespace cnfs
    {
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
    }

    /// @brief %Scale widget which can display a numerical scale.
    struct Scale : Widget
    {
        /// @brief Construct a scale widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Scale, cnfs::Scale, "scale", Widget);

        /// @brief Get the current value as integer or float.
        double get()
        {
            return this->tk->call<double>(this->_w, "get");
        }

        /// @brief Set the current value.
        double set(double value)
        {
            return this->tk->call<double>(this->_w, "set", value);
        }

        /// @brief Return a tuple (X,Y) of the point along the centerline of the trough that corresponds to the current value.
        std::array<long long, 2> coords()
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "coords");
        }
        /// @brief Return a tuple (X,Y) of the point along the centerline of the trough that corresponds to VALUE.
        std::array<long long, 2> coords(double value)
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "coords", value);
        }

        /// @brief Return where the point X,Y lies. Valid return values are "slider", "though1" and "though2".
        std::string identify(detail::ScreenUnits x, detail::ScreenUnits y)
        {
            return this->tk->call<std::string>(this->_w, "identify", x, y);
        }
    };

    namespace cnfs
    {
		/// @brief Argument for Scrollbar::Scrollbar().
        struct Scrollbar
        {

        };
    }

    /// @brief %Scrollbar widget which displays a slider at a certain position.
    struct Scrollbar : Widget
    {
		/// @brief Construct a scrollbar widget.
		CNF_CONSTRUCTOR_AND_ASSIGNMENT(Scrollbar, cnfs::Scrollbar, "scrollbar", Widget);

		/// @brief Get the active element.
        /// 
        /// @returns The name of the element that is currently active, or "" if no element is active.
        std::string activate()
		{
			return this->tk->call<std::string>(this->_w, "activate");
		}
        /// @brief Marks the element indicated by index as active.
        /// 
        /// The only index values understood by this method are "arrow1", "slider", or "arrow2".
        /// If any other value is specified then no element of the scrollbar will be active.
        void activate(const std::string& index)
        {
			this->tk->call(this->_w, "activate", index);
        }

        /// @brief Return the fractional change of the scrollbar setting if it would be moved by DELTAX or DELTAY pixels.
        double delta(long long deltax, long long deltay)
        {
			this->tk->call<double>(this->_w, "delta", deltax, deltay);
        }

        /// @brief Return the fractional value which corresponds to a slider position of X, Y.
        double fraction(long long x, long long y)
        {
			return this->tk->call<double>(this->_w, "fraction", x, y);
        }

        /// @brief Return the element under position X,Y as one of "arrow1", "slider", "arrow2" or "".
        std::string identify(long long x, long long y)
        {
			return this->tk->call<std::string>(this->_w, "identify", x, y);
        }

        /// @brief Return the current fractional values (upper and lower end) of the slider position.
        std::array<double, 2> get()
        {
			return this->tk->call<std::array<double, 2>>(this->_w, "get");
        }

        /// @brief Set the fractional values of the slider position (upper and lower ends as value between 0 and 1).
        void set(double first, double last)
        {
			this->tk->call(this->_w, "set", first, last);
        }
    };

    /// @brief %Text widget which can display text in various forms.
    struct Text;

    namespace detail
    {
        /// @brief Satisfied if R is a range of a type convertible to std::string.
        template<typename R>
        concept sized_range_of_string = std::ranges::sized_range<R> && std::convertible_to<std::ranges::range_value_t<R>, std::string>;

        /// @brief Internal class. It wraps the command in the widget OptionMenu.
        struct _setit
        {
            StringVar _var;
            std::string _value;
            std::function<void(const StringVar&)> _callback;

            void operator()()
            {
                this->_var.set(this->_value);
                if (this->_callback)
                    this->_callback(this->_var);
            }
        };
    }

    /// @brief %OptionMenu which allows the user to select a value from a menu.
    class OptionMenu : public Menubutton
    {
        template<typename T>
        friend class utility::weak;
    protected:
        struct impl : Menubutton::impl
        {
            std::optional<Menu> _menu;
            std::string menuname;

            /// @brief Destroy this widget and the associated menu.
            void destroy() override
            {
                // keeps this from being destroyed before this function returns
                auto temp = this->shared_from_this();

                this->_menu.reset();

                this->BaseWidget::impl::destroy();
            }
        };

    private:
        REF_TO_IMPL(_menu);
    public:
        REF_TO_IMPL(menuname);

    protected:
        void _init_(const Misc& master, const StringVar& variable, const detail::sized_range_of_string auto& values, const std::function<void(const StringVar&)>& command)
        {
            if (values.size() == 0)
                throw detail::construct_exception<std::invalid_argument>("values must be non-empty");

            this->Menubutton::_init_("menubutton",
                cnfs::Menubutton{ .master = master, .anchor = "c", .borderwidth = 2, .highlightthickness = 2, .indicatoron = 1, .relief = constants::RAISED, .textvariable = variable });

            this->widgetName = "tk_optionMenu";
            auto&& menu = this->_menu.emplace(cnfs::Menu{ .master = *this, .name = "menu", .tearoff = 0 });
            this->menuname = menu._w;

            for (auto& v : values)
                menu.add_command({ .command = detail::_setit(variable, v, command), .label = v });

            (*this)["menu"] = menu;
        }

        template<std::derived_from<impl> I>
        OptionMenu(const std::shared_ptr<I>& pimpl) :
            Menubutton(pimpl),
            _menu(pimpl->_menu),
            menuname(pimpl->menuname)
        {

        }
    public:
        DEFINE_ASSIGNMENT_OPERATOR(OptionMenu);

        /// @brief Construct an optionmenu widget.
        OptionMenu(const Misc& master, const StringVar& variable, const detail::sized_range_of_string auto& values, const std::function<void(const StringVar&)>& command = {}) :
            OptionMenu(std::make_shared<impl>())
        {
            this->_init_(master, variable, values, command);
        }

        template<typename R>
            requires detail::FromObjConcept<R> || std::same_as<R, Menu> || std::same_as<R, std::optional<Menu>>
        R _getitem_(const std::string& name, std::type_identity<R>)
        {
            if (name == "menu")
            {
                if constexpr (std::same_as<R, Menu>)
                    return this->_menu.value();
                else if constexpr (std::same_as<R, std::optional<Menu>>)
					return this->_menu;
            }
            else
            {
                if constexpr (detail::FromObjConcept<R>)
                    return this->Menubutton::_getitem_(name, std::type_identity<R>{});
            }

            throw detail::construct_exception<std::invalid_argument>(std::format("requested type {} not compatible with provided ressource name {}", reflect::type_name<R>, name));
        }
    };

    /// @brief %Base class for images.
    struct Image;

    /// @brief %Widget which can display images in PGM, PPM, GIF, PNG format.
    struct PhotoImage;

    /// @brief %Widget which can display images in XBM format.
    struct BitmapImage;

    namespace cnfs
    {
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
            opt<size_t> insertofftime;
            opt<size_t> insertontime;
            opt_screenunits insertwidth;
            opt_entry_validate_command invalidcommand;
            opt_entry_validate_command invcmd;
            opt_string justify;
            opt_string name;
            opt_string readonlybackground;
            opt_relief relief;
            opt<size_t> repeatdelay;
            opt<size_t> repeatinterval;
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
            opt_xy_scroll_command xscrollcommand;
        };
    }

    /// @brief %Spinbox widget.
    struct Spinbox : Widget
    {
        /// @brief Construct a spinbox widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Spinbox, cnfs::Spinbox, "spinbox", Widget);

        /// @brief Return a tuple of X1,Y1,X2,Y2 coordinates for a rectangle which encloses the character given by index.
        /// 
        /// The first two elements of the list give the x and y coordinates of the upper - left corner of the screen area covered by the character (in pixels relative to the widget)
        /// and the last two elements give the width and height of the character, in pixels.
        /// The bounding box may refer to a region outside the visible area of the window.
        std::array<long long, 4> bbox(const detail::index auto& index)
        {
            return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", detail::to_index(index));
        }

        /// @brief Delete one element of the spinbox.
        void delete_(const detail::index auto& index)
        {
            this->tk->call(this->_w, "delete", detail::to_index(index));
        }
        /// @brief Delete elements of the spinbox.
        ///
        /// First is the index of the first character to delete, and last is the index of the character just after the last one to delete.
        /// If last isn't specified it defaults to first + 1, i.e. a single character is deleted.
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", detail::to_index(first), detail::to_index(last));
        }

        /// @brief Returns the spinbox's string.
        std::string get()
        {
            return this->tk->call<std::string>(this->_w, "get");
        }

        /// @brief Alter the position of the insertion cursor.
        /// 
        /// The insertion cursor will be displayed just before the character given by index.
        void icursor(const detail::index auto& index)
        {
            this->tk->call(this->_w, "icursor", detail::to_index(index));
        }

        /// @brief Returns the name of the widget at position x, y
        ///
        /// @returns none, buttondown, buttonup, entry
        std::string identify(long long x, long long y)
        {
            return this->tk->call<std::string>(this->_w, "identify", x, y);
        }

        /// @brief Returns the numerical index corresponding to index.
        long long index(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "index", detail::to_index(index));
        }

        /// @brief Insert string s at index.
        void insert(const detail::index auto& index, const std::string& s)
        {
            this->tk->call(this->_w, "insert", detail::to_index(index), s);
        }

        /// @brief Causes the specified element to be invoked
        ///
        /// The element could be buttondown or buttonup triggering the action associated with it.
        void invoke(const std::string& element)
        {
            this->tk->call(this->_w, "invoke", element);
        }

        /// @brief Internal function.
        // void scan();

        /// @brief Records x and the current view in the spinbox window.
        /// 
        /// Used in conjunction with later scan dragto commands. Typically this command is associated with a mouse button press in the widget.
        void scan_mark(long long x)
        {
            this->tk->call(this->_w, "scan", "mark", x);
        }

        /// @brief Compute the difference between the given x argument and the x argument to the last scan mark command
        /// 
        /// It then adjusts the view left or right by 10 times the difference in x - coordinates.
        /// This command is typically associated with mouse motion events in the widget, to produce the effect of dragging the spinbox at high speed through the window.
        void scan_dragto(long long x)
        {
            this->tk->call(this->_w, "scan", "dragto", x);
        }

        /// @brief Internal function.
        // void selection();

        /// @brief Locate the end of the selection nearest to the character given by index, then adjust that end of the selection to be at index (i.e including but not going beyond index).
        /// 
        /// The other end of the selection is made the anchor point for future select to commands.
        /// If the selection isn't currently in the spinbox, then a new selection is created to include the characters between index and the most recent selection anchor point, inclusive.
        void selection_adjust(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "adjust", detail::to_index(index));
        }

        /// @brief Clear the selection
        /// 
        /// If the selection isn't in this widget then the command has no effect.
        void selection_clear()
        {
            this->tk->call(this->_w, "selection", "clear");
        }

        /// @brief Gets the currently selected element.
        std::string selection_element()
        {
            return this->tk->call<std::string>(this->_w, "selection", "element");
        }
        /// @brief Sets the currently selected element.
        /// 
        /// If a spinbutton element is specified, it will be displayed depressed.
        void selection_element(const std::string& element)
        {
            this->tk->call(this->_w, "selection", "element", element);
        }

        /// @brief Set the fixed end of a selection to INDEX.
        void selection_from(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "from", detail::to_index(index));
        }

        /// @brief Return true if there are characters selected in the spinbox, false otherwise.
        bool selection_present()
        {
            return this->tk->call<bool>(this->_w, "selection", "present");
        }

        /// @brief Set the selection from START to END (not included).
        void selection_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->tk->call(this->_w, "selection", "range", detail::to_index(start), detail::to_index(end));
        }

        /// @brief Set the variable end of a selection to INDEX.
        void selection_to(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "to", detail::to_index(index));
        }
    };

    namespace cnfs
    {
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
    }

    /// @brief %Labelframe widget.
    struct LabelFrame : Widget
    {
        /// @brief Construct a labelframe widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(LabelFrame, cnfs::LabelFrame, "labelframe", Widget);
    };

    namespace cnfs
    {
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

        /// @brief Argument for PanedWindow::add().
        struct PanedWindow_add
        {
            Widget child;
            opt<Widget> after;
            opt<Widget> before;
            opt_screenunits height;
            opt_screenunits minsize;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_string style;
            opt_string stretch;
            opt_screenunits width;
        };

        /// @brief Argument for PanedWindow::paneconfigure().
        struct paneconfigure
        {
            Widget tagOrId;
            opt<Widget> after;
            opt<Widget> before;
            opt_screenunits height;
            opt_screenunits minsize;
            opt_screenunits padx;
            opt_screenunits pady;
            opt_string style;
            opt_string stretch;
            opt_screenunits width;
        };
    }

    /// @brief %Panedwindow widget.
    struct PanedWindow : Widget
    {
        /// @brief Construct a panedwindow widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(PanedWindow, cnfs::PanedWindow, "panedwindow", Widget);

        /// @brief Add a child widget to the panedwindow in a new pane.
        /// 
        /// The child argument is the name of the child widget followed by pairs of arguments that specify how to manage the windows.
        /// The possible options and values are the ones accepted by the paneconfigure() method.
        template<cnfs::is_cnf CNF = cnfs::PanedWindow_add>
        void add(CNF&& cnf)
        {
            this->tk->call(this->_w, "add", std::forward<CNF>(cnf).child, this->_options(std::forward<CNF>(cnf), { "child" }));
        }

        /// @brief Remove the pane containing child from the panedwindow
        /// 
        /// All geometry management options for child will be forgotten.
        void remove(const std::derived_from<Widget> auto& child)
        {
            this->tk->call(this->_w, "forget", child);
        }

        /// @copydoc remove
        void forget(const std::derived_from<Widget> auto& child)
        {
            this->remove(child);
        }

        /// @brief Not implemented
        /// 
        /// Identify the panedwindow component at point x, y.
        /// 
        /// If the point is over a sash or a sash handle, the result is a two element list containing the index of the sash or handle, and a word indicating whether it is over a sash or a handle, such as { 0 sash } or {2 handle}.
        /// If the point is over any other part of the panedwindow, the result is an empty list.
        void identify(long long x, long long y);

        /// @brief 
        // void proxy();

        /// @brief Return the x and y pair of the most recent proxy location.
        std::array<long long, 2> proxy_coord()
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "proxy", "coord");
        }

        /// @brief Remove the proxy from the display.
        void proxy_forget()
        {
            this->tk->call(this->_w, "proxy", "forget");
        }

        /// @brief Place the proxy at the given x and y coordinates.
        void proxy_place(long long x, long long y)
        {
            this->tk->call(this->_w, "proxy", "place", x, y);
        }

        /// @brief 
        // void sash();

        /// @brief Return the current x and y pair for the sash given by index.
        /// 
        /// Index must be an integer between 0 and 1 less than the number of panes in the panedwindow.
        /// The coordinates given are those of the top left corner of the region containing the sash.
        /// pathName sash dragto index x y This command computes the difference between the given coordinates and the coordinates given to the last sash coord command for the given sash.
        /// It then moves that sash the computed difference.
        std::array<long long, 2> sash_coord(const detail::index auto& index)
        {
            return this->tk->call<std::array<long long, 2>>(this->_w, "sash", "coord", detail::to_index(index));
        }

        /// @brief Records x and y for the sash given by index.
        /// 
        /// Used in conjunction with later dragto commands to move the sash.
        void sash_mark(const detail::index auto& index)
        {
            this->tk->call(this->_w, "sash", "mark", detail::to_index(index));
        }

        /// @brief Place the sash given by index at the given coordinates.
        void sash_place(const detail::index auto& index, long long x, long long y)
        {
            this->tk->call(this->_w, "sash", "place", detail::to_index(index), x, y);
        }

        /// @brief Query a management option for window.
        /// 
        /// Option may be any value allowed by the paneconfigure subcommand.
        void panecget(const std::derived_from<Widget> auto& child, const std::string& option)
        {
            this->tk->call(this->_w, "panecget", child, "-" + option);
        }

        /// @brief Query or modify the management options for window.
        /// 
        /// If no option is specified, returns a list describing all of the available options for pathName.
        /// If option is specified with no value, then the command returns a list describing the one named option
        /// (this list will be identical to the corresponding sublist of the value returned if no option is specified).
        /// If one or more option - value pairs are specified, then the command modifies the given widget option(s) to have the given value(s);
        /// in this case the command returns an empty string. The following options are supported:
        /// 
        /// - <b>after window</b>: Insert the window after the window specified.window should be the name of a window already managed by pathName.
        /// - <b>before window</b>: Insert the window before the window specified. window should be the name of a window already managed by pathName.
        /// - <b>height size</b>: Specify a height for the window. The height will be the outer dimension of the window including its border, if any.
        /// If size is an empty string, or if - height is not specified, then the height requested internally by the window will be used initially;
        /// the height may later be adjusted by the movement of sashes in the panedwindow. Size may be any value accepted by Tk_GetPixels.
        /// - <b>minsize n</b>: Specifies that the size of the window cannot be made less than n.
        /// This constraint only affects the size of the widget in the paned dimension -- the x dimension for horizontal panedwindows, the y dimension for vertical panedwindows.
        /// May be any value accepted by Tk_GetPixels.
        /// - <b>padx n</b>: Specifies a non - negative value indicating how much extra space to leave on each side of the window in the X - direction.
        /// The value may have any of the forms accepted by Tk_GetPixels.
        /// - <b>pady n</b>: Specifies a non - negative value indicating how much extra space to leave on each side of the window in the Y - direction.
        /// The value may have any of the forms accepted by Tk_GetPixels.
        /// - <b>sticky style</b>: If a window's pane is larger than the requested dimensions of the window, this option may be used to position(or stretch) the window within its pane.
        /// Style is a string that contains zero or more of the characters n, s, e or w. The string can optionally contains spaces or commas, but they are ignored.
        /// Each letter refers to a side (north, south, east, or west) that the window will "stick" to.
        /// If both n and s (or e and w) are specified, the window will be stretched to fill the entire height (or width) of its cavity.
        /// - <b>stretch when</b>: Controls how extra space is allocated to each of the panes. When is one of always, first, last, middle, and never.
        /// The panedwindow will calculate the required size of all its panes. Any remaining (or deficit) space will be distributed to those panes marked for stretching.
        /// The space will be distributed based on each panes current ratio of the whole. The when values have the following definition:
        ///     - <b>always</b>: This pane will always stretch.
        ///     - <b>first</b>: Only if this pane is the first pane (left-most or top-most) will it stretch.
        ///     - <b>last</b>: Only if this pane is the last pane (right-most or bottom-most) will it stretch. This is the default value.
        ///     - <b>middle</b>: Only if this pane is not the first or last pane will it stretch.
        ///     - <b>never</b>: This pane will never stretch.
        /// - <b>width size</b>: Specify a width for the window.The width will be the outer dimension of the window including its border, if any.
        /// If size is an empty string, or if - width is not specified, then the width requested internally by the window will be used initially;
        /// the width may later be adjusted by the movement of sashes in the panedwindow. Size may be any value accepted by Tk_GetPixels.
        auto paneconfigure(const std::derived_from<Widget> auto& tagOrId) -> decltype(_getconfigure({}))
        {
            return this->_getconfigure({ _cpptkinter::AsObj(this->_w), _cpptkinter::AsObj("paneconfigure"), _cpptkinter::AsObj(tagOrId) });
        }
        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&)
        auto paneconfigure(const std::derived_from<Widget> auto& tagOrId, const std::string& cnf) -> decltype(_getconfigure1({}))
        {
            return this->_getconfigure1({ _cpptkinter::AsObj(this->_w), _cpptkinter::AsObj("paneconfigure"), _cpptkinter::AsObj(tagOrId), _cpptkinter::AsObj("-"+ cnf)});
        }
        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&)
        template<cnfs::is_cnf CNF = cnfs::paneconfigure>
        void paneconfigure(CNF&& cnf)
        {
            this->tk->call(this->_w, "paneconfigure", std::forward<CNF>(cnf).tagOrId, this->_options(std::forward<CNF>(cnf), { "tagOrId" }));
        }

        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&)
        auto paneconfig(const std::derived_from<Widget> auto& tagOrId) -> decltype(paneconfigure(tagOrId))
        {
            return this->paneconfigure(tagOrId);
        }
        /// @copydoc paneconfigure(const std::derived_from<Widget> auto&, const std::string&)
        auto paneconfig(const std::derived_from<Widget> auto& tagOrId, const std::string& cnf) -> decltype(paneconfigure(tagOrId, cnf))
        {
            return this->paneconfigure(tagOrId, cnf);
        }
        /// @copydoc paneconfigure(CNF&&)
        template<cnfs::is_cnf CNF = cnfs::paneconfigure>
        void paneconfig(CNF&& cnf)
        {
            this->paneconfigure(std::forward<CNF>(cnf));
        }

        /// @brief Returns an ordered list of the child panes.
        std::vector<_cpptkinter::Tcl_Obj> panes()
        {
            return this->tk->call<std::vector<_cpptkinter::Tcl_Obj>>(this->_w, "panes");
        }
    };
}

cpptkinter::Tk cpptkinter::detail::_get_default_root(const std::string& what)
{
    if (!_support_default_root)
        throw detail::construct_exception<std::runtime_error>("No master specified and tkinter is configured to not support default root");

    if (_default_root.get() == nullptr)
    {
        if (!what.empty())
            throw detail::construct_exception<std::runtime_error>(std::format("Too early to {}: no default root window", what));
        auto root = cpptkinter::Tk();
        if (_default_root != root.pimpl)
            throw detail::construct_exception<std::runtime_error>("?");
        return root;
    }
    else
        return std::shared_ptr(_default_root);
}

void cpptkinter::Misc::destroy()
{
    this->pimpl->destroy();
}

template<typename...Args>
auto cpptkinter::Misc::bind_class(const std::string& className, Args&&...args)
    requires requires { this->_bind({}, std::forward<Args>(args)..., true); }
{
    return this->_root()._bind({ "bind", className }, std::forward<Args>(args)..., true);
}

void cpptkinter::Misc::unbind_class(const std::string& className, const std::string& sequence)
{
    this->_root()._unbind({ "bind", className, sequence });
}

cpptkinter::Misc cpptkinter::Misc::nametowidget(std::string_view name)
{
    auto index = name.find('.');
    std::shared_ptr<impl> w;

    if (index == 0)
    {
        w = this->_root().pimpl;
        name.remove_prefix(1);
        index = name.find('.');
    }
    else
        w = this->pimpl;

    do {
        auto current_name = name.substr(0, index);
        if (current_name.empty())
            break;

        w = w->children.at(std::string(current_name)).pimpl;
        name.remove_prefix(index + 1);
    } while ((index = name.find('.')) != std::string_view::npos);

    return w;
}

cpptkinter::Tk cpptkinter::Misc::_root() const
{
    std::shared_ptr<impl> w = this->pimpl;

    while (w->master.has_value())
        w = w->master.value().pimpl;

    return std::static_pointer_cast<Tk::impl>(w);
}

void cpptkinter::Misc::_report_exception()
{
    auto exc_ptr = std::current_exception();
    auto root = this->_root();
    root.report_callback_exception(root, exc_ptr);
}

cpptkinter::Misc cpptkinter::Wm::wm_iconwindow(this auto&& self)
{
    return self.nametowidget(self.tk->template call<std::string>("wm", "wm_iconwindow", self._w));
}

cpptkinter::Misc cpptkinter::Wm::iconwindow(this auto&& self)
{
    return self.wm_iconwindow();
}

cpptkinter::Misc cpptkinter::Wm::wm_transient(this auto&& self)
{
    return self.nametowidget(self.tk->template call<std::string >("wm", "transient", self._w));
}

cpptkinter::Misc cpptkinter::Wm::transient(this auto&& self)
{
    return self.wm_transient();
}
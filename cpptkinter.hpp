#pragma once
#include "_cpptkinter.hpp"

/// @file cpptkinter.hpp
/// @brief Implements __init__.py.

#define COMMA ,

#define REF_TO_IMPL(member) decltype(impl::member)& member

#define DEFINE_ASSIGNMENT_OPERATOR(cl) \
    cl& operator=(const cl& other) \
    { \
        std::destroy_at(this); \
        return *std::construct_at(this, other); \
    }
// expanded DEFINE_ASSIGNMENT_OPERATOR(cl) macro to make COMMA macro work
#define CNF_CONSTRUCTOR_AND_ASSIGNMENT(cl, cnf_type, str, base) \
    template<cnfs::is_cnf CNF = cnf_type> \
    cl(CNF&& cnf = {}) : cl(std::make_shared<impl>()) \
    {   \
        this->_init_(str, std::forward<CNF>(cnf));  \
    }   \
    cl() : cl(cnf_type{}) { }   \
    cl& operator=(const cl& other) \
    {   \
        std::destroy_at(this);  \
        return *std::construct_at(this, other); \
    }   \
    using base::base


namespace cpptkinter
{
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
        template<typename...Args>
        class set_get_proxy;

        using Anchor = std::string;
        template<typename T> // default is void
        using ButtonCommand = std::variant<std::string, std::function<T()>>;
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

        /// @brief Internal class.
        /// 
        /// Stores function to call when some user defined Tcl function is called e.g. after an event occurred.
        template<typename R, typename...Args>
        struct CallWrapper;

        void _print_command(std::vector<std::string> cmd);

        /// Internal function
        void _tkerror();

        /// Internal function. Calling it will throw std::runtime_error.
        void _exit();

		/// @brief Check if a weak_ptr is empty.
        /// 
		/// @tparam T The type of the weak_ptr.
		/// @param weak The weak_ptr to check.
        template<typename T>
        bool weak_ptr_is_empty(const std::weak_ptr<T>& weak)
        {
            return !weak.owner_before(std::weak_ptr<T>{}) && !std::weak_ptr<T>{}.owner_before(weak);
        }

        Tk _get_default_root(const std::string& what = {});

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
                auto key = rfl::fields<A>()[I].name()/*reflect::member_name<I, A>()*/;
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

        /// @brief Concept for ranges of types satisfying AsObjConcept.
        template<typename R>
        concept range_of_AsObj = std::ranges::range<R> && AsObjConcept<std::ranges::range_value_t<R>>;

        /// @brief Concept for types allowed as indices to e.g. Entry and Listbox.
        template<typename T>
        concept index = AsObjConcept<T> && (std::convertible_to<T, long long> || std::convertible_to<T, std::string>);

        /// @brief set to true to print executed Tcl / Tk commands
        inline bool _debug =
#ifdef NDEBUG
            false;
#else
			true;
#endif
        inline bool _support_default_root = true;
        inline std::shared_ptr<Tk_impl> _default_root = nullptr;
        inline long long _varnum = 0;
        inline long long _checkbutton_count = 0;

        inline size_t tcl_command_name_counter = 0;
        const inline std::set<char> tcl_forbidden_chars{ ' ', '{', '}', '[', ']', '(', ')', '"', '\\', '$', ';', '|', '&', '*', '~', '<', '>', ':' };
    }

    /// @brief Contains structs to be passed to many of cpptkinter's functions.
    ///
    /// Replaces Python's **kwargs.
    namespace cnfs
    {
        template<typename T>
        struct is_cnf_member_trait : std::bool_constant<detail::AsObjConcept<T> || detail::createcommand_concept<T>> { };
        template<typename...Args>
            requires (is_cnf_member_trait<Args>::value && ...)
        struct is_cnf_member_trait<std::variant<Args...>> : std::true_type {};
        template<typename T>
            requires is_cnf_member_trait<T>::value
        struct is_cnf_member_trait<std::optional<T>> : std::true_type {};

		template<typename T>
		concept is_cnf_member = is_cnf_member_trait<T>::value;

        template<typename T, typename IS = std::make_index_sequence<reflect::size<T>()>>
        struct is_cnf_trait : std::false_type { };
        template<typename T, size_t...I>
            requires (!std::is_array_v<std::remove_cvref_t<T>>) && (is_cnf_member<std::remove_cvref_t<reflect::member_type<I, T>>> && ...)
        struct is_cnf_trait<T, std::integer_sequence<size_t, I...>> : std::true_type { };
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

    /// @brief Initialize cpptkinter.
    /// 
    /// This function must be called before doing anything else.
    /// @param argc: the first argument of startup function main().
    /// @param argv: the second argument of startup function main().
    /// @param tcl_library: Path to the Tcl library. Only used if TCL_CORE_LIBRARY_IS_EMBEDDED is false.
    void init(int argc, char* argv[], const std::string& tcl_library = {});

    void mainloop(int n = 0);

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
                data | std::views::stride(2) | std::views::transform(lambda),
                data | std::views::drop(1) | std::views::stride(2)
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
        void deletecommand(const std::string& name);

        /// @brief Call the mainloop of Tk.
        void mainloop(int n = 0);

        /// @brief Quit the Tcl interpreter. All widgets will be destroyed.
        void quit();
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

            auto outer_visitor = [this , &raii, &ignore_fields]<typename T>(T&& value, auto I) {
                auto&& k = reflect::member_name<I, CNF>();
                if (ignore_fields.contains(std::string(k)))
                    return;

                if (k.ends_with('_'))
                    k.remove_suffix(1);

                raii.emplace_back(_cpptkinter::AsObj("-" + std::string(k)));
                raii.emplace_back(this->_options_inner_visitor(std::forward<T>(value)));
            };

            reflect::for_each<CNF>([&outer_visitor, &cnf](auto I) {
                utility::invoke_or_and_then(outer_visitor, reflect::get<I>(std::forward<CNF>(cnf)), I);
                });

            return raii;
        }
    public:
        /// @brief Return the Tkinter instance of a widget identified by its Tcl name NAME.
        Misc nametowidget(std::string_view name);
        /// @brief Return the Tkinter instance of a widget.
        Misc nametowidget(_cpptkinter::tk_window_type window);

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

        void _report_exception();

        std::map<std::string, std::array<std::variant<long long, std::string>, 5>> _getconfigure(std::vector<_cpptkinter::Tcl_Obj>&& raii);
        std::vector<std::string> _getconfigure1(std::vector<_cpptkinter::Tcl_Obj>&& raii);

        auto _configure(const std::vector<std::string>& cmd) -> decltype(_getconfigure({}));
        auto _configure(const std::vector<std::string>& cmd, const std::string& cnf) -> decltype(_getconfigure1({}));
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
        auto configure(T&& v)
			requires requires { this->_configure({ }, std::declval<T>()); }
        {
            return this->_configure({ "configure" }, std::forward<T>(v));
        }
        /// @brief Configure a resource of a widget.
        ///
        /// @param keyword The keyword of the resource.
        /// @param value The new value of the resource.
        template<typename T>
        void configure(const std::string& key, T&& value)
            requires requires { this->_options_inner_visitor(std::declval<T>()); }
        {
            this->tk->call(this->_w, "configure", "-" + key, this->_options_inner_visitor(std::forward<T>(value)));
        }

        /// @copydoc configure(T&&)
        template<typename T>
        auto config(T&& v)
			requires requires { this->configure(std::declval<T>()); }
        {
            return this->configure(std::forward<T>(v));
        }
        /// @copydoc configure(const std::string&, T&&)
        template<typename T>
        void config(const std::string& key, T&& value)
            requires requires { this->configure(key, std::declval<T>()); }
        {
            this->configure(key, std::forward<T>(value));
        }

        template<detail::FromObjConcept R>
        R cget(const std::string& key)
        {
            return this->tk->call<R>(this->_w, "cget", "-" + key);
        }

        detail::set_get_proxy<> operator[](const std::string& key);

        /// @brief Return a list of all resource names of this widget.
        std::vector<std::string> keys();

        /// @brief Return the window path name of this widget.
        operator std::string() const;
		/// @brief Outputs the window path name of this widget to an output stream.
        friend std::ostream& operator<<(std::ostream& os, const Misc& self);

        /// @brief Get the status for propagation of geometry information.
        ///
        /// @returns The current setting.
        bool pack_propagate();
        /// @brief Set the status for propagation of geometry information.
        /// 
        /// @param flag Specifies whether the geometry information of the slaves will determine the size of this widget.
        void pack_propagate(bool flag);

        /// @copydoc pack_propagate()
        bool propagate();
        /// @copydoc pack_propagate(bool)
        void propagate(bool flag);

        /// @brief Return a list of all slaves of this widget in its packing order.
        std::vector<Misc> pack_slaves();
        /// @copydoc pack_slaves
        std::vector<Misc> slaves();

        /// @brief Return a list of all slaves of this widget in its packing order.
        std::vector<Misc> place_slaves();

        /// @brief The anchor value controls how to place the grid within the master when no row / column has any weight.
        ///
        /// The default anchor is nw.
        void grid_anchor(const std::string& anchor = {});
        /// @copydoc grid_anchor
        void anchor(const std::string& anchor = {});

        /// @brief Return a tuple of integer coordinates for the bounding box of this widget controlled by the geometry manager grid.
        ///
        /// If COLUMN, ROW is given the bounding box applies from the cell with row and column 0 to the specified cell.
        /// If COL2 and ROW2 are given the bounding box starts at that cell.
        /// The returned integers specify the offset of the upper left corner in the master widget and the width and height.
        std::array<long long, 4> grid_bbox(const cnfs::grid_bbox& cnf = {});
        /// @copydoc grid_bbox
        std::array<long long, 4> bbox(const cnfs::grid_bbox& cnf = {});

    protected:
        cnfs::grid_column_row_configure_return _grid_configure(const std::string& command, const std::variant<long long, std::string>& index);
    public:
        /// @brief Configure column INDEX of a grid.
        cnfs::grid_column_row_configure_return grid_columnconfigure(const std::variant<long long, std::string>& index);
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
        cnfs::grid_column_row_configure_return columnconfigure(const std::variant<long long, std::string>& index);
        /// @copydoc grid_columnconfigure()
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void columnconfigure(std::variant<size_t, std::vector<size_t>, std::string> index, CNF&& cnf)
        {
            return this->grid_columnconfigure(index, std::forward<CNF>(cnf));
        }

        /// @brief Return a tuple of column and row which identify the cell at which the pixel at position X and Y inside the master widget is located.
        std::array<long long, 2> grid_location(const detail::ScreenUnits& x, const detail::ScreenUnits& y);
        /// @copydoc grid_location
        std::array<long long, 2> location(const detail::ScreenUnits& x, const detail::ScreenUnits& y);

        /// @brief Get the status for propagation of geometry information.
        ///
        /// A boolean argument specifies whether the geometry information of the slaves will determine the size of this widget.
        bool grid_propagate();
        /// @brief Set the status for propagation of geometry information.
        ///
        /// A boolean argument specifies whether the geometry information of the slaves will determine the size of this widget.
        void grid_propagate(bool flag);

        /// @brief Configure row INDEX of a grid.
        cnfs::grid_column_row_configure_return grid_rowconfigure(const std::variant<long long, std::string>& index);
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
        cnfs::grid_column_row_configure_return rowconfigure(const std::variant<long long, std::string>& index);
        /// @copydoc grid_rowconfigure()
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void rowconfigure(const std::variant<size_t, std::vector<size_t>, std::string>& index, CNF&& cnf)
        {
            return this->grid_rowconfigure(index, std::forward<CNF>(cnf));
        }

        /// @brief Return a tuple of the number of column and rows in the grid.
        std::array<long long, 2> grid_size();
		/// @copydoc grid_size
        std::array<long long, 2> size();

        /// @brief Return a list of all slaves of this widget in its packing order.
        std::vector<Misc> grid_slaves(std::optional<long long> row = {}, std::optional<long long> column = {});
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

    Misc Wm::wm_iconwindow(this auto&& self)
    {
        return self.nametowidget(self.tk->template call<std::string>("wm", "wm_iconwindow", self._w));
    }
    Misc Wm::iconwindow(this auto&& self)
    {
        return self.wm_iconwindow();
    }
    Misc Wm::wm_transient(this auto&& self)
    {
        return self.nametowidget(self.tk->template call<std::string >("wm", "transient", self._w));
    }
    Misc Wm::transient(this auto&& self)
    {
        return self.wm_transient();
    }

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
        virtual void destroy();
    };

    template<typename R, typename...Args>
    struct detail::CallWrapper
    {
        std::function<R(Args...)> func;
        //utility::weak<Misc> widget;
        Misc widget;

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

    template<typename...Args>
    class detail::set_get_proxy
    {
        struct from_tk
        {
            Misc misc;
            std::string keyword;
        };
    public:
        std::variant<from_tk, std::reference_wrapper<Args>...> data;

		set_get_proxy(const Misc& misc, const std::string& keyword) : data(std::in_place_type<from_tk>, misc, keyword)
        {

        }
        template<hhh::meta::variant_converting_constructor_constraint<std::variant<Args...>> T>
        set_get_proxy(T& value) :
            data(std::in_place_type<std::reference_wrapper<hhh::meta::variant_converting_constructor_result_t<T, std::variant<Args...>>>> , value)
        {

        }

        template<typename T>
			requires requires (Misc m) { m.configure(std::string(), std::declval<T>()); } || hhh::meta::variant_converting_constructor_constraint<T, std::variant<Args...>>
        void operator=(T&& value)
        {
            auto visitor = [&]<typename T2>(T2& e) {
                if constexpr (std::same_as<T2, from_tk>)
                    e.misc.configure(e.keyword, std::forward<T>(value));
                else if constexpr (requires { e.get() = std::forward<T>(value); })
                    e.get() = std::forward<T>(value);
				else
                    throw construct_exception<std::invalid_argument>(std::format("passed type {} but expected {}", typeid(T).name()/*reflect::type_name<T>()*/, reflect::type_name<T2>()));
            };

            return std::visit(visitor, this->data);
        }

        template<typename R>
         requires detail::FromObjConcept<R> || hhh::meta::contains<R, Args...>
        R get()
        {
			auto visitor = []<typename T>(T& e) -> R {
				if constexpr (std::same_as<T, from_tk>)
                {
                    if constexpr (detail::FromObjConcept<R>)
                        return e.misc.cget<R>(e.keyword);
                }
                else if constexpr (std::same_as<T, std::reference_wrapper<R>>)
                {
                    return e.get();
                }

                throw construct_exception<std::invalid_argument>(std::format("requested type {} but got {}", reflect::type_name<R>(), reflect::type_name<T>()));
            };

            return std::visit(visitor, this->data);
        }
        template<typename R>
            requires detail::FromObjConcept<R> || hhh::meta::contains<R, Args...>
        operator R()
        {
            return this->get<R>();
        }
    };

    struct detail::Tk_impl : Misc::impl
    {
        bool _tkloaded = false;

        /// @brief Destroy this and all descendants widgets.
        /// 
        /// This will end the application of this Tcl interpreter.
        void destroy() override;
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

        void _init_(const std::string& screenName, const std::string& baseName, const std::string& className, bool useTk, bool sync, const std::string& use);

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
        Tk(const std::string& screenName = {}, const std::string& baseName = {}, const std::string& className = "Tk", bool useTk = true, bool sync = false, const std::string& use = {});

        void loadtk();
    private:
        void _loadtk();
    public:
        /// @brief This function prints the exception to stderr.
        ///
        /// It is the default callback registered in @ref report_callback_exception.
        static void default_report_callback_exception(Tk&, const std::exception_ptr& exc_ptr);

        /// @brief Internal function.
        /// 
        /// It reads .BASENAME.tcl and .CLASSNAME.tcl into the Tcl Interpreter.
        void readprofile(const std::string& baseName, const std::string& className);

        /// @brief Report callback exception on stderr.
        ///
        /// Applications may want to override this internal function. Default value is @ref _report_callback_exception-
        std::function<void(Tk&, const std::exception_ptr&)> report_callback_exception = &Tk::default_report_callback_exception;

        /// @brief Not implementable until c++26 when (hopefully) reflection will allow for code gen.
        void __getattr__() = delete;
    };

    Tk Tcl(const std::string& screenName = {}, const std::string& baseName = {}, const std::string& className = "Tk", bool useTk = true);

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

            impl(const Tk& root);
            /// @brief Unset the variable in Tcl.
            virtual ~impl();
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
        operator std::string() const;

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
        std::vector<std::tuple<std::vector<std::string>, std::string>> trace_info();

        template<typename Self, std::derived_from<Variable> Other>
        bool operator==(this const Self& self, const Other& other)
        {
            return self->_name == other._name && self->_tk == other._tk;
        }
    };
    namespace detail
    {
        template<typename T>
			requires detail::AsObjConcept<T> && detail::FromObjConcept<T> && std::default_initializable<T>
        struct TypedVariable : Variable
        {
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
    }
    /// @brief Value holder for string variables.
    using StringVar = detail::TypedVariable<std::string>;
    /// @brief Value holder for integer variables.
    using IntVar = detail::TypedVariable<long long>;
    /// @brief Value holder for float variables.
    using DoubleVar = detail::TypedVariable<double>;
    /// @brief Value holder for boolean variables.
    using BooleanVar = detail::TypedVariable<bool>;

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
                vec | std::views::stride(2),
                vec | std::views::drop(1) | std::views::stride(2)
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
            void destroy() override;
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

        template<std::derived_from<impl> I>
        BaseWidget(const std::shared_ptr<I>& pimpl) :
            Misc(pimpl),
            widgetName(pimpl->widgetName),
            _name(pimpl->_name)
        {

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
    };

    /// @brief Internal class.
    /// 
    /// Base class for a widget which can be positioned with the geometry managers Pack, Place or Grid.
    struct Widget : BaseWidget, Pack, Grid, Place
    {
		using BaseWidget::BaseWidget;
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
        void tk_popup(long long x, long long y);
        /// @brief Post the menu at position X,Y with entry ENTRY.
        void tk_popup(long long x, long long y, long long entry);

        /// @brief Activate entry at INDEX.
        void activate(const std::variant<long long, std::string>& index);

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
        void insert(long long index, const std::string& itemType, CNF&& cnf)
        {
            this->tk->call(this->_w, "insert", index, itemType, this->_options(std::forward<CNF>(cnf)));
        }

        /// @brief Add hierarchical menu item at INDEX.
        template<cnfs::is_cnf CNF = cnfs::add_cascade>
        void insert_cascade(long long index, CNF&& cnf = {})
        {
			this->insert(index, "cascade", std::forward<CNF>(cnf));
        }

        /// @brief Add checkbutton menu item at INDEX.
		template<cnfs::is_cnf CNF = cnfs::add_checkbutton<bool>>
        void insert_checkbutton(long long index, CNF&& cnf = {})
        {
            this->insert(index, "checkbutton", std::forward<CNF>(cnf));
        }

        /// @brief Add command menu item at INDEX.
		template<cnfs::is_cnf CNF = cnfs::add_command>
        void insert_command(long long index, CNF&& cnf = {})
        {
            this->insert(index, "command", std::forward<CNF>(cnf));
        }

        /// @brief Add radio menu item at INDEX.
		template<cnfs::is_cnf CNF = cnfs::add_radiobutton<int>>
        void insert_radiobutton(long long index, CNF&& cnf = {})
        {
            this->insert(index, "radiobutton", std::forward<CNF>(cnf));
        }

        /// @brief Add separator at INDEX.
		template<cnfs::is_cnf CNF = cnfs::add_separator>
        void insert_separator(long long index, CNF&& cnf = {})
		{
            this->insert(index, "separator", std::forward<CNF>(cnf));
		}

        /// @brief Delete menu items at INDEX.
        void delete_(long long index);
        /// @brief Delete menu items between INDEX1 and INDEX2 (included).
        void delete_(long long index1, long long index2);

        /// @brief Return the resource value of a menu item for OPTION at INDEX.
        template<detail::FromObjConcept R>
        R entrycget(long long index, const std::string& option)
        {
			return this->tk->call<R>(this->_w, "entrycget", index, "-" + option);
        }

        /// @brief Configure a menu item at INDEX.
        template<cnfs::is_cnf CNF>
        auto entryconfigure(long long index, CNF&& cnf)
        {
            return this->_configure({ "entryconfigure", std::to_string(index) }, std::forward<CNF>(cnf));
        }
        /// @brief Configure a menu item at INDEX.
        auto entryconfigure(long long index) -> decltype(this->_configure({}));

		/// @copydoc entryconfigure(long long, CNF&&)
        template<cnfs::is_cnf CNF>
        auto entryconfig(long long index, CNF&& cnf)
        {
			return this->entryconfigure(index, std::forward<CNF>(cnf));
        }
		/// @copydoc entryconfigure(long long)
        auto entryconfig(long long index) -> decltype(this->entryconfigure(index));

        /// @brief Return the index of a menu item identified by INDEX.
        long long index(long long index);

        /// @brief Invoke a menu item identified by INDEX and execute the associated command.
        template<detail::FromObjConcept R = void>
        R invoke(long long index)
        {
			return this->tk->call<R>(this->_w, "invoke", index);
        }

        /// @brief Display a menu at position X,Y.
        void post(long long x, long long y);

        /// @brief Return the type of the menu item at INDEX.
        std::string type(long long index);

        /// @brief Unmap a menu.
        void unpost();

        /// @brief Return the x-position of the leftmost pixel of the menu item at INDEX.
        long long xposition(long long index);

        /// @brief "Return the y-position of the topmost pixel of the menu item at INDEX.
        long long yposition(long long index);
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
        template<typename R>
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
            opt<detail::ButtonCommand<R>> command;
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
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Button, cnfs::Button<void>, "button", Widget);

        /// @brief Flash the button.
        /// 
        /// This is accomplished by redisplaying the button several times, alternating between active and normal colors.
        /// At the end of the flash the button is left in the same normal/active state as when the command was invoked.
        /// This command is ignored if the button's state is disabled.
        void flash();

        /// @brief Invoke the command associated with the button.
        template<detail::FromObjConcept R = void>
        R invoke()
        {
            return this->tk->call<R>(this->_w, "invoke");
        }
    };

    /// @brief A button widget with defined callback return type.
    ///
	/// Button takes a callback with arbitrary return type. TypedButton restricts the return type to a specific type.
    /// @tparam R The return type of the callback.
    /// @see Button
    template<typename R>
        requires detail::FromObjConcept<R> && detail::PythonCmd_ClientDataReturnConcept<R>
    struct TypedButton : Button
    {
        /// @brief Construct a new TypedButton widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(TypedButton, cnfs::Button<R>, "button", Button);

        /// @copydoc Button::invoke
        R invoke()
        {
            return this->Button::template invoke<R>();
        }
    };

    /// @brief %Canvas widget to display graphical elements like lines or text.
    struct Canvas;

    namespace cnfs
    {
        /// @brief Argument for Checkbutton::Checkbutton() and TypedCheckbutton::TypedCheckbutton().
        template<typename T, typename R>
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
			opt<detail::ButtonCommand<R>> command;
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
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Checkbutton, cnfs::Checkbutton<long long COMMA void>, "checkbutton", Widget);

        /// @brief Put the button in off-state.
        void deselect();

        /// @brief Flash the button.
        void flash();

        /// @brief Toggle the button and invoke a command if given as resource.
        template<detail::FromObjConcept R = void>
        R invoke()
        {
            return this->tk->call<R>(this->_w, "invoke");
        }

        /// @brief Put the button in on-state.
        void select();

        /// @brief Toggle the button.
        void toggle();
    };

    /// @brief A checkbutton widget with defined value and callback return types.
    ///
    /// Checkbutton has arbitrary value and callback return types. TypedCheckbutton restricts the value and callback type to a specific type.
    /// @tparam T The value type.
    /// @tparam R The return type of the callback.
    /// @see Checkbutton
    template<detail::AsObjConcept T, typename R>
        requires detail::FromObjConcept<R>&& detail::PythonCmd_ClientDataReturnConcept<R>
    struct TypedCheckbutton : Checkbutton
    {
        /// @brief Construct a new TypedCheckbutton widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(TypedCheckbutton, cnfs::Checkbutton<T COMMA R>, "radiobutton", Checkbutton);

        /// @copydoc Checkbutton::invoke
        R invoke()
        {
            return this->Checkbutton::template invoke<R>();
        }
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
			opt<detail::XYScrollCommand> xscrollcommand;
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
            this->tk->call(this->_w, "delete", index);
        }
        /// @brief Delete text from FIRST to LAST (not included).
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", first, last);
        }

        /// @brief Return the text.
        std::string get();

        /// @brief Insert cursor at INDEX.
        void icursor(const detail::index auto& index)
        {
            this->tk->call(this->_w, "icursor", index);
        }

        /// @brief Return position of cursor.
        long long index(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "index", index);
        }

        /// @brief Insert STRING at INDEX.
        void insert(const detail::index auto& index, const std::string& string)
        {
            this->tk->call(this->_w, "insert", index, string);
        }

        /// @brief unknown
        void scan_mark(long long x);

        /// @brief unknown
        void scan_dragto(long long x);

        /// @brief Adjust the end of the selection near the cursor to INDEX.
        void selection_adjust(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "adjust", index);
        }

		/// @copydoc selection_adjust
        void select_adjust(const detail::index auto& index)
        {
            this->selection_adjust(index);
        }

        /// @brief Clear the selection if it is in this widget.
        void selection_clear();

		/// @copydoc selection_clear
        void select_clear();

        /// @brief Set the fixed end of a selection to INDEX.
        void selection_from(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "from", index);
        }

		/// @copydoc selection_from
        void select_from(const detail::index auto& index)
        {
            this->selection_from(index);
        }

        /// @brief Return true if there are characters selected in the entry, false otherwise.
        bool selection_present();

		/// @copydoc selection_present
        bool select_present();

        /// @brief Set the selection from START to END (not included).
        void selection_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->tk->call(this->_w, "selection", "range", start, end);
        }

		/// @copydoc selection_range
        void select_range(const detail::index auto& start, const detail::index auto& end)
        {
            this->selection_range(start, end);
        }

        /// @brief Set the variable end of a selection to INDEX.
        void selection_to(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "to", index);
        }

		/// @copydoc selection_to
        void select_to(const detail::index auto& index)
        {
            this->selection_to(index);
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
            opt<detail::XYScrollCommand> xscrollincrement;
            opt<detail::XYScrollCommand> yscrollincrement;
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
            this->tk->call(this->_w, "activate", index);
        }

        /// @brief Return a tuple of X1,Y1,X2,Y2 coordinates for a rectangle which encloses the item identified by the given index.
        std::array<long long, 4> bbox(const detail::index auto& index)
        {
            return this->tk->call<std::array<long long, 4>>(this->_w, "bbox", index);
        }

        /// @brief Return the indices of currently selected item.
        std::vector<long long> curselection();

        /// @brief Delete item at index.
        void delete_(const detail::index auto& index)
        {
            this->tk->call(this->_w, "delete", index);
        }
        /// @brief Delete items from FIRST to LAST (included).
        void delete_(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "delete", first, last);
        }

        /// @brief Get the item at index.
        template<detail::FromObjConcept R>
        R get(const detail::index auto& index)
        {
			if constexpr (std::same_as<R, std::string>)
				return this->tk->call<R>(this->_w, "get", index);
			else
            {
                auto res = this->tk->call<std::variant<R, std::string>>(this->_w, "get", index);
				if (std::holds_alternative<R>(res))
					return std::get<R>(res);
				else
                {
                    if (std::get<std::string>(res).empty())
                        throw detail::construct_exception<std::invalid_argument>(std::format("index {} was out of bounds", index));
                    else
                        throw detail::construct_exception<std::invalid_argument>(std::format("expected type {} but got std::string", reflect::type_name<R>()));
                }
            }
        }
        /// @brief Get list of items from FIRST to LAST (included).
        template<detail::FromObjConcept R>
        std::vector<R> get(const detail::index auto& first, const detail::index auto& last)
        {
            return this->tk->call<std::vector<R>>(this->_w, "get", first, last);
        }

        /// @brief Return index of item identified with INDEX.
        long long index(const detail::index auto& index)
        {
            return this->tk->call<long long>(this->_w, "index", index);
        }

        /// @brief Insert ELEMENTS at INDEX.
        void insert(const detail::index auto& index, const detail::AsObjConcept auto&...elements)
        {
			this->tk->call(this->_w, "insert", index, elements...);
        }
        /// @copydoc insert(const detail::index auto&, const detail::AsObjConcept auto&...)
        void insert(const detail::index auto& index, detail::range_of_AsObj auto&& elements)
        {
            this->tk->call(this->_w, "insert", index, std::forward<decltype(elements)>(elements) | std::views::transform([](auto& val) { return _cpptkinter::AsObj(val); }));
        }

        /// @brief Get index of item which is nearest to y coordinate Y.
        long long nearest(long long y);

        /// @brief unknown
        void scan_mark(long long x, long long y);

        /// @brief unknown
        void scan_dragto(long long x, long long y);

        /// @brief Scroll such that INDEX is visible.
        void see(const detail::index auto& index)
        {
            this->tk->call(this->_w, "see", index);
        }

        /// @brief Set the fixed end oft the selection to INDEX.
        void selection_anchor(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "anchor", index);
        }

		/// @copydoc selection_anchor
        void select_anchor(const detail::index auto& index)
        {
            this->selection_anchor(index);
        }

        /// @brief Clear the selection at index.
        void selection_clear(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "clear", index);
        }
        /// @brief Clear the selection from FIRST to LAST (included).
        void selection_clear(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "selection", "clear", first, last);
        }

		/// @copydoc selection_clear(const detail::index auto&)
        void select_clear(const detail::index auto& index)
        {
            this->selection_clear(index);
        }
		/// @copydoc selection_clear(const detail::index auto&, const detail::index auto&)
        void select_clear(const detail::index auto& first, const detail::index auto& last)
        {
            this->selection_clear(first, last);
        }

        /// @brief Return True if INDEX is part of the selection.
        bool selection_includes(const detail::index auto& index)
        {
            return this->tk->call<bool>(this->_w, "selection", "includes", index);
        }

		/// @copydoc selection_includes
        bool select_includes(const detail::index auto& index)
        {
            return this->selection_includes(index);
        }

        /// @brief Set the selection for index without changing the currently selected elements.
        void selection_set(const detail::index auto& index)
        {
            this->tk->call(this->_w, "selection", "set", index);
        }
        /// @brief Set the selection from FIRST to LAST (included) without changing the currently selected elements.
        void selection_set(const detail::index auto& first, const detail::index auto& last)
        {
            this->tk->call(this->_w, "selection", "set", first, last);
        }

		/// @copydoc selection_set(const detail::index auto&)
        void select_set(const detail::index auto& index)
        {
            this->selection_set(index);
        }
		/// @copydoc selection_set(const detail::index auto&, const detail::index auto&)
        void select_set(const detail::index auto& first, const detail::index auto& last)
        {
            this->selection_set(first, last);
        }

        /// @brief Return the number of elements in the listbox.
        long long size();

        /// @brief Return the resource value for an ITEM and an OPTION.
        template<detail::FromObjConcept R>
        void itemcget(const detail::index auto& index, const std::string& option)
        {
			return this->tk->call<R>(this->_w, "itemcget", index, "-" + option);
        }

        /// @brief Get allowed keywords.
        std::map<std::string, std::array<std::string, 5>> itemconfigure(const detail::index auto& index)
        {
            auto map = this->_configure({ "itemconfigure", std::to_string(index) });

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
            this->_configure({ "itemconfigure", std::to_string(index) }, std::forward<CNF>(cnf));
        }

        /// @copydoc itemconfigure(const detail::index auto&)
        std::map<std::string, std::array<std::string, 5>> itemconfig(const detail::index auto& index)
        {
            return this->itemconfigure(index);
        }
		/// @copydoc itemconfigure(const detail::index auto&, CNF&&)
		template<cnfs::is_cnf CNF = cnfs::Listbox_itemconfigure>
        void itemconfig(const detail::index auto& index, CNF&& cnf = {})
        {
            this->itemconfigure(index, std::forward<CNF>(cnf));
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
        template<typename T, typename R>
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
            opt<detail::ButtonCommand<R>> command;
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
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Radiobutton, cnfs::Radiobutton<long long COMMA void>, "radiobutton", Widget);

		/// @brief Put the button in off-state.
        void deselect();

        /// @brief Flash the button.
        void flash();

        /// @brief Toggle the button and invoke a command if given as resource.
        template<detail::FromObjConcept R = void>
        R invoke()
        {
            return this->tk->call<R>(this->_w, "invoke");
        }

        /// @brief Put the button in on-state.
        void select();
    };

    /// @brief A radiobutton widget with defined value and callback return types.
    ///
    /// Radiobutton has arbitrary value and callback return types. TypedRadiobutton restricts the value and callback type to a specific type.
    /// @tparam T The value type.
    /// @tparam R The return type of the callback.
    /// @see Radiobutton
    template<detail::AsObjConcept T, typename R>
        requires detail::FromObjConcept<R> && detail::PythonCmd_ClientDataReturnConcept<R>
    struct TypedRadiobutton : Button
    {
        /// @brief Construct a new TypedRadiobutton widget.
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(TypedRadiobutton, cnfs::Radiobutton<T COMMA R>, "radiobutton", Button);

        /// @copydoc Radiobutton::invoke
        R invoke()
        {
            return this->Radiobutton::template invoke<R>();
        }
    };

    namespace cnfs
    {
        /// @brief Argument for Scale::Scale().
        template<typename T>
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
            opt<std::variant<std::string, std::function<T(std::string)>>> command;
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
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(Scale, cnfs::Scale<void>, "scale", Widget);

        /// @brief Get the current value as integer or float.
        double get();

		/// @brief Set the current value.
        double set(double value);

        /// @brief Return a tuple (X,Y) of the point along the centerline of the trough that corresponds to the current value.
        std::array<long long, 2> coords();
        /// @brief Return a tuple (X,Y) of the point along the centerline of the trough that corresponds to VALUE.
        std::array<long long, 2> coords(double value);

        /// @brief Return where the point X,Y lies. Valid return values are "slider", "though1" and "though2".
        std::string identify(detail::ScreenUnits x, detail::ScreenUnits y);
    };

    /// @brief %Scrollbar widget which displays a slider at a certain position.
    struct Scrollbar;

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

            void operator()();
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
			void destroy() override;
        };

    private:
		REF_TO_IMPL(_menu);
    public:
		REF_TO_IMPL(menuname);

    protected:
        void _init_(const Misc& master, const StringVar& variable, detail::sized_range_of_string auto&& values, std::function<void(const StringVar&)>&& command)
        {
            if (values.size() == 0)
				throw detail::construct_exception<std::invalid_argument>("values must be non-empty");

            this->Menubutton::_init_("menubutton",
                cnfs::Menubutton{ .master = master, .anchor = "c", .borderwidth = 2, .highlightthickness = 2, .indicatoron = 1, .relief = constants::RAISED, .textvariable = variable });

            this->widgetName = "tk_optionMenu";
            auto&& menu = this->_menu.emplace(cnfs::Menu{ .master = *this, .name = "menu", .tearoff = 0 });
            this->menuname = menu._w;

            for (auto&& v : values)
                menu.add_command({ .command = detail::_setit(variable, v, std::move(command)), .label = v });

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
        OptionMenu(const Misc& master, const StringVar& variable, detail::sized_range_of_string auto&& values, std::function<void(const StringVar&)>&& command = {}) :
            OptionMenu(std::make_shared<impl>())
        {
            this->_init_(master, variable, std::forward<decltype(values)>(values), std::move(command));
        }

        detail::set_get_proxy<std::optional<cpptkinter::Menu>> operator[](const std::string& name);
    };

    /// @brief %Base class for images.
    struct Image;

    /// @brief %Widget which can display images in PGM, PPM, GIF, PNG format.
    struct PhotoImage;

    /// @brief %Widget which can display images in XBM format.
    struct BitmapImage;

    /// @brief %Spinbox widget.
    struct Spinbox;

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
        /// @brief Construct a labelframe widget
        CNF_CONSTRUCTOR_AND_ASSIGNMENT(LabelFrame, cnfs::LabelFrame, "labelframe", Widget);
    };

    /// @brief %Panedwindow widget.
    struct PanedWindow;
}
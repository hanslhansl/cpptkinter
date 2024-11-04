#pragma once
#include "_tkinter.hpp"

/// @file __init__.hpp
/// @brief Implements __init__.py.

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
        struct set_get_proxy;

        using _Anchor = std::string;
        template<typename T> // default is void
        using _ButtonCommand = std::variant<std::string, std::function<T()>>;
        using _Compound = std::string;
        using _Cursor = std::variant<std::string,
            std::tuple<std::string>,
            std::tuple<std::string, std::string>,
            std::tuple<std::string, std::string, std::string>,
            std::tuple<std::string, std::string, std::string, std::string>>;
        using _EntryValidateCommand = std::variant<std::string, std::vector<std::string>, std::function<bool()>>;
        using _ImageSpec = std::variant<std::string/*, _Image*/>;
        DEVIATING_IMPLEMENTATION_WARNING("_ImageSpec not done");
        using _Relief = std::string;
        using _ScreenUnits = std::variant<long long, double, std::string>;
        using _XYScrollCommand = std::variant<std::string, std::function<void(double, double)>>;
        using _TakeFocusValue = std::variant<bool, std::function<bool(std::string)>>;
        using _FontDescription = std::variant<std::string, /*Font, */
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

        template<typename V, typename Conv = const std::nullopt_t&>
        auto _splitdict(std::map<std::string, V>&& v, bool cut_minus = true, Conv&& conv = std::nullopt)
        {
            std::map<std::string, std::conditional_t<std::same_as<Conv, bool>, V, std::invoke_result_t<Conv, std::string, V&&>>> result{};
            while (!v.empty())
            {
				auto&& node = v.extract(v.begin());
				auto&& key = node.key();
				auto&& mapped = node.mapped();

				if (cut_minus && key.starts_with('-'))
					key = key.substr(1);

				if constexpr (!std::same_as<Conv, const std::nullopt_t&>)
                {
					auto&& value = conv(key, std::move(mapped));
                    result.emplace(std::move(key), std::move(value));
                }
                else
                    result.insert(std::move(node));
            }
            return result;
        }
        template<typename A, typename V, typename Conv = const std::nullopt_t&>
        A _splitdict_to_aggregate(std::map<std::string, V>&& v, bool cut_minus = true, Conv&& conv = std::nullopt)
        {
            if (v.size() != reflect::size<A>())
                throw detail::construct_exception<std::invalid_argument>(std::format("map has {} elements but '{}' has {} members", v.size(), reflect::type_name<A>(), reflect::size<A>()));

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

        /// @brief set to true to print executed Tcl / Tk commands
        inline bool _debug = false;
        inline bool _support_default_root = true;
        inline std::shared_ptr<Tk_impl> _default_root = nullptr;
        inline long long _varnum = 0;

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
        template<typename T>
            requires hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::variant>&& hhh::meta::apply_conjunction<std::remove_cvref_t<T>, is_cnf_member_trait>::value
        struct is_cnf_member_trait<T> : std::true_type { };
        template<typename T>
            requires hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::optional>&& is_cnf_member_trait<decltype(std::declval<T>().value())>::value
        struct is_cnf_member_trait<T> : std::true_type { };
		template<typename T>
		concept is_cnf_member = is_cnf_member_trait<T>::value;

        template<typename T, typename IS = std::make_index_sequence<reflect::size<T>()>>
        struct is_cnf_trait : std::false_type { };
        template<typename T, size_t...I>
            requires (!std::is_array_v<std::remove_cvref_t<T>>) && (is_cnf_member<decltype(reflect::get<I>(std::declval<T>()))> && ...)
        struct is_cnf_trait<T, std::integer_sequence<size_t, I...>> : std::true_type { };
		/// @brief Satsified if T is a cnf struct.
        /// 
        /// Usually cnf structs are passed to cpptkinter::Misc::_options() internally.
        template<typename T>
        concept is_cnf = is_cnf_trait<T>::value;

        using pad_type = utility::extend_variants<detail::_ScreenUnits, std::array<detail::_ScreenUnits, 2>>::type;

        template<typename T>
        using opt = std::optional<T>;
        using opt_string = opt<std::string>;
		using opt_bool = opt<bool>;
        using opt_screenunits = opt<detail::_ScreenUnits>;
        using opt_pad_type = opt<pad_type>;

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

    class Wm
    {
    public:
        /// @brief Set the name of the icon for this widget. Return the name if None is given.
        std::string wm_iconname(this auto&& self, const std::string& newName = {})
        {
            return self.tk->template call<std::string>("wm", "iconname", self._w, newName);
        }
        /// @see wm_iconname
        std::string iconname(this auto&& self, const std::string& newName = {})
        {
            return self.wm_iconname(newName);
        }

        /// @brief Set the title of this widget.
        std::string wm_title(this auto&& self, const std::string& string = {})
        {
            return self.tk->template call<std::string>("wm", "title", self._w, string);
        }
        /// @see wm_title
        std::string title(this auto&& self, const std::string& string = {})
        {
            return self.wm_title(string);
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
        /// @see wm_protocol
        template<detail::FromObjConcept R = void, typename Func>
            requires detail::createcommand_concept<Func> || detail::AsObjConcept<std::remove_cvref_t<Func>>
        R protocol(this auto && self, const std::string & name, Func && func)
        {
            return self.template wm_protocol<R>(name, std::forward<Func>(func));
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

		Misc(const std::shared_ptr<impl>& pimpl);
    public:
        Misc& operator=(const Misc& other);

		/// @brief Calls this->pimpl->destroy().
        void destroy();

        /// Call the mainloop of Tk.
        void mainloop(int n = 0);
    protected:
        template<typename T>
            requires (cnfs::is_cnf_member<T> && !hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::optional>)
        Tcl_Obj* _options_inner_visitor(T&& value)
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
        detail::Tcl_Obj_vector_raii _options(CNF&& cnf, std::set<std::string> ignore_fields = {})
        {
            detail::Tcl_Obj_vector_raii raii{};

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

        Tk _root();

        void _report_exception();

        std::vector<std::vector<std::string>> _getconfigure(detail::Tcl_Obj_vector_raii&& raii);
        std::vector<std::string> _getconfigure1(detail::Tcl_Obj_vector_raii&& raii);

        std::vector<std::vector<std::string>> _configure(const std::vector<std::string>& cmd);
        std::vector<std::string> _configure(const std::vector<std::string>& cmd, const std::string& cnf);
        template<cnfs::is_cnf CNF>
        void _configure(const std::vector<std::string>& cmd, CNF&& cnf)
        {
            detail::Tcl_Obj_vector_raii raii{ };
            for (auto& c : cmd)
                raii.emplace_back(_cpptkinter::AsObj(c));

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
            detail::Tcl_Obj_vector_raii raii{ };
			raii.emplace_back(this->_options_inner_visitor(std::forward<T>(value)));
            this->tk->call(this->_w, "configure", "-" + key, std::move(raii));
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

        detail::set_get_proxy operator[](const std::string& key);

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
        std::array<long long, 2> grid_location(const detail::_ScreenUnits& x, const detail::_ScreenUnits& y);
        /// @copydoc grid_location
        std::array<long long, 2> location(const detail::_ScreenUnits& x, const detail::_ScreenUnits& y);

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
        void grid_rowconfigure(std::variant<size_t, std::vector<size_t>, std::string> index, CNF&& cnf)
        {
            this->tk->call("grid", "rowconfigure", this->_w, index, this->_options(std::forward<CNF>(cnf)));
        }
		/// @copydoc grid_rowconfigure(const std::variant<long long, std::string>&)
        cnfs::grid_column_row_configure_return rowconfigure(const std::variant<long long, std::string>& index);
        /// @copydoc grid_rowconfigure()
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void rowconfigure(std::variant<size_t, std::vector<size_t>, std::string> index, CNF&& cnf)
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

    struct Misc::impl : public hhh::misc::extended_enable_shared_from_this
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

    struct detail::set_get_proxy
    {
        Misc misc;
        std::string keyword;

        template<typename T>
			requires requires (Misc m) { m.configure(std::string(), std::declval<T>()); }
        void operator=(T&& value)
        {
            this->misc.configure(this->keyword, std::forward<T>(value));
        }
        template<detail::FromObjConcept R>
        R get()
        {
            return this->misc.cget<R>(this->keyword);
        }
        template<detail::FromObjConcept R>
        operator R()
        {
            return this->get<R>();
        }
    };

    namespace  cnfs
    {
        using opt_master = opt<Misc>;

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
        struct _PackInfo
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

		/// @brief Argument for Place::place_configure().
        struct place_configure
        {
            /// NSEW (or subset) - position anchor according to given direction
            opt<detail::_Anchor> anchor;
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
        struct _PlaceInfo
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
        struct _GridInfo
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

        /// @brief Argument for cpptkinter::Toplevel().
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
            opt<detail::_Cursor> cursor;
            opt_screenunits height;
            opt_string highlightbackground;
            opt_string highlightcolor;
            opt_screenunits highlightthickness;
            //opt<Menu> menu;
            opt_string name;
            opt_screenunits padx;
            opt_screenunits pady;
            opt<detail::_Relief> relief;
            opt_string screen;
            opt<detail::_TakeFocusValue> takefocus;
            opt<size_t> use;
            opt<std::variant<std::string, std::tuple<std::string, long long>>> visual;
            opt_screenunits width;
        };

        /// @brief Argument for cpptkinter::Button() and cpptkinter::TypedButton().
        template<typename T>    // default is void
        struct Button
        {
            opt_master master;
            opt_string activebackground;
            opt_string activeforeground;
            opt<detail::_Anchor> anchor;
            opt_string background;
            opt_screenunits bd;
            opt_string bg;
            opt_string bitmap;
            opt_screenunits border;
            opt_screenunits borderwidth;
            opt<detail::_ButtonCommand<T>> command;
            opt<detail::_Compound> compound;
            opt<detail::_Cursor> cursor;
            opt_string default_;
            opt_string disabledforeground;
            opt_string fg;
            opt<detail::_FontDescription> font;
            opt_string foreground;
            opt_screenunits height;
            opt_string highlightbackground;
            opt_string highlightcolor;
            opt_screenunits highlightthickness;
            opt<detail::_ImageSpec> image;
            opt_string justify;
            opt_string name;
            opt<detail::_Relief> overrelief;
            opt_screenunits padx;
            opt_screenunits pady;
            opt<detail::_Relief> relief;
            opt<size_t> repeatdelay;
            opt<size_t> repeatinterval;
            opt_string state;
            opt<detail::_TakeFocusValue> takefocus;
            opt<std::variant<double, std::string>> text;
            opt_string textvariable;
            opt<size_t> underline;
            opt_screenunits width;
            opt_screenunits wraplength;
        };

		/// @brief Argument for cpptkinter::Frame().
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
            opt<detail::_Cursor> cursor;
            opt_screenunits height;
            opt_string highlightbackground;
            opt_string highlightcolor;
            opt_screenunits highlightthickness;
            opt_string name;
            opt_screenunits padx;
            opt_screenunits pady;
            opt<detail::_Relief> relief;
            opt<detail::_TakeFocusValue> takefocus;
            opt<std::variant<std::string, std::tuple<std::string, long long>>> visual;
            opt_screenunits width;
        };
    }

    struct detail::Tk_impl : Misc::impl
    {
        bool _tkloaded = false;

        /// @brief Destroy this and all descendants widgets.
        /// 
        /// This will end the application of this Tcl interpreter.
        virtual void destroy() override;
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

        decltype(impl::_tkloaded)& _tkloaded;

    public:
        Tk(const std::shared_ptr<impl>& pimpl);
        /// @brief Create a new Tk object.
        ///
        /// A new Tcl interpreter will be created.
        /// @param baseName will be used for the identification of the profile file (see detail::Tk::readprofile()).
        /// It is constructed from @ref detail::argv[0] without extensions if none is given.
        /// @param className is the name of the widget class.
        /// @return A shared pointer to the newly created detail::Tk object.
        Tk(const std::string& screenName = {}, std::string baseName = {}, const std::string& className = "Tk", bool useTk = true, bool sync = false, const std::string& use = {});

        static std::shared_ptr<Tk> create(const std::string&, std::string, const std::string&, bool, bool, const std::string&);

        void loadtk();
    private:
        void _loadtk();

        /// @brief This function prints the exception to stderr.
        ///
        /// It is the default callback registered in @ref report_callback_exception.
        static void _report_callback_exception(Tk&, const std::exception_ptr& exc_ptr);
    public:
        /// @brief Internal function.
        /// 
        /// It reads .BASENAME.tcl and .CLASSNAME.tcl into the Tcl Interpreter.
        void readprofile(std::string_view baseName, std::string_view className);

        /// @brief Report callback exception on stderr.
        ///
        /// Applications may want to override this internal function. Default value is @ref _report_callback_exception-
        std::function<void(Tk&, const std::exception_ptr&)> report_callback_exception = &Tk::_report_callback_exception; // std::bind_front()

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
                throw detail::construct_exception<TclError>(std::format("got {} values but '{}' has {} members", map.size(), reflect::type_name<T>(), reflect::size<T>()));

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

    /// @brief Class to define value holders for e.g. buttons.
    ///
    /// Subclasses StringVar, IntVar, DoubleVar, BooleanVar are specializations that constrain the type of the value returned from get().
    class Variable
    {
    protected:
        std::set<std::string> _tclCommands{};
        Tk _root;
        std::shared_ptr<_cpptkinter::TkappObject> _tk;
        std::string _name;

        template<typename T>
        Variable(std::optional<Misc>& master, T&& value, const std::string& name, bool is_default) :
            _root(master.has_value() ? master->_root() : detail::_get_default_root("create variable"))
        {
			if (master.has_value())
				this->_tk = master->tk;
			else
                this->_tk = this->_root.tk;

            if (!name.empty())
                this->_name = name;
            else
                this->_name = std::format("PY_VAR{}", detail::_varnum++);

            if (!is_default)
                this->initialize(std::forward<T>(value));
            else if (this->_tk->call<long long>("info", "exists", this->_name))
                this->initialize(std::forward<T>(value));
            DEVIATING_IMPLEMENTATION_WARNING("original uses self.tk.getboolean to convert to bool");
        }

    public:
        /// @brief Construct a variable
        /// 
        /// The variable's name will be set PY_VARnum.
        /// @param master The master widget. Can be empty.
        Variable(std::optional<Misc> master = {});
        /// @brief Construct a variable
        /// 
        /// If name matches an existing variable and value is omitted then the existing value is retained.
        /// @param master The master widget. Can be empty.
        /// @param value The initial value of the variable.
        /// @param name An optional Tcl name (defaults to PY_VARnum).
        template<detail::AsObjConcept T>
		Variable(std::optional<Misc> master, T&& value, const std::string& name = {}) : Variable(master, std::forward<T>(value), name, false)
        {

        }

        /// @brief Unset the variable in Tcl.
        ~Variable();

        /// @brief Return the name of the variable in Tcl.
        operator std::string();

        /// @brief Set the variable to VALUE.
        void set(detail::AsObjConcept auto&& value)
        {
            this->_tk->globalsetvar(this->_name, std::forward<decltype(value)>(value));
        }
        /// @copydoc set
        void initialize(detail::AsObjConcept auto&& value)
        {
            this->set(std::forward<decltype(value)>(value));
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
            requires detail::AsObjConcept<const T&>&& detail::FromObjConcept<T>
        struct TypedVariable : Variable
        {
            /// @copydoc Variable::Variable(const detail::master_t&)
            TypedVariable(std::optional<Misc> master = {}) : Variable(master, T{}, {}, true)
            {

            }
            /// @copydoc Variable::Variable(const detail::master_t&, T&&, const std::string&)
			TypedVariable(std::optional<Misc> master, const T& value, const std::string& name = {}) : Variable(master, value, name, false)
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
        cnfs::_PackInfo pack_info(this auto&& self)
        {
            return detail::pack_grid_info<cnfs::_PackInfo>(self, "pack", "info", self._w);
        }
    };

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

        cnfs::_PlaceInfo place_info(this auto&& self)
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

            return detail::_splitdict_to_aggregate<cnfs::_PlaceInfo>(std::move(map), true, converter);
        }
    };

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
        cnfs::_GridInfo grid_info(this auto&& self)
        {
            return detail::pack_grid_info<cnfs::_GridInfo>(self, "grid", "info", self._w);
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
        decltype(impl::widgetName)& widgetName;
    private:
        decltype(impl::_name)& _name;

        /// @brief Internal function. Sets up information about children.
        template<typename Self>
        void _setup(this Self&& self, const std::optional<Misc>& master_, auto& cnf, std::set<std::string>& ignore_fields)
        {
            auto&& master = master_.has_value() ? master_.value() : detail::_get_default_root();
            self.master = master;
            self.tk = master.tk;

            std::string name{};
            if constexpr (requires { cnf.name; })
            {
                utility::invoke_or_and_then([&ignore_fields, &name]<typename T>(T && v) {
                    name = std::forward<T>(v);
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

    public:
        BaseWidget(const std::shared_ptr<impl>& pimpl);
    protected:
        /// @brief Construct a widget.
        ///
		/// master is passed within cnf instead of as a separate argument.
        template<cnfs::is_cnf CNF>
		BaseWidget(const std::string& widgetName_, CNF&& cnf, detail::Tcl_Obj_vector_raii extra = {}, std::set<std::string> ignore_fields = {}) :
            BaseWidget(std::make_shared<impl>())
        {
            std::optional<Misc> master{};
            if constexpr (requires { cnf.master; })
            {
                utility::invoke_or_and_then([&master]<typename T>(T&& v) {
                    master = std::forward<T>(v);
                }, std::forward<CNF>(cnf).master);
                ignore_fields.insert("master");
            }

            this->widgetName = widgetName_;
            this->_setup(master, cnf, ignore_fields);
            this->tk->call(this->widgetName, this->_w, std::move(extra), this->_options(std::forward<CNF>(cnf), std::move(ignore_fields)));
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

    /// @brief %Toplevel widget, e.g. for dialogs.
    class Toplevel : public BaseWidget, public Wm
    {
        template<cnfs::is_cnf CNF>
        static std::pair<detail::Tcl_Obj_vector_raii, std::set<std::string>> make_extra_and_ignore_fields(CNF&& cnf)
        {
            std::pair<detail::Tcl_Obj_vector_raii, std::set<std::string>> ret{};
            auto& [extra, ignore_fields] = ret;

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

			return ret;
        }

        template<cnfs::is_cnf CNF>
        Toplevel(CNF&& cnf, std::pair<detail::Tcl_Obj_vector_raii, std::set<std::string>>&& extra_and_ignore_fields) :
            BaseWidget("toplevel", std::forward<CNF>(cnf), std::move(extra_and_ignore_fields.first), std::move(extra_and_ignore_fields.second))
        {
            auto root = this->_root();
            this->iconname(root.iconname());
            this->title(root.title());
            this->protocol("WM_DELETE_WINDOW", std::function<void()>(std::bind_front(&Toplevel::destroy, *this)));
        }

    public:
        using BaseWidget::BaseWidget;

        /// @brief Create a new Toplevel widget.
        template<cnfs::is_cnf CNF = cnfs::Toplevel>
		Toplevel(CNF&& cnf = {}) : Toplevel(std::forward<CNF>(cnf), make_extra_and_ignore_fields(std::forward<CNF>(cnf)))
		{

		}
    };

    /// @brief %Button widget.
    /// 
    /// @see TypedButton, cpptkinter::Button()
    struct Button : Widget
    {
        using Widget::Widget;

        /// @brief Create a new Button widget.
        template<detail::PythonCmd_ClientDataReturnConcept T = void, cnfs::is_cnf CNF = cnfs::Button<T>>
		Button(CNF&& cnf = {}) : Widget("button", std::forward<CNF>(cnf))
        {

        }

        /// @brief Flash the button.
        /// 
        /// This is accomplished by redisplaying the button several times, alternating between active and normal colors.
        /// At the end of the flash the button is left in the same normal/active state as when the command was invoked.
        /// This command is ignored if the button's state is disabled.
        void flash();

        /// @brief Invoke the command associated with the button.
        ///
        /// The return value is the return value from the command, or an empty string if there is no command associated with the button.
        /// This command is ignored if the button's state is disabled.
        template<detail::FromObjConcept R = void>
        R invoke()
        {
            return this->tk->call<R>(this->_w, "invoke");
        }
    };

    /// @brief A button widget with defined callback return type.
    ///
    /// Button takes a callback with void return type. The original tkinter allows callbacks to have a return type/value.
    /// To emulate this, use TypedButton with the desired return type.
    /// @tparam T The return type of the callback.
    /// @see Button, cpptkinter::TypedButton()
    template<typename T>
        requires detail::FromObjConcept<T> && detail::PythonCmd_ClientDataReturnConcept<T>
    struct TypedButton : Button
    {
		using Button::Button;

        /// @brief Create a new TypedButton widget.
        template<cnfs::is_cnf CNF = cnfs::Button<T>>
        TypedButton(CNF&& cnf = {}) : Button(std::forward<CNF>(cnf))
		{

		}

        /// @copydoc Button::invoke
        T invoke()
        {
            return this->Button::template invoke<T>();
        }
    };

    /// @brief %Canvas widget to display graphical elements like lines or text.
    struct Canvas;

    /// @brief %Checkbutton widget which is either in on- or off-state.
    struct Checkbutton;

    /// @brief %Entry widget which allows displaying simple text.
    struct Entry;

	/// @brief %Frame widget which may contain other widgets and can have a 3D border.
    class Frame : public Widget
    {
        template<cnfs::is_cnf CNF>
        static std::pair<detail::Tcl_Obj_vector_raii, std::set<std::string>> make_extra_and_ignore_fields(CNF&& cnf)
        {
            std::pair<detail::Tcl_Obj_vector_raii, std::set<std::string>> ret{};
			auto& [extra, ignore_fields] = ret;

            if constexpr (requires { cnf.class_; })
            {
                utility::invoke_or_and_then([&extra , &ignore_fields]<typename T>(T && v) {
                    extra.emplace_back(_cpptkinter::AsObj("-class"));
                    extra.emplace_back(_cpptkinter::AsObj(std::forward<T>(v)));
                    ignore_fields.insert("class_");
                }, cnf.class_);
            }

            return ret;
        }

        template<cnfs::is_cnf CNF>
        Frame(CNF&& cnf, std::pair<detail::Tcl_Obj_vector_raii, std::set<std::string>>&& extra_and_ignore_fields) :
            Widget("frame", std::forward<CNF>(cnf), std::move(extra_and_ignore_fields.first), std::move(extra_and_ignore_fields.second))
        {
            
        }

    public:
        /// @brief Construct a frame widget.
        template<cnfs::is_cnf CNF = cnfs::Frame>
		Frame(CNF&& cnf = {}) : Frame(std::forward<CNF>(cnf), make_extra_and_ignore_fields(std::forward<CNF>(cnf)))
		{

		}
    };

    /// @brief %Label widget which can display text and bitmaps.
    struct Label;

    /// @brief %Listbox widget which can display a list of strings.
    struct Listbox;

    /// @brief %Menu widget which allows displaying menu bars, pull-down menus and pop-up menus.
    struct Menu;

    /// @brief %Menubutton widget, obsolete since Tk8.0.
    struct Menubutton;

    /// @brief %Message widget to display multiline text. Obsolete since Label does it too.
    struct Message;

    /// @brief %Radiobutton widget which shows only one of several buttons in on-state.
    struct Radiobutton;

    /// @brief %Scale widget which can display a numerical scale.
    struct Scale;

    /// @brief %Scrollbar widget which displays a slider at a certain position.
    struct Scrollbar;

    /// @brief %Text widget which can display text in various forms.
    struct Text;

    /// @brief %OptionMenu which allows the user to select a value from a menu.
    struct OptionMenu;

    /// @brief %Base class for images.
    struct Image;

    /// @brief %Widget which can display images in PGM, PPM, GIF, PNG format.
    struct PhotoImage;

    /// @brief %Widget which can display images in XBM format.
    struct BitmapImage;

    /// @brief %Spinbox widget.
    struct Spinbox;

    /// @brief %Labelframe widget.
    struct LabelFrame;

    /// @brief %Panedwindow widget.
    struct PanedWindow;
}
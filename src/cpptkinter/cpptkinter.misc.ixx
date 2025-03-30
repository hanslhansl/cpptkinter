module;
#include "../global.hpp"
export module cpptkinter:cpptkinter.misc;
import :utility;
import :cpptkinter.detail;
import :cpptkinter.wm;
import std;


using substitute_long_long = const std::variant<long long, std::string>&;
#define MISC_SUBSTITUTE_PARAMETERS  const std::string& nsign, substitute_long_long b, substitute_long_long f, substitute_long_long h, substitute_long_long k, const std::string& s, \
                                    const std::string& t, substitute_long_long w, substitute_long_long x, substitute_long_long y, const std::string& A, substitute_long_long E, \
                                    const std::string& K, substitute_long_long N, const std::string& W, substitute_long_long T, const std::string& X, const std::string& Y, \
                                    substitute_long_long D
#define MISC_SUBSTITUTE_ARGUMENTS nsign, b, f, h, k, s, t, w, x, y, A, E, K, N, W, T, X, Y, D


export namespace cpptkinter::cnfs
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
    template<typename T, std::size_t...I>
        requires (!std::is_array_v<std::remove_cvref_t<T>>) && (is_cnf_member<std::remove_cvref_t<reflect::member_type<I, T>>> && ...)
    struct is_cnf_trait<T, std::integer_sequence<std::size_t, I...>> : std::true_type {};
    /// @brief Satsified if T is a cnf struct.
    /// 
    /// Usually cnf structs are passed to cpptkinter::Misc::_options() internally.
    template<typename T>
    concept is_cnf = is_cnf_trait<std::remove_cvref_t<T>>::value;

    template<typename T> requires requires { typename std::remove_cvref_t<T>::constructor_cnf; }
    using get_constructor_cnf = typename std::remove_cvref_t<T>::constructor_cnf;

    template<typename T>
    using opt = std::optional<T>;
    using opt_screenunits = opt<detail::ScreenUnits>;
    using opt_string = opt<std::string>;

    /// @brief Argument for Misc::grid_columnconfigure() and Misc::grid_rowconfigure().
    struct grid_column_row_configure
    {
        opt_screenunits minsize;
        opt_screenunits pad;
        opt_string uniform;
        opt<std::size_t> weight;
    };

    /// @brief Argument for Misc::grid_bbox().
    struct grid_bbox
    {
        opt<long long> column, row, col2, row2;
    };
}

export namespace cpptkinter
{
    class Variable;
    class Tk;

    /// @brief Internal class.
    /// 
    /// Base class which defines methods common for interior widgets.
    class Misc : public utility::enable_operator_string_formatting
    {
        template<typename R, typename...Args>
        friend struct detail::CallWrapper;
        friend Wm;
        template<typename Self>
        friend struct XView;
        template<typename Self>
        friend struct YView;
        friend struct Pack;
        friend struct Grid;
        friend struct Place;
        friend class BaseWidget;
        friend Variable;
        friend detail::Tk_impl;
        friend detail::widget_friend;

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
        Misc(const std::shared_ptr<I>& pimpl) noexcept :
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
        DEFINE_COPY_MOVE_CONSTRUCTORS_AND_ASSIGNMENT(Misc);

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

	private:
        std::string after_impl(const auto& ms, std::function<void()>&& func)
        {
            struct callit
            {
                std::function<void()> func;
                std::shared_ptr<std::string> name;
                Misc self_;

				void operator()() const
				{
                    // self.deletecommand() destructs this so therefor we need to prevent name and self from destruction
                    auto name = *this->name;
					auto self = this->self_;
                    try
                    {
                        this->func();
                    }
					catch (...) { }

                    try
                    {
                        self.deletecommand(name);
                    }
                    catch (const TclError&) { }
				}
            } callit{ std::move(func), std::make_shared<std::string>(), *this };

			auto& name = *callit.name;
			name = this->_register(std::move(callit));
			return this->tk->call<std::string>("after", ms, name);
        }
    public:
        std::string after(long long ms, std::function<void()> func)
        {
			return this->after_impl(ms, std::move(func));
        }
        std::string after(const std::string& ms, std::function<void()> func)
        {
			return this->after_impl(ms, std::move(func));
        }

    protected:
        Tk _root() const;

    public:
        /// @brief Return requested height of this widget.
        long long winfo_reqheight()
        {
            return this->tk->template call<long long>("winfo", "reqheight", this->_w);
        }

        /// @brief Return requested width of this widget.
        long long winfo_reqwidth()
        {
            return this->tk->template call<long long>("winfo", "reqwidth", this->_w);
        }

        /// @brief Set the list of bindtags for this widget.
        /// 
        /// The bindtags determine in which order events are processed(see bind).
        void bindtags(const std::vector<std::string>& tagList)
        {
            this->tk->call("bindtags", this->_w, tagList);
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
        void _bind(std::vector<std::string>&& what, const std::string& sequence, const std::string& func, bool = false, bool = true)
        {
            this->tk->call(what | std::views::transform(_cpptkinter::AsObj), sequence, func);
        }
    private:
        template<typename Func>
        std::string _bind_if_2(Func&& func, bool needcleanup)
        {
            return this->_register(
                [func = std::forward<Func>(func), self = utility::weak(*this)](MISC_SUBSTITUTE_PARAMETERS) {
                    return func(self.lock()._substitute(MISC_SUBSTITUTE_ARGUMENTS));
                },
                needcleanup
            );
        }
    public:
        /// @brief Internal function.
        /// 
        /// Implements the second if statement with sequence != None.
        /// Creates a tcl command with func and binds it to sequence.
        /// @returns An identifier for the created tcl command.
        template<std::invocable<Event> Func>
        std::string _bind(const std::vector<std::string>& what, const std::string& sequence, Func&& func, bool add = false, bool needcleanup = true)
        {
            auto funcid = this->_bind_if_2(std::forward<Func>(func), needcleanup);
            auto cmd = std::format("{}if {{\"[{} {}]\" == \"break\"}} break\n", add ? "+" : "", funcid, this->_subst_format_str);
            this->tk->call(what | std::views::transform(_cpptkinter::AsObj), sequence, cmd);
            return funcid;
        }
        /// @brief Internal function.
        /// 
        /// Implements the second if statement with sequence == None.
        /// Creates a tcl command with func but doesn't bind it (i.e. does nothing with it).
        /// @returns An identifier for the created tcl command.
        template<std::invocable<Event> Func>
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
            return this->tk->call<std::string>(what | std::views::transform(_cpptkinter::AsObj), sequence);
        }
        /// @brief Internal function.
        /// 
        /// Implements the fourth if statement.
        /// @returns A list of all bound events associated with this widget.
        std::vector<std::string> _bind(std::vector<std::string>&& what)
        {
            return this->tk->call<std::vector<std::string>>(what | std::views::transform(_cpptkinter::AsObj));
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
            this->tk->call(what | std::views::transform(_cpptkinter::AsObj), "");
        }
        /// @brief Internal function.
        /// 
        /// Implements the second if statement.
        void _unbind(std::vector<std::string>&& what, const std::string& funcid)
        {
            auto prefix = std::format("if {{\"[{} ", funcid);
            std::string keep{};
            for (auto&& s : this->tk->call<std::string>(what | std::views::transform(_cpptkinter::AsObj))
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

            this->tk->call(what | std::views::transform(_cpptkinter::AsObj), keep);
            this->deletecommand(funcid);
        }

        /// @brief Bind to widgets with bindtag CLASSNAME at event SEQUENCE a call of function FUNC.
        /// 
        /// An additional boolean parameter ADD specifies whether FUNC will be called additionally to the other bound function or whether it will replace the previous function.
        /// @see bind for the return value.
        template<typename...Args>
        auto bind_class(const std::string& className, Args&&...args) requires requires { this->_bind({}, std::forward<Args>(args)..., true); }
        {
            return detail::widget_friend::misc_bind_class_impl(this, className, std::forward<Args>(args)...);
        }

        /// @brief Unbind for all widgets with bindtag CLASSNAME for event SEQUENCE all functions.
        void unbind_class(const std::string& className, const std::string& sequence);

        /// @brief Bind to all widgets at an event SEQUENCE a call to function FUNC.
        /// 
        /// An additional boolean parameter ADD specifies whether FUNC will be called additionally to the other bound function or whether it will replace the previous function.
        /// @see bind for the return value.
        template<typename...Args>
        auto bind_all(Args&&...args) requires requires { this->_bind({}, std::forward<Args>(args)..., true); }
        {
            return detail::widget_friend::misc_bind_class_impl(this, "all", std::forward<Args>(args)...);
        }

        /// @brief Unbind for all widgets for event SEQUENCE all functions.
        void unbind_all(const std::string& sequence)
        {
            this->unbind_class("all", sequence);
        }

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
    private:
        template<typename T>
            requires (cnfs::is_cnf_member<std::remove_cvref_t<T>> && !hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::optional>)
        _cpptkinter::Tcl_Obj _options_inner_visitor(T&& value)
        {
            auto visitor = [&]<typename T2>(T2 && value) {
                if constexpr (detail::createcommand_concept<T2>)
                    return _cpptkinter::AsObj(this->_register(std::forward<T2>(value)));
                else
                    return _cpptkinter::AsObj(value);
            };

            return utility::visit_or_invoke(visitor, std::forward<T>(value));
        }
    protected:
        /// @brief Converts a cnf struct to a vector of Tcl_Obj* which can be passed to TkappObject::call().
        template<cnfs::is_cnf CNF>
        std::vector<_cpptkinter::Tcl_Obj> _options(CNF&& cnf, const std::set<std::string>& ignore_fields = {})
        {
            std::vector<_cpptkinter::Tcl_Obj> raii{};

            auto visitor = [&]<typename T>(T && value, auto I) {
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
        Misc nametowidget(const _cpptkinter::tk_window_type& window)
        {
            return this->nametowidget((std::string)window);
        }

    protected:
        /// @brief Return a newly created Tcl function.
        ///
        /// If said Tcl function is called, the C++ function func will be executed.
        template<detail::createcommand_concept Func>
        std::string _register(Func&& func, bool needcleanup = true)
        {
            DEVIATING_IMPLEMENTATION_WARNING("original has subst");

            auto f = detail::CallWrapper{ utility::callable_to_std_function(std::forward<Func>(func)), *this };
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

		/// @copydoc _register
        template<detail::createcommand_concept Func>
        std::string register_(Func&& func, bool needcleanup = true)
        {
			return this->_register(std::forward<Func>(func), needcleanup);
        }

    protected:
        static constexpr std::array _subst_format = { "%#"sv, "%b"sv, "%f"sv, "%h"sv, "%k"sv, "%s"sv, "%t"sv, "%w"sv, "%x"sv, "%y"sv, "%A"sv,
            "%E"sv, "%K"sv, "%N"sv, "%W"sv, "%T"sv, "%X"sv, "%Y"sv, "%D"sv };
        static const inline std::string _subst_format_str = hhh::misc::join_strings(_subst_format, " ");
        /// @brief Internal function.
        Event _substitute(MISC_SUBSTITUTE_PARAMETERS)
        {
            // print args

            // [&](auto&...args) { (utility::visit_or_invoke([](auto& a) { std::println("{}", a); }, args), ...); }(MISC_SUBSTITUTE_ARGUMENTS);

            static auto get_long_long = [](auto&& p, long long def = std::numeric_limits<long long>::min()) {

                auto inner = [&]<typename T>(const T & arg)->long long {
                    if constexpr (std::same_as<T, std::string>)
                    {
                        if (arg == "??")
                            return def;

                        try
                        {
                            return std::stoll(arg);
                        }
                        catch (const std::invalid_argument& e)
                        {
                            throw utility::construct_exception<std::runtime_error>(std::format("{} on argument {}", e.what(), arg));
                        }
                    }
                    else
                    {
                        return arg;
                    }
                };

                return utility::visit_or_invoke(inner, p);
                };

            static auto get_bool = [&]<typename T>(const T & p) {
                auto ll = get_long_long(p, 0);
                if (ll == 0)
                    return false;
                else if (ll == 1)
                    return true;
                else
                    throw utility::construct_exception<std::runtime_error>(std::format("expected 0 or 1 but got {}", ll));
            };

            auto serial = get_long_long(nsign);
            auto num = get_long_long(b);
            auto focus = get_bool(f);
            auto height = get_long_long(h);
            auto width = get_long_long(w);
            auto keycode = get_long_long(k);
            auto state = get_long_long(s);
            auto time = get_long_long(t);
            auto x_ = get_long_long(x);
            auto y_ = get_long_long(y);
            auto x_root = get_long_long(X);
            auto y_root = get_long_long(Y);
            auto char_ = A;
            auto send_event = get_bool(E);
            auto keysym = K;
            auto keysym_num = get_long_long(N);
            auto type = EventType(get_long_long(T));
            auto widget = this->nametowidget(W);
            auto delta = get_long_long(D, 0);

            return {
                    serial,
                    num,
                    focus,
                    height,
                    width,
                    keycode,
                    state,
                    time,
                    x_,
                    y_,
                    x_root,
                    y_root,
                    char_,
                    send_event,
                    keysym,
                    keysym_num,
                    type,
                    widget,
                    delta
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
        void _configure(const std::vector<std::string>& cmd, CNF&& cnf, const std::set<std::string>& ignore_fields = {})
        {
            this->tk->call(this->_w, cmd | std::views::transform(_cpptkinter::AsObj), this->_options(std::forward<CNF>(cnf), ignore_fields));
        }
    public:
        /// @brief Configure resources of a widget.
        /// 
        /// To get an overview about the allowed keyword arguments call the method keys.
        /// @param cnf A string or cnf struct.
        auto configure(const std::string& cnf)
        {
            return this->_configure({ "configure" }, cnf);
        }
        /// @brief Configure resources of a widget.
        /// 
        /// To get an overview about the allowed keyword arguments call the method keys.
        /// @param cnf A string or cnf struct.
        template<typename Self, cnfs::is_cnf CNF = cnfs::get_constructor_cnf<Self>>
        auto configure(this Self&& self, CNF&& cnf)
        {
            if (cnf.master.has_value())
                throw utility::construct_exception<std::invalid_argument>("master cannot be set in configure");
            self._configure({ "configure" }, std::forward<CNF>(cnf), { "name" });
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

        /// @copydoc configure(const std::string&)
        template<typename T>
        auto config(const std::string& cnf)
        {
            return this->configure(cnf);
        }
        /// @copydoc configure(this Self&&, CNF&&)
        template<typename Self, cnfs::is_cnf CNF = cnfs::get_constructor_cnf<Self>>
        auto config(this Self&& self, CNF&& cnf)
        {
            return self.configure(std::forward<CNF>(cnf));
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
        detail::grid_column_row_configure_return _grid_configure(const std::string& command, const std::variant<long long, std::string>& index)
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
                throw utility::construct_exception<TclError>("unexpected return value " + detail::format_tuple(temp));

            return { std::get<1>(temp), std::get<3>(temp), std::get<5>(temp), std::get<7>(temp) };
        }
    public:
        /// @brief Configure column INDEX of a grid.
        detail::grid_column_row_configure_return grid_columnconfigure(detail::index auto&& index)
        {
            return this->_grid_configure("columnconfigure", detail::to_index(index));
        }
        /// @brief Configure column INDEX of a grid.
        /// 
        /// Valid resources are minsize (minimum size of the column), weight (how much does additional space propagate to this column),
        /// pad (how much space to let additionally) and uniform.
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void grid_columnconfigure(std::variant<std::size_t, std::vector<std::size_t>, std::string> index, CNF&& cnf)
        {
            this->tk->call("grid", "columnconfigure", this->_w, index, this->_options(std::forward<CNF>(cnf)));
        }

        /// @copydoc grid_columnconfigure(const std::variant<long long, std::string>&)
        detail::grid_column_row_configure_return columnconfigure(detail::index auto&& index)
        {
            return this->grid_columnconfigure(detail::to_index(index));
        }
        /// @copydoc grid_columnconfigure()
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void columnconfigure(std::variant<std::size_t, std::vector<std::size_t>, std::string> index, CNF&& cnf)
        {
            return this->grid_columnconfigure(index, std::forward<CNF>(cnf));
        }

        /// @brief Return a tuple of column and row which identify the cell at which the pixel at position X and Y inside the master widget is located.
        std::array<long long, 2> grid_location(detail::screenunits_arg auto&& x, detail::screenunits_arg auto&& y)
        {
            return this->tk->call<std::array<long long, 2>>("grid", "location", this->_w, detail::to_screenunits_arg(x), detail::to_screenunits_arg(y));
        }
        /// @copydoc grid_location
        std::array<long long, 2> location(detail::screenunits_arg auto&& x, detail::screenunits_arg auto&& y)
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
        detail::grid_column_row_configure_return grid_rowconfigure(detail::index auto&& index)
        {
            return this->_grid_configure("rowconfigure", detail::to_index(index));
        }
        /// @brief Configure row INDEX of a grid.
        ///
        /// Valid resources are minsize (minimum size of the row), weight (how much does additional space propagate to this row),
        /// pad (how much space to let additionally) and uniform.
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void grid_rowconfigure(const std::variant<std::size_t, std::vector<std::size_t>, std::string>& index, CNF&& cnf)
        {
            this->tk->call("grid", "rowconfigure", this->_w, index, this->_options(std::forward<CNF>(cnf)));
        }

        /// @copydoc grid_rowconfigure(const std::variant<long long, std::string>&)
        detail::grid_column_row_configure_return rowconfigure(detail::index auto&& index)
        {
            return this->grid_rowconfigure(detail::to_index(index));
        }
        /// @copydoc grid_rowconfigure()
        template<cnfs::is_cnf CNF = cnfs::grid_column_row_configure>
        void rowconfigure(const std::variant<std::size_t, std::vector<std::size_t>, std::string>& index, CNF&& cnf)
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
}

void cpptkinter::Misc::destroy()
{
    this->pimpl->destroy();
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
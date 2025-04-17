module;
#include "../global.hpp"
export module cpptkinter:cpptkinter1;
import :utility;
import :_cpptkinter;
import :cpptkinter.detail;
export import :cpptkinter.misc;
import std;


export namespace cpptkinter
{
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
        friend Tk detail::_get_default_root(const std::string&);

    protected:
        using impl = detail::Tk_impl;
        REF_TO_IMPL(_tkloaded);

        void _init_(const std::string& screenName, const std::string& className, bool useTk, bool sync, const std::string& use)
        {
            this->_w = ".";

            auto interactive = false;
            this->tk = _cpptkinter::TkappObject::create(screenName, className, interactive, useTk, sync, use);
            if (detail::_debug)
                this->tk->settrace(detail::_print_command);
            if (useTk)
                this->_loadtk();
            this->readprofile(className);
        }

        DEFINE_IMPL_CONSTRUCTOR(Tk, Misc);
        DEFINE_COPY_MOVE_CONSTRUCTORS_AND_ASSIGNMENT(Tk);

        /// @brief Create a new Tk object.
        ///
        /// A new Tcl interpreter will be created.
        /// @param baseName will be used for the identification of the profile file (see detail::Tk::readprofile()).
        /// It is constructed from @ref detail::argv[0] without extensions if none is given.
        /// @param className is the name of the widget class.
        /// @return A shared pointer to the newly created detail::Tk object.
        Tk(const std::string& screenName = {}, const std::string& className = "Tk", bool useTk = true, bool sync = false, const std::string& use = {}) : Tk(std::make_shared<impl>())
        {
            this->_init_(screenName, className, useTk, sync, use);
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
                throw utility::construct_exception<std::runtime_error>(std::format("tk.h version {} doesn't match libtk.a version {}", _cpptkinter::TK_VERSION, tk_version));

            // Under unknown circumstances, tcl_version gets coerced to float
            auto tcl_version = this->tk->getvar<std::string>("tcl_version");
            if (tcl_version != _cpptkinter::TCL_VERSION)
                throw utility::construct_exception<std::runtime_error>(std::format("tcl.h version {} doesn't match libtcl.a version {}", _cpptkinter::TCL_VERSION, tcl_version));

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
        void readprofile(const std::string& className)
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

    Tk Tcl(const std::string& screenName = {}, const std::string& className = "Tk", bool useTk = true)
    {
        return Tk(screenName, className, useTk);
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
            opt_string name;
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
            requires detail::AsObjConcept<T>&& std::default_initializable<T>
        void _init_(const cnfs::Variable<T>& cnf)
        {
            if (cnf.master.has_value())
                this->_tk = cnf.master->tk;
            else
                this->_tk = this->_root.tk;

            if (cnf.name.has_value() && !cnf.name.value().empty())
                this->_name = cnf.name.value();
            else
                this->_name = std::format("PY_VAR{}", detail::_varnum++);

            if (cnf.value.has_value())
                this->initialize(cnf.value.value());
            else if (!this->_tk->call<long long>("info", "exists", this->_name))
                this->initialize(T{});
            DEVIATING_IMPLEMENTATION_WARNING("original uses self.tk.getboolean to convert to bool");
        }

        template<std::derived_from<impl> I>
        Variable(const std::shared_ptr<I>& pimpl) : pimpl(pimpl)
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

        DEFINE_COPY_MOVE_CONSTRUCTORS_AND_ASSIGNMENT(Variable);

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
        R get() const
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
        requires detail::AsObjConcept<T>&& detail::FromObjConcept<T>&& std::default_initializable<T>
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
        T get() const
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

    /// @brief %Base class for images.
    struct Image
    {
    private:
        static inline long long _last_id = 0;
    public:
        std::string name;
        std::shared_ptr<_cpptkinter::TkappObject> tk;

        template<typename CNF>
        Image(const std::string& imgtype, CNF&& cnf) : tk{ !cnf.master.has_value() ?
                detail::_get_default_root("create image").tk :
            (std::holds_alternative<Misc>(cnf.master.value()) ?
                std::get<Misc>(cnf.master.value()).tk :
                std::get<std::shared_ptr<_cpptkinter::TkappObject>>(cnf.master.value())) }
        {
            const static std::set<std::string> ignore = { "name", "master" };
            if (cnf.name.empty())
            {
                Image::_last_id++;
                this->name = std::format("cppimage{}", Image::_last_id);
            }

            std::vector<_cpptkinter::Tcl_Obj> options{};
            auto visitor = [&]<typename T>(T && value, auto I) {
                constexpr auto k = reflect::member_name<CNF, I>();

                if constexpr (k != "name" && k != "master")
                {
                    options.emplace_back(_cpptkinter::AsObj("-" + std::string(k)));
                    options.emplace_back(_cpptkinter::AsObj(std::forward<T>(value)));
                }
            };
            reflect::enumerate<CNF>([&](auto I) { utility::invoke_or_and_then(visitor, reflect::get<I>(std::forward<CNF>(cnf)), I); });

            this->tk->call("image", "create", imgtype, this->name, options);
        }

        ~Image()
        {
            if (!this->name.empty())
            {
                this->tk->call("image", "delete", this->name);
            }
        }

        operator std::string() const
        {
            return this->name;
        }

        template<detail::AsObjConcept T>
        void _setitem_(const std::string& key, T&& value)
        {
            this->tk->call(this->name, "configure", "-" + key, std::forward<T>(value));
        }

        template<detail::FromObjConcept R>
        R _getitem_(const std::string& key, std::type_identity<R>)
        {
            return this->tk->call<R>(this->name, "configure", "-" + key);
        }

        /// @brief Returns a proxy object which can be used to set/get resources of a widget.
        template<typename Self>
        detail::set_get_proxy<std::remove_cvref_t<Self>> operator[](this Self&& self, const std::string& key)
        {
            return { std::forward<Self>(self), key };
        }

        /// @brief Configure the image.
        template<typename Self, cnfs::is_cnf CNF = cnfs::get_constructor_cnf<Self>>
        void configure(this Self&& self, CNF&& cnf)
        {
            std::vector<_cpptkinter::Tcl_Obj> options{};
            auto visitor = [&]<typename T>(T && value, auto I) {
                constexpr auto k = reflect::member_name<I, CNF>();

                if constexpr (k != "name" && k != "master")
                {
                    auto key = std::string(k);
                    if (key.ends_with('_'))
                        key.pop_back();
                    options.emplace_back(_cpptkinter::AsObj("-" + key));
                    options.emplace_back(_cpptkinter::AsObj(std::forward<T>(value)));
                }
            };
            reflect::for_each<CNF>([&](auto I) { utility::invoke_or_and_then(visitor, reflect::get<I>(std::forward<CNF>(cnf)), I); });

            self.tk->call(self.name, "config", options);
        }
        /// @copydoc configure
        template<typename Self, cnfs::is_cnf CNF = cnfs::get_constructor_cnf<Self>>
        void config(this Self&& self, CNF&& cnf)
        {
            self.configure(std::forward<CNF>(cnf));
        }

        /// @brief Return the height of the image.
        long long height()
        {
            return this->tk->call<long long>("image", "height", this->name);
        }

        /// @brief Return the type of the image, e.g. "photo" or "bitmap".
        std::string type()
        {
            return this->tk->call<std::string>("image", "type", this->name);
        }

        /// @brief Return the width of the image.
        long long width()
        {
            return this->tk->call<long long>("image", "width", this->name);
        }
    };
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

cpptkinter::Tk cpptkinter::detail::_get_default_root(const std::string& what)
{
    if (!_support_default_root)
        throw utility::construct_exception<std::runtime_error>("No master specified and tkinter is configured to not support default root");

    if (_default_root.get() == nullptr)
    {
        if (!what.empty())
            throw utility::construct_exception<std::runtime_error>(std::format("Too early to {}: no default root window", what));
        auto root = cpptkinter::Tk();
        if (_default_root != root.pimpl)
            throw utility::construct_exception<std::runtime_error>("?");
        return root;
    }
    else
        return std::shared_ptr(_default_root);
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

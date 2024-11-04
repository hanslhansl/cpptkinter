#include "__init__.hpp"

/// @file __init__.cpp
/// @brief Implements __init__.py.


void cpptkinter::detail::_print_command(std::vector<std::string> cmd)
{
    for (auto& c : cmd)
        std::cerr << c << " ";
    std::cerr << std::endl;
}

void cpptkinter::detail::_tkerror()
{

}

void cpptkinter::detail::_exit()
{
    throw detail::construct_exception<std::runtime_error>("");
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

void cpptkinter::init(int argc, char* argv[], const std::string& tcl_library)
{
    _cpptkinter::detail::argc = argc;
    _cpptkinter::detail::argv = argv;

    _cpptkinter::init(tcl_library);
}

void cpptkinter::mainloop(int n)
{
    detail::_get_default_root("call mainloop").tk->mainloop(n);
}

std::ostream& cpptkinter::operator<<(std::ostream& os, const Misc& self)
{
    return os << self._w;
}

void cpptkinter::Misc::impl::destroy()
{
    // keeps this from being destroyed before this function returns
    auto temp = this->shared_from_this();

    for (auto&& name : this->_tclCommands)
        this->tk->deletecommand(name);
}

cpptkinter::Misc::Misc(const std::shared_ptr<impl>& pimpl) :
    pimpl(pimpl),
    _tclCommands(pimpl->_tclCommands),
    _last_child_ids(pimpl->_last_child_ids),
	_w(pimpl->_w),
    master(pimpl->master),
    tk(pimpl->tk),
    children(pimpl->children)
{

}

cpptkinter::Misc& cpptkinter::Misc::operator=(const Misc& other)
{
    std::destroy_at(this);
    return *std::construct_at(this, other);
}

void cpptkinter::Misc::destroy()
{
	this->pimpl->destroy();
}

void cpptkinter::Misc::mainloop(int n)
{
    this->tk->mainloop(n);
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

cpptkinter::Misc cpptkinter::Misc::nametowidget(_cpptkinter::tk_window_type window)
{
    return this->nametowidget(Tcl_GetString(window.get()));
}

cpptkinter::Tk cpptkinter::Misc::_root()
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

std::vector<std::vector<std::string>> cpptkinter::Misc::_getconfigure(detail::Tcl_Obj_vector_raii&& raii)
{
    NOT_IMPLEMENTED_ERROR;
    this->tk->call<void>(std::move(raii));
}

std::vector<std::string> cpptkinter::Misc::_getconfigure1(detail::Tcl_Obj_vector_raii&& raii)
{
    NOT_IMPLEMENTED_ERROR;
    this->tk->call<void>(std::move(raii));
}

std::vector<std::vector<std::string>> cpptkinter::Misc::_configure(const std::vector<std::string>& cmd)
{
    detail::Tcl_Obj_vector_raii raii{ };
    raii.emplace_back(_cpptkinter::AsObj(this->_w));
    for (auto& c : cmd)
        raii.emplace_back(_cpptkinter::AsObj(c));

    return this->_getconfigure(std::move(raii));
}

std::vector<std::string> cpptkinter::Misc::_configure(const std::vector<std::string>& cmd, const std::string& cnf)
{
    detail::Tcl_Obj_vector_raii raii{ };
    raii.emplace_back(_cpptkinter::AsObj(this->_w));
    for (auto& c : cmd)
        raii.emplace_back(_cpptkinter::AsObj(c));
    raii.emplace_back(_cpptkinter::AsObj("-" + cnf));

    return this->_getconfigure1(std::move(raii));
}

cpptkinter::detail::set_get_proxy cpptkinter::Misc::operator[](const std::string& key)
{
    return { *this, key };
}

std::vector<std::string> cpptkinter::Misc::keys()
{
    auto vec = this->tk->call<std::vector<std::vector<std::variant<std::string, detail::ignore>>>>(this->_w, "configure");
    std::vector<std::string> res{};
    for (auto& outer : vec)
        res.emplace_back(std::get<std::string>(std::move(outer.at(0))).substr(1));
    return res;
}

cpptkinter::Misc::operator std::string() const
{
    return this->_w;
}

bool cpptkinter::Misc::pack_propagate()
{
    return this->tk->call<long long>("pack", "propagate", this->_w);
}

void cpptkinter::Misc::pack_propagate(bool flag)
{
    this->tk->call("pack", "propagate", this->_w, flag);
}

bool cpptkinter::Misc::propagate()
{
    return this->pack_propagate();
}

void cpptkinter::Misc::propagate(bool flag)
{
    this->pack_propagate(flag);
}

std::vector<cpptkinter::Misc> cpptkinter::Misc::pack_slaves()
{
    auto temp = this->tk->call<std::vector<_cpptkinter::tk_window_type>>("pack", "slaves", this->_w);
    std::vector<cpptkinter::Misc> result{};
    for (auto& t : temp)
        result.emplace_back(this->nametowidget(std::move(t)));
    return result;
}

std::vector<cpptkinter::Misc> cpptkinter::Misc::slaves()
{
    return this->pack_slaves();
}

std::vector<cpptkinter::Misc> cpptkinter::Misc::place_slaves()
{
    auto temp = this->tk->call<std::vector<_cpptkinter::tk_window_type>>("place", "slaves", this->_w);
    std::vector<Misc> result{};
    for (auto& t : temp)
        result.emplace_back(this->nametowidget(std::move(t)));
    return result;
}

void cpptkinter::Misc::grid_anchor(const std::string& anchor)
{
    if (anchor.empty())
        this->tk->call("grid", "anchor", this->_w);
    else
        this->tk->call("grid", "anchor", this->_w, anchor);
}

void cpptkinter::Misc::anchor(const std::string& anchor)
{
    this->grid_anchor(anchor);
}

std::array<long long, 4> cpptkinter::Misc::grid_bbox(const cnfs::grid_bbox& cnf)
{
    detail::Tcl_Obj_vector_raii args{ };
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

std::array<long long, 4> cpptkinter::Misc::bbox(const cnfs::grid_bbox& cnf)
{
    return this->grid_bbox(cnf);
}

cpptkinter::cnfs::grid_column_row_configure_return cpptkinter::Misc::_grid_configure(const std::string& command, const std::variant<long long, std::string>& index)
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
        throw detail::construct_exception<TclError>("unexpected return value " + utility::container_or_tuple_to_string(temp));
    return { std::get<1>(temp), std::get<3>(temp), std::get<5>(temp), std::get<7>(temp) };
}

cpptkinter::cnfs::grid_column_row_configure_return cpptkinter::Misc::grid_columnconfigure(const std::variant<long long, std::string>& index)
{
    return this->_grid_configure("columnconfigure", index);
}

cpptkinter::cnfs::grid_column_row_configure_return cpptkinter::Misc::columnconfigure(const std::variant<long long, std::string>& index)
{
    return this->grid_columnconfigure(index);
}

std::array<long long, 2> cpptkinter::Misc::grid_location(const detail::_ScreenUnits& x, const detail::_ScreenUnits& y)
{
    return this->tk->call<std::array<long long, 2>>("grid", "location", this->_w, x, y);
}

std::array<long long, 2> cpptkinter::Misc::location(const detail::_ScreenUnits& x, const detail::_ScreenUnits& y)
{
    return this->grid_location(x, y);
}

bool cpptkinter::Misc::grid_propagate()
{
    return this->tk->call<long long>("grid", "propagate", this->_w);
}

void cpptkinter::Misc::grid_propagate(bool flag)
{
    this->tk->call<std::string>("grid", "propagate", this->_w, flag);
}

cpptkinter::cnfs::grid_column_row_configure_return cpptkinter::Misc::grid_rowconfigure(const std::variant<long long, std::string>& index)
{
    return this->_grid_configure("rowconfigure", index);
}

cpptkinter::cnfs::grid_column_row_configure_return cpptkinter::Misc::rowconfigure(const std::variant<long long, std::string>& index)
{
	return this->grid_rowconfigure(index);
}

std::array<long long, 2> cpptkinter::Misc::grid_size()
{
	return this->tk->call<std::array<long long, 2>>("grid", "size", this->_w);
}

std::array<long long, 2> cpptkinter::Misc::size()
{
	return this->grid_size();
}

std::vector<cpptkinter::Misc> cpptkinter::Misc::grid_slaves(std::optional<long long> row, std::optional<long long> column)
{
    detail::Tcl_Obj_vector_raii args{ };
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

cpptkinter::Variable::Variable(std::optional<Misc> master) : Variable(master, "", {}, true)
{

}

cpptkinter::Variable::operator std::string()
{
    return this->_name;
}

std::vector<std::tuple<std::vector<std::string>, std::string>> cpptkinter::Variable::trace_info()
{
    return this->_tk->call<std::vector<std::tuple<std::vector<std::string>, std::string>>>("trace", "info", "variable", this->_name);
}

cpptkinter::Variable::~Variable()
{
    if (this->_tk == nullptr)
        return;
    DEVIATING_IMPLEMENTATION_WARNING("original uses self.tk.getboolean to convert to bool");
    if (this->_tk->call<long long>("info", "exists", this->_name))
        this->_tk->globalunsetvar(this->_name);
    for (auto&& name : this->_tclCommands)
        this->_tk->deletecommand(name);
}

void cpptkinter::detail::Tk_impl::destroy()
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

void cpptkinter::BaseWidget::impl::destroy()
{
    // keeps this from being destroyed before this function returns
    auto temp = this->shared_from_this();

    for (auto&& child : std::vector(std::from_range, std::views::values(this->children)))
        child.destroy();
    this->tk->call("destroy", this->_w);
    this->master.value().children.erase(this->_name);
    this->Misc::impl::destroy();
}

cpptkinter::BaseWidget::BaseWidget(const std::shared_ptr<impl>& pimpl) :
    Misc(std::move(pimpl)),
	widgetName(pimpl->widgetName),
	_name(pimpl->_name)
{

}

cpptkinter::Tk::Tk(const std::shared_ptr<impl>& pimpl) :
    Misc(pimpl),
    _tkloaded(pimpl->_tkloaded)
{

}

cpptkinter::Tk::Tk(const std::string& screenName, std::string baseName, const std::string& className, bool useTk, bool sync, const std::string& use) : Tk(std::make_shared<impl>())
{
    this->_w = ".";

    if (baseName.empty())
    {
        auto p = std::filesystem::path(_cpptkinter::detail::argv[0]);

        auto ext = p.extension();
        if (ext != ".py" && ext != ".pyc")
            baseName = p.string();
        else
            baseName = p.stem().string();
    }
    auto interactive = false;
    this->tk = _cpptkinter::create(screenName, baseName, className, interactive, useTk, sync, use);
    if (detail::_debug)
        this->tk->settrace(detail::_print_command);
    if (useTk)
        this->_loadtk();
    this->readprofile(baseName, className);
}

void cpptkinter::Tk::loadtk()
{
    if (!this->_tkloaded)
    {
        this->tk->loadtk();
        this->loadtk();
    }
}

void cpptkinter::Tk::_loadtk()
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

    // Create and register the tkerror and exit commands. We need to inline parts of _register here, _ register would register differently-named commands.
    this->tk->createcommand("tkerror", detail::_tkerror);
    this->tk->createcommand("exit", detail::_exit);
    this->_tclCommands.insert("tkerror");
    this->_tclCommands.insert("exit");
    if (detail::_support_default_root && detail::_default_root.get() == nullptr)
        detail::_default_root = std::static_pointer_cast<impl>(this->pimpl);
    this->protocol("WM_DELETE_WINDOW", std::function<void()>(std::bind_front(&Tk::destroy, *this)));
}

void cpptkinter::Tk::readprofile(std::string_view baseName, std::string_view className)
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

void cpptkinter::Tk::_report_callback_exception(Tk& self, const std::exception_ptr& exc_ptr)
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

cpptkinter::Tk cpptkinter::Tcl(const std::string& screenName, const std::string& baseName, const std::string& className, bool useTk)
{
    return Tk(screenName, baseName, className, useTk);
}

void cpptkinter::Button::flash()
{
    this->tk->call(this->_w, "flash");
}

double cpptkinter::Scale::get()
{
    return this->tk->call<double>(this->_w, "get");
}

double cpptkinter::Scale::set(double value)
{
    return this->tk->call<double>(this->_w, "set", value);
}

std::array<long long, 2> cpptkinter::Scale::coords()
{
	return this->tk->call<std::array<long long, 2>>(this->_w, "coords");
}

std::array<long long, 2> cpptkinter::Scale::coords(double value)
{
    return this->tk->call<std::array<long long, 2>>(this->_w, "coords", value);
}

std::string cpptkinter::Scale::identify(detail::_ScreenUnits x, detail::_ScreenUnits y)
{
    return this->tk->call<std::string>(this->_w, "identify", x, y);
}

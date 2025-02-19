#include "_cpptkinter.hpp"

/// @file _cpptkinter.cpp
/// @brief Implements _tkinter.c, _tkinter.c.h and tkappinit.c.


cpptkinter::_cpptkinter::Tcl_Obj::Tcl_Obj(::Tcl_Obj* ptr) : ptr{ ptr }
{
	if (!this->ptr)
		throw detail::construct_exception<std::invalid_argument>("nullptr in Tcl_Obj");
	Tcl_IncrRefCount(this->ptr);
}

cpptkinter::_cpptkinter::Tcl_Obj::Tcl_Obj(const Tcl_Obj& other) noexcept : ptr{ other.ptr }
{
	Tcl_IncrRefCount(this->ptr);
}

cpptkinter::_cpptkinter::Tcl_Obj& cpptkinter::_cpptkinter::Tcl_Obj::operator=(const Tcl_Obj& other) noexcept
{
	if (this->ptr != other.ptr)
	{
		this->ptr = other.ptr;
		Tcl_IncrRefCount(this->ptr);
	}

	return *this;
}

cpptkinter::_cpptkinter::Tcl_Obj::~Tcl_Obj() noexcept
{
	using ::Tcl_Obj;
	Tcl_DecrRefCount(this->ptr);
}

::Tcl_Obj* cpptkinter::_cpptkinter::Tcl_Obj::get() const noexcept
{
	return this->ptr;
}

::Tcl_Obj* cpptkinter::_cpptkinter::Tcl_Obj::operator->() const noexcept
{
	return this->get();
}

std::string cpptkinter::_cpptkinter::Tcl_Obj::to_string() const
{
	if (!this->ptr->typePtr->name)
		throw detail::construct_exception<TclError>(std::format("Tcl_Obj->typePtr->name is nullptr (Tcl_GetString: {})", Tcl_GetString(this->ptr)));
	return std::format("<{} object: {}>", this->ptr->typePtr->name, Tcl_GetString(this->ptr));
}

cpptkinter::_cpptkinter::Tcl_Obj::operator::Tcl_Obj*() const noexcept
{
	return this->get();
}

std::string cpptkinter::_cpptkinter::detail::_get_tcl_lib_path()
{
	DEVIATING_IMPLEMENTATION_WARNING("original tries to find the tcl lib inside the python directory");
	if (_tcl_lib_path.has_value())
		return _tcl_lib_path.value();
	else
		throw construct_exception<std::runtime_error>("tcl_library must be specified with tkinter::init because");
}

void cpptkinter::_cpptkinter::detail::log_error(const std::string& message, const std::source_location location)
{
	std::cerr << "file: "
		<< location.file_name() << '('
		<< location.line() << ':'
		<< location.column() << ") in "
		<< location.function_name() << ": "
		<< message << std::endl;
}

std::string cpptkinter::_cpptkinter::detail::Tcl_Obj_to_string_impl(TkappObject* tkapp, ::Tcl_Obj* value)
{
	if (!value->typePtr)
		return std::format("(unknown type)'{}'", Tcl_GetString(value));
	else if (!value->typePtr->name)
		throw construct_exception<TclError>(std::format("Tcl_Obj->typePtr->name is nullptr (Tcl_GetString: {})", Tcl_GetString(value)));
	else
	{
		std::string result{};

		if (value->typePtr == tkapp->DictType)
		{
			Tcl_Interp* interp = Tkapp_Interp(tkapp);
			Tcl_DictSearch search{};
			::Tcl_Obj* keyPtr, * valuePtr;
			int done{};
			if (Tcl_DictObjFirst(interp, value, &search, &keyPtr, &valuePtr, &done) != TCL_OK)
				throw Tkinter_Error(tkapp);

			result += value->typePtr->name;
			result += "{ ";

			while (!done) {
				result += Tcl_Obj_to_string_impl(tkapp, keyPtr) + " : " + Tcl_Obj_to_string_impl(tkapp, valuePtr) + ", ";

				Tcl_DictObjNext(&search, &keyPtr, &valuePtr, &done);
			}

			Tcl_DictObjDone(&search);

			result += "}";
		}
		else if (value->typePtr == tkapp->ListType)
		{
			Tcl_Interp* interp = Tkapp_Interp(tkapp);
			Tcl_Size size{};
			if (Tcl_ListObjLength(interp, value, &size) == TCL_ERROR)
				throw Tkinter_Error(tkapp);

			result += value->typePtr->name;
			result += "[ ";

			for (Tcl_Size i = 0; i < size; i++)
			{
				::Tcl_Obj* tcl_elem{};
				if (Tcl_ListObjIndex(interp, value, i, &tcl_elem) == TCL_ERROR)
					throw Tkinter_Error(tkapp);
				result += Tcl_Obj_to_string_impl(tkapp, tcl_elem);
				if (i != size - 1)
					result += ", ";
			}

			result += "]";
		}
		else
		{
			result += std::format("({})'{}'", value->typePtr->name, Tcl_GetString(value));
		}
		return result;
	}
}

std::string cpptkinter::_cpptkinter::detail::Tcl_Obj_to_string(TkappObject* tkapp, const Tcl_Obj& value)
{
	return Tcl_Obj_to_string_impl(tkapp, value);
}

int cpptkinter::_cpptkinter::Tcl_AppInit(Tcl_Interp* interp)
{
	if (Tcl_Init(interp) == TCL_ERROR)
	{
		std::cerr << "Tcl_Init error: " << Tcl_GetStringResult(interp) << std::endl;
		return TCL_ERROR;
	}

	auto _tkinter_skip_tk_init = Tcl_GetVar(interp, "_tkinter_skip_tk_init", TCL_GLOBAL_ONLY);
	if (_tkinter_skip_tk_init != nullptr && _tkinter_skip_tk_init == std::string_view("1"))
		return TCL_OK;

	if (Tk_Init(interp) == TCL_ERROR)
	{
		std::cerr << "Tk_Init error: " << Tcl_GetStringResult(interp) << std::endl;
		return TCL_ERROR;
	}

	DEVIATING_IMPLEMENTATION_WARNING("original calls Tk_MainWindow(interp) if macro WITH_APPINIT is defined. i dont know why tho...");

	return TCL_OK;
}

void cpptkinter::_cpptkinter::init(const std::string& tcl_library)
{
	DEVIATING_IMPLEMENTATION_WARNING("original is called PyInit__tkinter");

	if (!tcl_library.empty())
		detail::_tcl_lib_path = tcl_library;

	tcl_lock.emplace();

	auto uexe = detail::argv[0];
	if (uexe)
	{
		auto cexe = uexe;
		if (cexe)
		{
#ifdef MS_WINDOWS
			bool set_var = false;
#if !TCL_CORE_LIBRARY_IS_EMBEDDED
			DWORD ret = GetEnvironmentVariableA("TCL_LIBRARY", NULL, 0);
			if (!ret && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
				auto str_path = detail::_get_tcl_lib_path();
				SetEnvironmentVariableA("TCL_LIBRARY", str_path.data());
				set_var = true;
			}
#endif	// !TCL_CORE_LIBRARY_IS_EMBEDDED
			Tcl_FindExecutable(cexe);

			if (set_var)
				SetEnvironmentVariableW(L"TCL_LIBRARY", NULL);
#else
			Tcl_FindExecutable(PyBytes_AS_STRING(cexe));
#endif	// MS_WINDOWS
		}
	}
}

int cpptkinter::_cpptkinter::Tcl_EvalObjv(Tcl_Interp* interp, const std::vector<Tcl_Obj>& objects, int flags)
{
	auto objs = objects | std::views::transform(&Tcl_Obj::get) | std::ranges::to<std::vector>();
	return ::Tcl_EvalObjv(interp, objs.size(), objs.data(), flags);
}

cpptkinter::_cpptkinter::Tcl_Obj cpptkinter::_cpptkinter::Tcl_NewListObj(const std::vector<Tcl_Obj>& objects)
{
	auto objs = objects | std::views::transform(&Tcl_Obj::get) | std::ranges::to<std::vector>();
	return Tcl_Obj(::Tcl_NewListObj(objs.size(), objs.data()));
}

bool cpptkinter::_cpptkinter::fromBoolean(TkappObject* tkapp, const Tcl_Obj& value)
{
	int boolValue{};
	if (Tcl_GetBooleanFromObj(Tkapp_Interp(tkapp), value, &boolValue) == TCL_ERROR)
		throw Tkinter_Error(tkapp);

	return bool(boolValue);
}

long long cpptkinter::_cpptkinter::fromWideIntObj(TkappObject* tkapp, const Tcl_Obj& value)
{
	long long wideValue;
	if (Tcl_GetWideIntFromObj(Tkapp_Interp(tkapp), value, &wideValue) == TCL_OK)
		return wideValue;

	throw Tkinter_Error(tkapp);
}

std::string cpptkinter::_cpptkinter::unicodeFromTclObj(TkappObject* tkapp, const Tcl_Obj& value)
{
	DEVIATING_IMPLEMENTATION_WARNING("original converts to and returns python unicode string using unicodeFromTclStringAndSize");
	const char* str = Tcl_GetString(value);
	if (str == nullptr)
		throw Tkinter_Error(tkapp);
	return str;
}

std::string cpptkinter::_cpptkinter::Tkapp_UnicodeResult(TkappObject* self)
{
	DEVIATING_IMPLEMENTATION_WARNING("original calls unicodeFromTclObj but we cant do that bc of infinite recursion (Tkapp_UnicodeResult->unicodeFromTclObj->Tkinter_Error->Tkapp_UnicodeResult)");

	const char* str = Tcl_GetString(Tcl_GetObjResult(self->interp));
	if (str == nullptr)
		throw detail::construct_exception<TclError>("Tcl_GetString returned nullptr");
	return str;
}

void cpptkinter::_cpptkinter::Tkapp_ThreadSend(TkappObject* self, Tcl_Event* ev, Tcl_Condition* cond, Tcl_Mutex* mutex) noexcept
{
	Tcl_MutexLock(mutex);
	Tcl_ThreadQueueEvent(self->thread_id, ev, TCL_QUEUE_TAIL);
	Tcl_ThreadAlert(self->thread_id);
	Tcl_ConditionWait(cond, mutex, NULL);
	Tcl_MutexUnlock(mutex);
}

void cpptkinter::_cpptkinter::EnableEventHook()
{
	DEVIATING_IMPLEMENTATION_WARNING("original: something with python input hook, not applicable to c++");
}

void cpptkinter::_cpptkinter::DisableEventHook()
{
	DEVIATING_IMPLEMENTATION_WARNING("see EnableEventHook() for explanation");
}

int cpptkinter::_cpptkinter::WaitForMainloop(TkappObject* self)
{
	for (int i = 0; i < 10; i++)
	{
		if (self->dispatching)
			return 1;

		Sleep(100);
	}

	if (self->dispatching)
		return 1;

	throw detail::construct_exception<std::runtime_error>("main thread is not in main loop");
}

std::optional<cpptkinter::_cpptkinter::detail::ignore> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<ignore>)
{
	return ignore{};
}

std::optional<std::string> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<std::string>)
{
	if (value->typePtr == nullptr
		|| (value->typePtr == tkapp->StringType && tkapp->StringType)
		|| (value->typePtr == tkapp->UTF32StringType && tkapp->UTF32StringType))
		return unicodeFromTclObj(tkapp, value);
	return {};
}

std::optional<bool> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<bool>)
{
	if ((value->typePtr == tkapp->BooleanType && tkapp->BooleanType)
		|| (value->typePtr == tkapp->OldBooleanType && tkapp->OldBooleanType))
		return fromBoolean(tkapp, value);
	return {};
}

std::optional<cpptkinter::_cpptkinter::byte_array> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<byte_array>)
{
	if (value->typePtr == tkapp->ByteArrayType && tkapp->ByteArrayType)
	{
		Tcl_Size size{};
		auto data = Tcl_GetByteArrayFromObj(value, &size);
		return byte_array(reinterpret_cast<std::byte*>(data), reinterpret_cast<std::byte*>(data + size));
	}
	return {};
}

std::optional<double> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<double>)
{
	if (value->typePtr == tkapp->DoubleType && tkapp->DoubleType)
		return value->internalRep.doubleValue;
	return {};
}

std::optional<long long> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<long long>)
{
	DEVIATING_IMPLEMENTATION_WARNING("original has special handling for tkapp->BignumType");

	if ((value->typePtr == tkapp->IntType && tkapp->IntType)
		|| (value->typePtr == tkapp->WideIntType && tkapp->WideIntType)
		|| (value->typePtr == tkapp->BignumType && tkapp->BignumType))
		return fromWideIntObj(tkapp, value);

	return {};
}

std::optional<cpptkinter::_cpptkinter::tk_window_type> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& ptr, std::type_identity<tk_window_type>)
{
	if (ptr->typePtr == tkapp->WindowType && tkapp->WindowType)
		return { tk_window_type(ptr) };
	return {};
}

std::optional<cpptkinter::_cpptkinter::Tcl_Obj> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& ptr, std::type_identity<Tcl_Obj>)
{
	return ptr;
}

std::shared_ptr<cpptkinter::_cpptkinter::TkappObject> cpptkinter::_cpptkinter::create(
	const std::string& screenName, const std::string& baseName, const std::string& className, bool interactive, bool wantTk, bool sync, const std::string& use)
{
	return std::make_shared<TkappObject>(screenName, className, interactive, wantTk, sync, use);
}

cpptkinter::_cpptkinter::Tcl_Obj cpptkinter::_cpptkinter::detail::AsObjImpl(const Tcl_Obj& value)
{
	return value;
}

cpptkinter::_cpptkinter::Tcl_Obj cpptkinter::_cpptkinter::detail::AsObjImpl(const byte_array& value)
{
	return Tcl_Obj(Tcl_NewByteArrayObj(reinterpret_cast<const unsigned char*>(value.data()), value.size()));
}

cpptkinter::_cpptkinter::Tcl_Obj cpptkinter::_cpptkinter::detail::AsObjImpl(double value)
{
	return Tcl_Obj(Tcl_NewDoubleObj(value));
}

cpptkinter::_cpptkinter::Tcl_Obj cpptkinter::_cpptkinter::detail::AsObjImpl(const std::string& value)
{
	return Tcl_Obj(Tcl_NewStringObj(value.data(), value.size()));
}

void cpptkinter::_cpptkinter::UnsetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, int flags)
{
	auto name1 = arg1.data();
	const char* name2 = arg2.empty() ? nullptr : arg2.data();

	if (self->trace)
	{
		if (flags & TCL_GLOBAL_ONLY)
		{
			if (name2)
				TRACE((TkappObject*)self, ("((sssN))", "uplevel", "#0", "unset", std::format("{}({})", name1, name2)));
			else
				TRACE((TkappObject*)self, ("((ssss))", "uplevel", "#0", "unset", name1));
		}
		else
		{
			if (name2)
				TRACE((TkappObject*)self, ("((sN))", "unset", std::format("{}({})", name1, name2)));
			else
				TRACE((TkappObject*)self, ("((ss))", "unset", name1));
		}
	}

	ENTER_TCL;
	int code = Tcl_UnsetVar2(Tkapp_Interp(self), name1, name2, flags);
	ENTER_OVERLAP;
	if (code == TCL_ERROR)
		throw Tkinter_Error(self);
	LEAVE_OVERLAP_TCL;
}

cpptkinter::_cpptkinter::TkappObject::TkappObject(const std::string& screenName, std::string className, int interactive, int wantTk, int sync, const std::string& use)
{
	this->interp = Tcl_CreateInterp();
	this->threaded = Tcl_GetVar2Ex(this->interp, "tcl_platform", "threaded", TCL_GLOBAL_ONLY) != nullptr;
	this->thread_id = Tcl_GetCurrentThread();

#ifndef TCL_THREADS
	if (this->threaded)
		throw detail::construct_exception<TclError>("Tcl is threaded but _tkinter is not");
#endif

	/* If Tcl is threaded, we don't need the lock. */
	if (this->threaded && tcl_lock.has_value())
		tcl_lock.reset();

	// Tcl 8.5 "booleanString" type is not registered and is renamed to "boolean" in Tcl 9.0. Based on approach suggested at https://core.tcl-lang.org/tcl/info/3bb3bcf2da5b
	auto value = Tcl_NewStringObj("true", -1);
	int boolValue{};
	Tcl_GetBooleanFromObj(NULL, value, &boolValue);
	this->BooleanType = value->typePtr;

	// "bytearray" type is not registered in Tcl 9.0
	value = Tcl_NewByteArrayObj(NULL, 0);
	this->ByteArrayType = value->typePtr;
	
	this->DoubleType = Tcl_GetObjType("double");
	/* TIP 484 suggests retrieving the "int" type without Tcl_GetObjType("int") since it is no longer registered in Tcl 9.0. But even though Tcl 8.7 
	   only uses the "wideInt" type on platforms with 32-bit long, it still has a registered "int" type, which FromObj() should recognize just in case.*/
	this->IntType = Tcl_GetObjType("int");
	if (this->IntType == nullptr)
	{
		auto value = Tcl_Obj(Tcl_NewIntObj(0));
		this->IntType = value->typePtr;
	}
	this->DictType = Tcl_GetObjType("dict");
	this->ListType = Tcl_GetObjType("list");
	this->StringType = Tcl_GetObjType("string");

	this->OldBooleanType = Tcl_GetObjType("boolean");
	this->WideIntType = Tcl_GetObjType("wideInt");
	this->BignumType = Tcl_GetObjType("bignum");
	this->UTF32StringType = Tcl_GetObjType("utf32string");


	Tcl_DeleteCommand(this->interp, "exit");

	if (!screenName.empty())
		Tcl_SetVar2(this->interp, "env", "DISPLAY", screenName.data(), TCL_GLOBAL_ONLY);

	if (interactive)
		Tcl_SetVar(this->interp, "tcl_interactive", "1", TCL_GLOBAL_ONLY);
	else
		Tcl_SetVar(this->interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

	if (std::isupper(className.at(0)))
		className.at(0) = std::toupper(className.at(0));
	Tcl_SetVar(this->interp, "argv0", className.c_str(), TCL_GLOBAL_ONLY);

	if (!wantTk)
		Tcl_SetVar(this->interp, "_tkinter_skip_tk_init", "1", TCL_GLOBAL_ONLY);

	// some initial arguments need to be in argv
	if (sync || !use.empty())
	{
		std::string args{};
		if (sync)
			args += "-sync";
		if (!use.empty())
		{
			if (sync)
				args += " ";
			args += "-use ";
			args += use;
		}

		Tcl_SetVar(this->interp, "argv", args.c_str(), TCL_GLOBAL_ONLY);
	}

#ifdef MS_WINDOWS
#if !TCL_CORE_LIBRARY_IS_EMBEDDED
	DWORD ret = GetEnvironmentVariableA("TCL_LIBRARY", NULL, 0);
	if (!ret && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
	{
		auto str_path = detail::_get_tcl_lib_path();
		Tcl_SetVar(this->interp, "tcl_library", str_path.c_str(), TCL_GLOBAL_ONLY);
	}
#endif	// !TCL_CORE_LIBRARY_IS_EMBEDDED
#endif	// MS_WINDOWS

	if (Tcl_AppInit(this->interp) != TCL_OK)
		throw Tkinter_Error(this);

	// get the "window" type ptr
	this->WindowType = Tcl_GetObjType("window");

	EnableEventHook();
}

decltype(cpptkinter::_cpptkinter::TkappObject::trace)& cpptkinter::_cpptkinter::TkappObject::gettrace()
{
	return this->trace;
}

void cpptkinter::_cpptkinter::TkappObject::loadtk()
{
	auto self = this;

	Tcl_Interp* interp = Tkapp_Interp(self);
	const char* _tk_exists;

	/* We want to guard against calling Tk_Init() multiple times */
	CHECK_TCL_APPARTMENT;
	ENTER_TCL;
	int err = Tcl_Eval(Tkapp_Interp(self), "info exists     tk_version");
	ENTER_OVERLAP;
	if (err == TCL_ERROR)
		throw Tkinter_Error(self);
	else
		_tk_exists = Tcl_GetStringResult(Tkapp_Interp(self));
	
	LEAVE_OVERLAP_TCL;

	if (_tk_exists == nullptr || _tk_exists != std::string_view("1"))
		if (Tk_Init(interp) == TCL_ERROR)
			throw Tkinter_Error(self);
}

const char* cpptkinter::_cpptkinter::varname_converter(const std::string& arg)
{
	return arg.data();
}

const char* cpptkinter::_cpptkinter::varname_converter(const Tcl_Obj& arg)
{
	return Tcl_GetString(arg);
}

void cpptkinter::_cpptkinter::TkappObject::unsetvar(const std::string& arg1, const std::string& arg2)
{
	var_invoke([&]() { UnsetVar(this, arg1, arg2, TCL_LEAVE_ERR_MSG); }, this);
}

void cpptkinter::_cpptkinter::TkappObject::globalunsetvar(const std::string& arg1, const std::string& arg2)
{
	var_invoke([&]() { UnsetVar(this, arg1, arg2, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
}

std::vector<std::string> cpptkinter::_cpptkinter::TkappObject::splitlist(const std::string& s)
{
	auto self = this;
	Tcl_Size argc{};
	const char** argv{};

	auto del = [](const char*** argv) { ckfree(*argv); };
	std::unique_ptr<const char**, decltype(del)> ptr{ nullptr, del };

	if (Tcl_SplitList(Tkapp_Interp(self), s.c_str(), &argc, &argv) == TCL_ERROR)
		throw Tkinter_Error(self);

	ptr.reset(&argv);

	return std::vector<std::string>(argv, argv + argc);
}

void cpptkinter::_cpptkinter::TkappObject::deletecommand(const std::string& name)
{
	auto self = this;
	int err = 0;

	TRACE(self, ("((sss))", "rename", name, ""));

	if (self->threaded && self->thread_id != Tcl_GetCurrentThread())
	{
		auto buffer = (CommandEvent*)attemptckalloc(sizeof(CommandEvent));
		if (buffer == nullptr)
			throw detail::construct_exception<TclError>("attemptckalloc failed to allocate memory");

		Tcl_Condition cond = NULL;
		auto&& ev = *std::construct_at(static_cast<CommandEvent*>(buffer));

		// have to specify PythonCmd_ClientData<...>:: here because of how createcommand works
		ev.proc = PythonCmd_ClientData<void>::Tkapp_CommandProc;
		ev.interp = self->interp;
		ev.create = 0;
		ev.name = name;
		ev.status = &err;
		ev.done = &cond;
		Tkapp_ThreadSend(self, &ev, &cond, &command_mutex);
		Tcl_ConditionFinalize(&cond);
	}
	else
	{
		ENTER_TCL;
		err = Tcl_DeleteCommand(self->interp, name.data());
		LEAVE_TCL;
	}

	if (err == -1)
		throw detail::construct_exception<TclError>(std::format("can't delete Tcl command '{}'", name));
}

void cpptkinter::_cpptkinter::TkappObject::mainloop(int threshold)
{
	auto self = this;

	CHECK_TCL_APPARTMENT;
	self->dispatching = 1;

	quitMainLoop = 0;
	while (Tk_GetNumMainWindows() > threshold && !quitMainLoop && !errorInCmd)
	{
		int result;

		if (self->threaded)
		{
			// Allow other Python threads to run.
			ENTER_TCL;
			result = Tcl_DoOneEvent(0);
			LEAVE_TCL;
		}
		else
		{
			{
				auto mutex_adapter = utility::optional_mutex_adaptor(tcl_lock);
				auto lock = std::scoped_lock(mutex_adapter);

				result = Tcl_DoOneEvent(TCL_DONT_WAIT);
			}

			if (result == 0)
				Sleep(Tkinter_busywaitinterval);
		}

		DEVIATING_IMPLEMENTATION_WARNING("original handles python signals with PyErr_CheckSignals");

		if (result < 0)
			break;
	}
	self->dispatching = 0;
	quitMainLoop = 0;

	if (errorInCmd)
	{
		errorInCmd = 0;
		auto intermediate = excInCmd;
		excInCmd = nullptr;
		std::rethrow_exception(intermediate);
	}
}

void cpptkinter::_cpptkinter::TkappObject::quit()
{
	quitMainLoop = 1;
}

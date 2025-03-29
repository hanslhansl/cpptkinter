module;
#include "../global.hpp"
#include <tk.h>
#ifdef _WIN32
#include <windows.h>
#endif
export module cpptkinter:_cpptkinter.functions;
import std;
import :utility;


constexpr auto _TK_VERSION = TK_VERSION;
constexpr auto _TCL_VERSION = TCL_VERSION;
#undef TK_VERSION
#undef TCL_VERSION

/// @brief Implementation of the Python module _tkinter in C++.
export namespace cpptkinter::_cpptkinter
{
	constexpr std::string_view TK_VERSION = _TK_VERSION;
	constexpr std::string_view TCL_VERSION = _TCL_VERSION;
	constexpr auto READABLE = TCL_READABLE;
	constexpr auto WRITABLE = TCL_WRITABLE;
	constexpr auto EXCEPTION = TCL_EXCEPTION;
	constexpr auto WINDOW_EVENTS = TCL_WINDOW_EVENTS;
	constexpr auto FILE_EVENTS = TCL_FILE_EVENTS;
	constexpr auto TIMER_EVENTS = TCL_TIMER_EVENTS;
	constexpr auto IDLE_EVENTS = TCL_IDLE_EVENTS;
	constexpr auto ALL_EVENTS = TCL_ALL_EVENTS;
	constexpr auto DONT_WAIT = TCL_DONT_WAIT;

	std::optional<std::mutex> tcl_lock;
	Tcl_Mutex var_mutex;
	Tcl_Mutex call_mutex;
	Tcl_Mutex command_mutex;
	DEVIATING_IMPLEMENTATION_WARNING("TCL_DECLARE_MUTEX(..._mutex);");

	int quitMainLoop = 0;
	int errorInCmd = 0;
	std::exception_ptr excInCmd{};
	int Tkinter_busywaitinterval = 20;

	class Tcl_Obj;

	/// @brief Exception class for Tcl errors.
	struct TclError : std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
}

export namespace cpptkinter::detail
{
	using namespace _cpptkinter;

	struct TkappObjectImpl
	{
		Tcl_Interp* interp;
		//int wantobjects; deprecated, always true

		/// true if tcl_platform[threaded]
		int threaded;
		Tcl_ThreadId thread_id;
		int dispatching{};
		std::function<void(std::vector<std::string>)> trace{};
		// We cannot include tclInt.h, as this is internal. So we cache interesting types here.
		const Tcl_ObjType* BooleanType;
		const Tcl_ObjType* ByteArrayType;
		const Tcl_ObjType* DoubleType;
		const Tcl_ObjType* IntType;
		const Tcl_ObjType* ListType;
		const Tcl_ObjType* DictType;
		const Tcl_ObjType* StringType;
		const Tcl_ObjType* WindowType;
		const Tcl_ObjType* ParsedVarName;
		[[deprecated]] const Tcl_ObjType* OldBooleanType;
		[[deprecated]] const Tcl_ObjType* WideIntType;
		[[deprecated]] const Tcl_ObjType* BignumType;
		[[deprecated]] const Tcl_ObjType* UTF32StringType;
	};

	template<typename Func>
		requires requires { std::packaged_task<Func>{}; }
	struct TclBaseEvent : Tcl_Event
	{
		std::packaged_task<Func> task;
		Tcl_Condition* cond;
		//int flags;

		TclBaseEvent(Tcl_EventProc* proc, std::packaged_task<Func>&& task, Tcl_Condition* cond) : Tcl_Event{ proc }, task{ std::move(task) }, cond{ cond }
		{

		}
	};

	std::optional<std::string> _tcl_lib_path{};
	std::string _get_tcl_lib_path()
	{
		DEVIATING_IMPLEMENTATION_WARNING("original tries to find the tcl lib inside the python directory");
		if (_tcl_lib_path.has_value())
			return _tcl_lib_path.value();
		else
			throw utility::construct_exception<std::runtime_error>("tcl_library must be specified with tkinter::init because");
	}

	void log_error(const std::string& message, const std::source_location location = std::source_location::current())
	{
		std::cerr << "file: "
			<< location.file_name() << '('
			<< location.line() << ':'
			<< location.column() << ") in "
			<< location.function_name() << ": "
			<< message << std::endl;
	}

	template<typename T>
	std::string Tkapp_Trace_to_string(const T& t)
	{
		if constexpr (std::formattable<T, char>)
			return std::format("{}", t);

		// convertible to string
		else if constexpr (std::convertible_to<T, std::string>)
			return t;

		// stringstream
		else if constexpr (requires { std::ostringstream() << t; })
			return (std::ostringstream() << t).str();

		else
			return typeid(t).name();
	}
}

using TkappObjectImpl = cpptkinter::detail::TkappObjectImpl;

export namespace cpptkinter::_cpptkinter
{
	int Tcl_AppInit(Tcl_Interp* interp)
	{
		if (Tcl_Init(interp) != TCL_OK)
		{
			std::cerr << "Tcl_Init error: " << Tcl_GetStringResult(interp) << std::endl;
			return TCL_ERROR;
		}

		auto _tkinter_skip_tk_init = Tcl_GetVar(interp, "_tkinter_skip_tk_init", TCL_GLOBAL_ONLY);
		if (_tkinter_skip_tk_init != nullptr && _tkinter_skip_tk_init == std::string_view("1"))
			return TCL_OK;

		if (Tk_Init(interp) != TCL_OK)
		{
			std::cerr << "Tk_Init error: " << Tcl_GetStringResult(interp) << std::endl;
			return TCL_ERROR;
		}

		DEVIATING_IMPLEMENTATION_WARNING("original calls Tk_MainWindow(interp) if macro WITH_APPINIT is defined. i dont know why tho...");

		return TCL_OK;
	}

	void init(const std::string& executable_path, const std::string& tcl_library)
	{
		DEVIATING_IMPLEMENTATION_WARNING("original is called PyInit__tkinter");

		if (!tcl_library.empty())
			detail::_tcl_lib_path = tcl_library;

		tcl_lock.emplace();

#ifdef _WIN32
		bool set_var = false;
		if constexpr (!TCL_CORE_LIBRARY_IS_EMBEDDED)
		{
			DWORD ret = GetEnvironmentVariableA("TCL_LIBRARY", NULL, 0);
			if (!ret && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
				auto str_path = detail::_get_tcl_lib_path();
				SetEnvironmentVariableA("TCL_LIBRARY", str_path.data());
				set_var = true;
			}
		}
		Tcl_FindExecutable(executable_path.data());

		if (set_var)
			SetEnvironmentVariableW(L"TCL_LIBRARY", NULL);
#else
		Tcl_FindExecutable(executable_path.data());
#endif	// _WIN32
	}

	std::string Tkapp_UnicodeResult(TkappObjectImpl* self)
	{
		DEVIATING_IMPLEMENTATION_WARNING("original calls unicodeFromTclObj but we cant do that bc of infinite recursion (Tkapp_UnicodeResult->unicodeFromTclObj->Tkinter_Error->Tkapp_UnicodeResult)");

		const char* str = Tcl_GetString(Tcl_GetObjResult(self->interp));
		if (str == nullptr)
			throw utility::construct_exception<TclError>("Tcl_GetString returned nullptr");
		return str;
	}

#if !defined(NDEBUG) && defined(__cpp_lib_stacktrace)
	TclError Tkinter_Error(TkappObjectImpl* self, const std::stacktrace& tr = std::stacktrace::current())
	{
		return utility::construct_exception<TclError>(Tkapp_UnicodeResult(self), tr);
	}
#else
	TclError Tkinter_Error(TkappObjectImpl* self)
	{
		return utility::construct_exception<TclError>(Tkapp_UnicodeResult(self));
	}
#endif

	int WaitForMainloop(TkappObjectImpl* self)
	{
		for (int i = 0; i < 10; i++)
		{
			if (self->dispatching)
				return 1;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (self->dispatching)
			return 1;

		throw utility::construct_exception<std::runtime_error>("main thread is not in main loop");
	}

	void EnableEventHook()
	{
		DEVIATING_IMPLEMENTATION_WARNING("original: something with python input hook, not applicable to c++");
	}
	void DisableEventHook()
	{
		DEVIATING_IMPLEMENTATION_WARNING("see EnableEventHook() for explanation");
	}

	void Tkapp_ThreadSend(TkappObjectImpl* self, Tcl_Event* ev, Tcl_Condition* cond, Tcl_Mutex* mutex) noexcept
	{
		Tcl_MutexLock(mutex);
		Tcl_ThreadQueueEvent(self->thread_id, ev, TCL_QUEUE_TAIL);
		Tcl_ThreadAlert(self->thread_id);
		Tcl_ConditionWait(cond, mutex, NULL);
		Tcl_MutexUnlock(mutex);
	}

	void Tkapp_Trace(TkappObjectImpl* self, const auto&...args)
	{
		if (self->trace)
		{
			std::vector<std::string> ret{};
			(ret.emplace_back(detail::Tkapp_Trace_to_string(args)), ...);
			self->trace(std::move(ret));
		}
	}
}

export namespace cpptkinter::detail
{
	std::string Tcl_obj_type_string(::Tcl_Obj* value)
	{
		if (!value)
			return "nullptr";
		else if (!value->typePtr)
			return "unknown type";
		else if (!value->typePtr->name)
			return "unknown type";
		else
			return value->typePtr->name;
	}

	std::string Tcl_Obj_to_string(TkappObjectImpl* self, ::Tcl_Obj* value)
	{
		auto type_string = Tcl_obj_type_string(value);

		if (!value)
			throw utility::construct_exception<TclError>(std::format("Tcl_Obj* is nullptr"));
		else if (!value->typePtr)
			return std::format("(unknown type)'{}'", Tcl_GetString(value));
		else if (!value->typePtr->name)
			throw utility::construct_exception<TclError>(std::format("Tcl_Obj->typePtr->name is nullptr (Tcl_GetString: {})", Tcl_GetString(value)));
		else
		{
			std::string result{};

			if (value->typePtr == self->DictType)
			{
				Tcl_Interp* interp = self->interp;
				Tcl_DictSearch search{};
				::Tcl_Obj* keyPtr, * valuePtr;
				int done{};
				if (Tcl_DictObjFirst(interp, value, &search, &keyPtr, &valuePtr, &done) != TCL_OK)
					throw Tkinter_Error(self);

				result += type_string;
				result += "{ ";

				while (!done) {
					result += Tcl_Obj_to_string(self, keyPtr) + " : " + Tcl_Obj_to_string(self, valuePtr) + ", ";

					Tcl_DictObjNext(&search, &keyPtr, &valuePtr, &done);
				}

				Tcl_DictObjDone(&search);

				result += "}";
			}
			else if (value->typePtr == self->ListType)
			{
				Tcl_Interp* interp = self->interp;
				Tcl_Size size{};
				if (Tcl_ListObjLength(interp, value, &size) == TCL_ERROR)
					throw Tkinter_Error(self);

				result += type_string;
				result += "[ ";

				for (Tcl_Size i = 0; i < size; i++)
				{
					::Tcl_Obj* tcl_elem{};
					if (Tcl_ListObjIndex(interp, value, i, &tcl_elem) == TCL_ERROR)
						throw Tkinter_Error(self);
					result += Tcl_Obj_to_string(self, tcl_elem);
					if (i != size - 1)
						result += ", ";
				}

				result += "]";
			}
			else
			{
				result += std::format("({})'{}'", type_string, Tcl_GetString(value));
			}
			return result;
		}
	}
}
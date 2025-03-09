/// @file _cpptkinter.ixx
/// @brief Implements _tkinter.c, _tkinter.c.h and tkappinit.c.

module;
#include "global.hpp"
#include <reflect/reflect.hpp>
#include <tk.h>

#ifdef _WIN32
#define USE_TCL_UNICODE 1
#include <windows.h>
//#include <conio.h>
#define WAIT_FOR_STDIN
#else
#define USE_TCL_UNICODE 0
#endif
export module cpptkinter:_cpptkinter;
import :utility;
//import :tk;
import std;
import hhh;


using namespace std::literals;
static_assert(std::same_as<Tcl_WideInt, long long>);

/// If Tcl is compiled for threads, we must also define TCL_THREAD. We define it always; if Tcl is not threaded, the thread functions in Tcl are empty.
#define TCL_THREADS

#define ENTER_TCL				{ auto _opt_mutex_adapter = utility::optional_mutex_adaptor(tcl_lock); auto _temp_tcl_lock = std::scoped_lock(_opt_mutex_adapter)
#define LEAVE_TCL				}
#define ENTER_OVERLAP			// nothing
#define LEAVE_OVERLAP_TCL		}

#define ENTER_PYTHON			{ auto _opt_inv_mutex_adapter = utility::optional_inverse_mutex_adaptor(tcl_lock); auto _temp_tcl_inv_lock = std::scoped_lock(_opt_inv_mutex_adapter)
#define LEAVE_PYTHON			}

//#define CHECK_TCL_APPARTMENT	NOT_IMPLEMENTED_ERROR

#define CHECK_TCL_APPARTMENT  if (self->threaded && self->thread_id != Tcl_GetCurrentThread()) throw detail::construct_exception<std::runtime_error>("Calling Tcl from different apartment")

#define Tkapp_Interp(v) (((v))->interp)

#define Py_BuildValue(fmt_str, ...) __VA_ARGS__
#define TRACE(_self, ARGS) do {                 \
        if ((_self)->trace) {  \
            Tkapp_Trace((_self), Py_BuildValue ARGS);   \
        }   \
    } while (0)


/// @brief Implementation of the Python module tkinter in C++.
export namespace cpptkinter
{
	class Misc;
}

/// @brief Implementation of the Python module _tkinter in C++.
export namespace cpptkinter::_cpptkinter
{
	struct TkappObject;

	using byte_array = std::vector<std::byte>;
	using ssize_t = std::make_signed<size_t>::type;

	/// @brief Represents a Tcl object.
	class Tcl_Obj
	{
		::Tcl_Obj* ptr;

	public:
		explicit Tcl_Obj(::Tcl_Obj* ptr);
		Tcl_Obj(const Tcl_Obj& other) noexcept : ptr{ other.ptr }
		{
			Tcl_IncrRefCount(this->ptr);
		}
		Tcl_Obj& operator=(const Tcl_Obj& other) noexcept
		{
			if (this->ptr != other.ptr)
			{
				this->ptr = other.ptr;
				Tcl_IncrRefCount(this->ptr);
			}

			return *this;
		}
		~Tcl_Obj() noexcept
		{
			using ::Tcl_Obj;
			Tcl_DecrRefCount(this->ptr);
		}

		::Tcl_Obj* get() const noexcept
		{
			return this->ptr;
		}
		::Tcl_Obj* operator->() const noexcept
		{
			return this->get();
		}
		operator ::Tcl_Obj* () const noexcept
		{
			return this->get();
		}

		std::string _repr_() const;
		std::string to_string() const
		{
			return Tcl_GetString(this->ptr);
		}
	};

	/// @brief Represents a tcl object of type 'window' which is a type introduced by Tk.
	struct tk_window_type : Tcl_Obj
	{
		using Tcl_Obj::Tcl_Obj;
	};

	/// @brief Exception class for Tcl errors.
	struct TclError : std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};
}

/// @brief Implementation details of _cpptkinter.
/// 
/// This namespace contains implementation details of _cpptkinter as well as the implementation of _tkinter entities prefixed with an underscore (e.g. _tkinter._get_tcl_lib_path()).
export namespace cpptkinter::_cpptkinter::detail
{
	constexpr auto _TK_VERSION = TK_VERSION;
	constexpr auto _TCL_VERSION = TCL_VERSION;
#undef TK_VERSION
#undef TCL_VERSION

#if !defined(NDEBUG) && defined(__cpp_lib_stacktrace)
	template<typename T>
	T construct_exception(const std::string& str, const std::stacktrace& tr = std::stacktrace::current())
	{
		return T(std::format("{}\nat:\n{}", str, tr));
	}
#else
	template<typename T>
	T construct_exception(const std::string& str, const std::source_location& loc = std::source_location::current())
	{
		return T(std::format("{}\nin file {}, at line {} in function {}", str, loc.file_name(), loc.line(), loc.function_name()));
	}
#endif

	template<typename Func>
		requires requires { typename std::packaged_task<Func>; }
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
			throw construct_exception<std::runtime_error>("tcl_library must be specified with tkinter::init because");
	}

	/// Set by cpptkinter::init()
	int argc;
	/// Set by cpptkinter::init()
	char** argv;

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
		// Tcl_Obj
		if constexpr (std::same_as<T, Tcl_Obj>)
			return t.to_string();

		// string
		if constexpr (std::same_as<T, std::string>)
			return t;

		// string_view
		else if constexpr (std::same_as<T, std::string_view>)
			return std::string(t);

		// convertible to string
		else if constexpr (std::convertible_to<T, std::string>)
			return t;

		// to_string
		else if constexpr (requires { std::to_string(t); })
			return std::to_string(t);

		// pointer
		else if constexpr (std::is_pointer_v<T>)
		{
			if constexpr (std::same_as<T, char*>)
				return t;
			else if constexpr (std::same_as<T, const char*>)
				return t;
			else
				return std::string("pointer to ") + typeid(t).name();
		}

		// range or tuple
		else if constexpr (requires { utility::range_or_tuple_to_string(t); })
			return utility::range_or_tuple_to_string(t);

		// stringstream
		else if constexpr (requires { std::ostringstream() << t; })
			return (std::ostringstream() << t).str();

		else
			return typeid(t).name();
	}


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
	std::string Tcl_Obj_to_string(TkappObject* tkapp, ::Tcl_Obj* obj);

	template<typename T>
	struct AsObjImplTrait;

	/// @brief Try to convert a Tcl_Obj to Tcl_Obj (i.e. copy it).
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	Tcl_Obj AsObjImpl(const Tcl_Obj& value)
	{
		return value;
	}
	/// @brief Try to convert a byte_array to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	Tcl_Obj AsObjImpl(const byte_array& value)
	{
		return Tcl_Obj(Tcl_NewByteArrayObj(reinterpret_cast<const unsigned char*>(value.data()), value.size()));
	}
	/// @brief Try to convert a double to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	Tcl_Obj AsObjImpl(double value)
	{
		return Tcl_Obj(Tcl_NewDoubleObj(value));
	}
	/// @brief Try to convert a std::string to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	Tcl_Obj AsObjImpl(const std::string& value)
	{
		return Tcl_Obj(Tcl_NewStringObj(value.data(), value.size()));
	}
	/// @brief Try to convert an integral type to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<std::integral T>
	Tcl_Obj AsObjImpl(const T& value)
	{
		DEVIATING_IMPLEMENTATION_WARNING("original differentiates between long and long long, we treat all as long long");
		DEVIATING_IMPLEMENTATION_WARNING("python's int can represent more values than long long, in case of overflow the original handles it as tcl bignum");
		if constexpr (std::same_as<T, bool>)
			return Tcl_Obj(Tcl_NewBooleanObj(value));
		else
			return Tcl_Obj(Tcl_NewWideIntObj(value));
	}
	/// @brief Try to convert a std::shared_ptr<Misc> (or derived) to Tcl_Obj using the widget's string representation.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<std::derived_from<Misc> T>
	Tcl_Obj AsObjImpl(const std::shared_ptr<T>& value);
	/// @brief Try to convert a std::variant to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<typename...Args>
		requires (AsObjImplTrait<Args>::value && ...)
	Tcl_Obj AsObjImpl(const std::variant<Args...>& value);
	/// @brief Try to convert a tuple-like or a container to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<typename T>
		requires (hhh::meta::tuple_like<T> && hhh::meta::tuple_elements_satisfy<T, AsObjImplTrait>::value)
	|| (utility::is_vector<T> && AsObjImplTrait<typename T::value_type>::value)
		Tcl_Obj AsObjImpl(const T& value);
	/// @brief Try to convert a std::reference_wrapper or cpptkinter::utility::ref_wrapper to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<typename T>
		requires AsObjImplTrait<typename T::type>::value
	&& (hhh::meta::is_template_instance<T, std::reference_wrapper> || hhh::meta::is_template_instance<T, utility::ref_wrapper>)
		Tcl_Obj AsObjImpl(const T& value);

	template<typename T>
	struct AsObjImplTrait : std::bool_constant<requires { AsObjImpl(std::declval<T>()); }>{ };

	struct ignore {};

	template<typename T>
	struct FromObjImplTrait;

	/// @brief Do nothing with Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<ignore> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<ignore>)
	{
		return ignore{};
	}
	/// @brief Try to convert Tcl_Obj to std::string.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<std::string> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<std::string>);
	/// @brief Try to convert Tcl_Obj to bool.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<bool> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<bool>);
	/// @brief Try to convert Tcl_Obj to byte_array.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<byte_array> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<byte_array>);
	/// @brief Try to convert Tcl_Obj to double.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<double> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<double>);
	/// @brief Try to convert Tcl_Obj to long long.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<long long> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<long long>);
	/// @brief Try to convert Tcl_Obj to tk_window_type.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<tk_window_type> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& ptr, std::type_identity<tk_window_type>);
	/// @brief Try to convert Tcl_Obj to Tcl_Obj (i.e. do nothing).
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<Tcl_Obj> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& ptr, std::type_identity<Tcl_Obj>);
	/// @brief Try to convert Tcl_Obj to std::vector.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<utility::is_vector T>
		requires FromObjImplTrait<typename T::value_type>::value
	std::optional<T> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj to std::map.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<hhh::meta::is_template_instance<std::map> T>
		requires FromObjImplTrait<typename T::key_type>::value&& FromObjImplTrait<typename T::mapped_type>::value
	std::optional<T> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj to tuple-like.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<typename T>
		requires (hhh::meta::tuple_like<T>&& hhh::meta::tuple_elements_satisfy<T, FromObjImplTrait>::value)
	std::optional<T> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj to std::variant.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<typename...Args>
		requires ((FromObjImplTrait<Args>::value && ...) && sizeof...(Args) != 0)
	std::optional<std::variant<Args...>> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<std::variant<Args...>>);

	template<typename T>
	struct FromObjImplTrait : std::bool_constant<requires { FromObjImpl({}, std::declval<Tcl_Obj>(), std::type_identity<T>{}); }> {};


	/// @brief The constraint for the argument type of cpptkinter::_cpptkinter::AsObj().
	/// 
	/// T is intended to come from const T& as AsObjImpl doesn't make use of rvalues.
	/// This concept is satisfied if there exists an overload of cpptkinter::_cpptkinter::detail::AsObjImpl() for type T.
	template<typename T>
	concept AsObjConcept = AsObjImplTrait<T>::value;

	/// @brief The constraint for the return type of cpptkinter::_cpptkinter::FromObj().
	/// 
	/// This concept is satisfied if 
	/// 1. there exists an overload of cpptkinter::_cpptkinter::detail::FromObjImpl() for type R.
	/// or 
	/// 2. R is void (trivial case).
	template<typename R>
	concept FromObjConcept = std::same_as<R, void> || FromObjImplTrait<R>::value;

	template<typename T>
	concept range_of_Tcl_Obj = std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, Tcl_Obj>;

	template<typename T>
	concept call_argument_concept = range_of_Tcl_Obj<T> || AsObjConcept<T>;

	template<typename...Args>
	concept PythonCmd_ClientDataArgsConcept1 = std::invocable<std::function<void(Args...)>, std::remove_cvref_t<Args>...> && (FromObjConcept<std::remove_cvref_t<Args>> && ...);
	template<typename T>
	concept PythonCmd_ClientDataArgsConcept2Impl = std::invocable<std::function<void(T)>, std::vector<Tcl_Obj>&&>&& std::same_as<std::remove_cvref_t<T>, std::vector<Tcl_Obj>>;
	template<typename...Args>
	concept PythonCmd_ClientDataArgsConcept2 = sizeof...(Args) == 1 && (PythonCmd_ClientDataArgsConcept2Impl<Args> && ...);

	/// @brief The concept for cpptkinter::_cpptkinter::PythonCmd_ClientData type parameter Args.
	/// 
	/// Specifies that std::function<void(Args...)> can be invoked with 
	/// 1. cpptkinter::_cpptkinter::FromObj<std::remove_cvref_t<Args>>()...\n 
	/// or
	/// 2. std::vector<Tcl_Obj>&&.
	template<typename...Args>
	concept PythonCmd_ClientDataArgsConcept = PythonCmd_ClientDataArgsConcept1<Args...> || PythonCmd_ClientDataArgsConcept2<Args...>;

	/// @brief The concept for cpptkinter::_cpptkinter::PythonCmd_ClientData type parameter R.
	/// 
	/// Specifies that
	/// 1. R can be passed to cpptkinter::_cpptkinter::AsObj()\n 
	/// or
	/// 2. R is void.
	template<typename R>
	concept PythonCmd_ClientDataReturnConcept = std::same_as<R, void> || AsObjConcept<R>;
}

export namespace cpptkinter::_cpptkinter
{
	constexpr auto READABLE = TCL_READABLE;
	constexpr auto WRITABLE = TCL_WRITABLE;
	constexpr auto EXCEPTION = TCL_EXCEPTION;
	constexpr auto WINDOW_EVENTS = TCL_WINDOW_EVENTS;
	constexpr auto FILE_EVENTS = TCL_FILE_EVENTS;
	constexpr auto TIMER_EVENTS = TCL_TIMER_EVENTS;
	constexpr auto IDLE_EVENTS = TCL_IDLE_EVENTS;
	constexpr auto ALL_EVENTS = TCL_ALL_EVENTS;
	constexpr auto DONT_WAIT = TCL_DONT_WAIT;
	constexpr std::string_view TK_VERSION = detail::_TK_VERSION;
	constexpr std::string_view TCL_VERSION = detail::_TCL_VERSION;

	std::optional<std::mutex> tcl_lock;
	Tcl_Mutex var_mutex;
	Tcl_Mutex call_mutex;
	Tcl_Mutex command_mutex;
	DEVIATING_IMPLEMENTATION_WARNING("TCL_DECLARE_MUTEX(..._mutex);");

	int quitMainLoop = 0;
	int errorInCmd = 0;
	std::exception_ptr excInCmd{};
	int Tkinter_busywaitinterval = 20;

#ifndef _WIN32
	/// @brief Millisecond Sleep() for Unix platforms.
	void Sleep(int milli)
	{
		// XXX Too bad if you don't have select().
		struct timeval t;
		t.tv_sec = milli / 1000;
		t.tv_usec = (milli % 1000) * 1000;
		select(0, (fd_set*)0, (fd_set*)0, (fd_set*)0, &t);
	}
#endif // _WIN32

	int Tcl_AppInit(Tcl_Interp* interp)
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

	void init(const std::string& tcl_library)
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
				Tcl_FindExecutable(cexe);

				if (set_var)
					SetEnvironmentVariableW(L"TCL_LIBRARY", NULL);
#else
				Tcl_FindExecutable(PyBytes_AS_STRING(cexe));
#endif	// _WIN32
			}
		}
	}

	int Tcl_EvalObjv(Tcl_Interp* interp, const std::vector<Tcl_Obj>& objects, int flags)
	{
		auto objs = objects | std::views::transform(&Tcl_Obj::get) | std::ranges::to<std::vector>();
		return ::Tcl_EvalObjv(interp, objs.size(), objs.data(), flags);
	}

	Tcl_Obj Tcl_NewListObj(const std::vector<Tcl_Obj>& objects)
	{
		auto objs = objects | std::views::transform(&Tcl_Obj::get) | std::ranges::to<std::vector>();
		return Tcl_Obj(::Tcl_NewListObj(objs.size(), objs.data()));
	}

	std::string Tkapp_UnicodeResult(TkappObject* self);

#if !defined(NDEBUG) && defined(__cpp_lib_stacktrace)
	TclError Tkinter_Error(TkappObject* self, const std::stacktrace& tr = std::stacktrace::current())
	{
		return detail::construct_exception<TclError>(Tkapp_UnicodeResult(self), tr);
	}
#else
	TclError Tkinter_Error(TkappObject* self)
	{
		return detail::construct_exception<TclError>(Tkapp_UnicodeResult(self));
	}
#endif

	/// @brief Convert a Tcl_Obj to a bool.
	///
	/// @param value The Tcl_Obj to convert.
	bool fromBoolean(TkappObject* tkapp, const Tcl_Obj& value);
	/// @brief Convert a Tcl_Obj to a long long.
	///
	/// @param value The Tcl_Obj to convert.
	long long fromWideIntObj(TkappObject* tkapp, const Tcl_Obj& value);

	/// @brief Convert a Tcl_Obj to a string.
	///
	/// @param value The Tcl_Obj to convert.
	std::string unicodeFromTclObj(TkappObject* tkapp, const Tcl_Obj& value)
	{
		DEVIATING_IMPLEMENTATION_WARNING("original converts to and returns python unicode string using unicodeFromTclStringAndSize");
		const char* str = Tcl_GetString(value);
		if (str == nullptr)
			throw Tkinter_Error(tkapp);
		return str;
	}

	void Tkapp_ThreadSend(TkappObject* self, Tcl_Event* ev, Tcl_Condition* cond, Tcl_Mutex* mutex) noexcept;
	void EnableEventHook()
	{
		DEVIATING_IMPLEMENTATION_WARNING("original: something with python input hook, not applicable to c++");
	}
	void DisableEventHook()
	{
		DEVIATING_IMPLEMENTATION_WARNING("see EnableEventHook() for explanation");
	}
	int WaitForMainloop(TkappObject* self);

	std::shared_ptr<TkappObject> create(const std::string& screenName = {}, const std::string& baseName = {}, const std::string& className = "Tk",
		bool interactive = false, bool wantTk = true, bool sync = false, const std::string& use = "")
	{
		return std::make_shared<TkappObject>(screenName, className, interactive, wantTk, sync, use);
	}

	void Tkapp_Trace(TkappObject* self, const auto&...args);

	/// @brief Convert a c++ value to a tcl object.
	///
	/// This function is used to convert a value to a Tcl_Obj. Throws if the conversion fails.
	/// @param value The c++ value to convert.
	/// @return A Tcl_Obj representing the value.
	Tcl_Obj AsObj(const detail::AsObjConcept auto& value)
	{
		return detail::AsObjImpl(value);
	}

	/// @brief Convert a tcl object to a c++ value.
	/// 
	/// @tparam T Specifies the return type. Constrained by cpptkinter::_cpptkinter::detail::FromObjConcept.
	/// @param ptr The Tcl_Obj to convert. If this functions succeeds and the return value is a smart pointer (e.g. tk_window_type) which points to **ptr**,
	/// its reference count is incremented once (and decremented once whenever the smart pointer gets destroyed). Otherwise **ptr's** reference count will not be modified.
	/// @return A c++ value of type R.
	template<detail::FromObjConcept R>
	R FromObj(TkappObject* tkapp, const Tcl_Obj& ptr)
	{
		if constexpr (std::same_as<R, void>)
			return;
		else
		{
			auto opt_result = detail::FromObjImpl(tkapp, ptr, std::type_identity<R>{});
			if (opt_result.has_value())
				return std::move(*opt_result);

			std::string error_string = std::format("Got tcl object {}.\nExpected c++ type {}.", detail::Tcl_Obj_to_string(tkapp, ptr), reflect::type_name<R>());
			throw detail::construct_exception<TclError>(error_string);
		}
	}

	/// @brief Retrieve the current value of a tcl variable.
	template<detail::FromObjConcept R>
	R GetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, int flags);
	/// @copydoc GetVar
	template<detail::FromObjConcept R>
	R GetVar(TkappObject* self, const Tcl_Obj& arg1, const std::string& arg2, int flags);

	/// @brief Modify the value of a tcl variable. If the variable doesn't exist, it will be created.
	void SetVar(TkappObject* self, const std::string& arg1, const detail::AsObjConcept auto& arg2, int flags);
	/// @brief Modify the value of a tcl variable. If the variable doesn't exist, it will be created.
	void SetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, const detail::AsObjConcept auto& arg3, int flags);

	void UnsetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, int flags);

	template<typename Func>
		requires requires { typename std::packaged_task<Func>; }
	using Tkapp_CallEvent = detail::TclBaseEvent<Func>;

	/// @brief Convert c++ values to a std::vector<Tcl_Obj>.
	template<detail::call_argument_concept...Args>
	std::vector<Tcl_Obj> Tkapp_CallArgs(Args&&...args)
	{
		DEVIATING_IMPLEMENTATION_WARNING("major changes, original ignores None and any argument thereafter, not implementable in c++");

		std::vector<Tcl_Obj> raii{};

		auto lambda = [&raii]<typename T>(T && arg)
		{
			if constexpr (detail::range_of_Tcl_Obj<T>)
				raii.append_range(std::forward<T>(arg));
			else
				raii.emplace_back(AsObj(std::forward<T>(arg)));
		};

		(lambda(std::forward<Args>(args)), ...);

		return raii;
	}

	/// @brief Retrieve the interpreter's current result.
	template<detail::FromObjConcept R>
	R Tkapp_ObjectResult(TkappObject* self);

	/// @brief Execute a tcl command represented by args.
	template<detail::FromObjConcept R, detail::call_argument_concept...Args>
	R Tkapp_CallProc(TkappObject* self, int flags, Args&&...args);

	template<typename T>
	int call_proc(Tcl_Event* evPtr, int flags) noexcept
	{
		auto&& e = *static_cast<Tkapp_CallEvent<T>*>(evPtr);
		Tcl_Condition* cond = e.cond;
		try
		{
			e.task();
		}
		catch (const std::future_error& ex)
		{
			detail::log_error(ex.what());
		}
		catch (...)
		{
			detail::log_error("Unknown exception");
		}
		std::destroy_at(&e);
		Tcl_MutexLock(&call_mutex);
		Tcl_ConditionNotify(cond);
		Tcl_MutexUnlock(&call_mutex);
		return 1;
	}

	template<typename Func>
		requires requires { typename std::packaged_task<Func>; }
	using VarEvent = detail::TclBaseEvent<Func>;

	const char* varname_converter(const std::string& arg)
	{
		return arg.data();
	}
	const char* varname_converter(const Tcl_Obj& arg)
	{
		return Tcl_GetString(arg);
	}

	template<typename T>
	int var_proc(Tcl_Event* evPtr, int flags) noexcept
	{
		auto&& ev = *static_cast<VarEvent<T>*>(evPtr);
		Tcl_Condition* cond = ev.cond;
		ENTER_PYTHON;
		try
		{
			ev.task();
		}
		catch (const std::future_error& e)
		{
			detail::log_error(e.what());
		}
		catch (...)
		{
			detail::log_error("Unknown exception");
		}
		std::destroy_at(&ev);
		Tcl_MutexLock(&var_mutex);
		Tcl_ConditionNotify(cond);
		Tcl_MutexUnlock(&var_mutex);
		LEAVE_PYTHON;
		return 1;
	}

	template<typename Func>
	std::invoke_result_t<Func&&> var_invoke(Func&& func, TkappObject* self);

	struct CommandEvent : Tcl_Event
	{
		Tcl_Interp* interp;
		std::string name;
		int create;
		int* status;
		ClientData data;
		Tcl_Condition* done;
	};

	/// @brief A wrapper allowing for the contained c++ callback to be called from/through tcl.
	/// 
	/// Constrained by detail::PythonCmd_ClientDataReturnConcept and detail::PythonCmd_ClientDataArgsConcept. Used by TkappObject::createcommand().
	/// @tparam R The return type of the command.
	/// @tparam Args The argument types of the command.
	template<typename R, typename...Args>
		requires detail::PythonCmd_ClientDataReturnConcept<R>&& detail::PythonCmd_ClientDataArgsConcept<Args...>
	struct PythonCmd_ClientData
	{
		std::function<R(Args...)> func;
		TkappObject* self;
#ifndef NDEBUG 
		std::string name;
#endif

		std::string get_error_string_header(::Tcl_Obj* objv0)
		{
			auto tcl_invocation_name = Tcl_Obj(objv0).to_string();
			std::string error_string = std::format("In {}", tcl_invocation_name);
#ifndef NDEBUG 
			error_string += std::format(" ({})", this->name);
#endif
			return error_string;
		}

		void PythonCmdImpl(Tcl_Interp* interp, int objc, ::Tcl_Obj* const objv[])
		{
			if (objc < 1)
				throw detail::construct_exception<TclError>("this should never get triggered. objc should never be < 1 but is " + std::to_string(objc));

			auto args = std::ranges::subrange(objv + 1, objv + objc) | std::views::transform([](const auto& a) { return Tcl_Obj(a); }) | std::ranges::to<std::vector>();

			if constexpr (detail::PythonCmd_ClientDataArgsConcept2<Args...>)
				;//pass
			else
			{
				if (sizeof...(Args) != args.size())
				{
					std::string error_string = std::format("{}\ngot {} tcl arguments:", this->get_error_string_header(objv[0]), args.size());
					for (auto&& arg : args)
						error_string += "\n\t- " + detail::Tcl_Obj_to_string(this->self, arg);
					error_string += std::format("\nexpected {} c++ arguments:", sizeof...(Args));
					error_string += ("" + ... + ("\n\t- " + std::string(reflect::type_name<Args>())));
					throw detail::construct_exception<TclError>(error_string);
				}
			}

			ENTER_PYTHON;

			auto inner_caller = [&]<typename T, size_t I>() {
				auto opt_result = detail::FromObjImpl(this->self, args[I], std::type_identity<std::remove_cvref_t<T>>{});
				if (opt_result.has_value())
					return std::move(*opt_result);

				std::string error_string = std::format("{}\n{}. tcl argument was {}\nxpected c++ argument {}",
					this->get_error_string_header(objv[0]),
					I+1,
					detail::Tcl_Obj_to_string(this->self, args[I]),
					reflect::type_name<T>());
				throw detail::construct_exception<TclError>(error_string);
			};

			// necessary bc c++ doesn't define function argument evaluation order :(
			auto caller = [&]<size_t...I>(std::index_sequence<I...>) {
				if constexpr (detail::PythonCmd_ClientDataArgsConcept2<Args...>)	// pass std::vector<Tcl_Obj>
					this->func(std::move(args));
				else	// pass c++ args
					return this->func(inner_caller.template operator() < Args, I > ()...);
			};

			if constexpr (std::same_as<R, void>)
			{
				caller(std::make_index_sequence<sizeof...(Args)>{});
				Tcl_ResetResult(interp);
			}
			else
			{
				auto obj_res = AsObj(caller(std::make_index_sequence<sizeof...(Args)>{}));
				Tcl_SetObjResult(interp, obj_res);
			}

			LEAVE_PYTHON;
		}

		static int PythonCmd(ClientData clientData, Tcl_Interp* interp, int objc, ::Tcl_Obj* const objv[]) noexcept
		{
			try
			{
				static_cast<PythonCmd_ClientData*>(clientData)->PythonCmdImpl(interp, objc, objv);
			}
			catch (...)
			{
				errorInCmd = 1;
				excInCmd = std::current_exception();
				return TCL_ERROR;
			}

			return TCL_OK;
		}

		/// @brief Call delete on this.
		static void PythonCmdDelete(ClientData clientData) noexcept
		{
			delete static_cast<PythonCmd_ClientData*>(clientData);
		}

		static int Tkapp_CommandProc(Tcl_Event* evPtr, int flags) noexcept
		{
			auto&& ev = *static_cast<CommandEvent*>(evPtr);
			if (ev.create)
				*ev.status = Tcl_CreateObjCommand(ev.interp, ev.name.data(), PythonCmd_ClientData::PythonCmd, ev.data, PythonCmd_ClientData::PythonCmdDelete) == nullptr;
			else
				*ev.status = Tcl_DeleteCommand(ev.interp, ev.name.data());
			auto done = ev.done;
			std::destroy_at(&ev);
			Tcl_MutexLock(&command_mutex);
			Tcl_ConditionNotify(done);
			Tcl_MutexUnlock(&command_mutex);
			return 1;
		}
	};

	namespace detail
	{
		template<typename Func>
		auto new_PythonCmd_ClientData(Func&& func, TkappObject* self, const std::string& name) -> decltype(PythonCmd_ClientData{ std::forward<Func>(func) })*
		{
			return new decltype(PythonCmd_ClientData{ std::function(std::forward<Func>(func)) }){
				std::forward<Func>(func),
				self
#ifndef NDEBUG 
				,name
#endif
			};
		}

		template<typename Func>
		concept createcommand_concept = requires { utility::callable_to_std_function(std::declval<Func>()); };
	}

	/// @brief This class allows running Tcl code. Cpptkinter uses it internally a lot.
	struct TkappObject
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

		//TkappObject() = default;
		TkappObject(const std::string& screenName, std::string className, int interactive, int wantTk, int sync, const std::string& use)
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

#ifdef _WIN32
			if constexpr (!TCL_CORE_LIBRARY_IS_EMBEDDED)
			{
				DWORD ret = GetEnvironmentVariableA("TCL_LIBRARY", NULL, 0);
				if (!ret && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
				{
					auto str_path = detail::_get_tcl_lib_path();
					Tcl_SetVar(this->interp, "tcl_library", str_path.c_str(), TCL_GLOBAL_ONLY);
				}
			}
#endif	// _WIN32

			if (Tcl_AppInit(this->interp) != TCL_OK)
				throw Tkinter_Error(this);

			// get the "window" type ptr
			this->WindowType = Tcl_GetObjType("window");

			EnableEventHook();
		}

		void wilddispatch();
		//void wantobjects();

		template<typename Func>
			requires requires (Func&& func) { trace = std::forward<Func>(func); }
		void settrace(Func&& func)
		{
			this->trace = std::forward<Func>(func);
		}
		decltype(trace)& gettrace()
		{
			return this->trace;
		}

		/// @brief This is the main entry point for calling a Tcl command.
		/// 
		/// In tkinter, if an argument is None, it and any following arguments are ignored. This feature isn't implemented here because c++ doesn't really have None.
		/// 
		/// There are three cases, with regard to threading:
		/// 1. Tcl is not threaded:
		///		Must have the Tcl lock, then can invoke command in the context of the calling thread.
		/// 2. Tcl is threaded, caller of the command is in the interpreter thread:
		/// 	Execute the command in the calling thread. Since the Tcl lock will not be used, we can merge that with case 1.
		/// 3. Tcl is threaded, caller is in a different thread:
		///		Must queue an event to the interpreter thread. Allocation of Tcl objects needs to occur in the interpreter thread,
		///		so we ship the PyObject* args to the target thread, and perform processing there.
		template<detail::FromObjConcept R = void, detail::call_argument_concept...Args>
		R call(Args&&...args)
		{
			DEVIATING_IMPLEMENTATION_WARNING("major changes");

			auto self = this;
			int flags = TCL_EVAL_DIRECT | TCL_EVAL_GLOBAL;

			if (self->threaded && self->thread_id != Tcl_GetCurrentThread())
			{
				hhh::misc::printl("call if 1");
				DEVIATING_IMPLEMENTATION_WARNING("original: flags isnt actually passed, probably a bug in tkinter");
				auto lambda = [&]() { return Tkapp_CallProc<R>(this, flags, std::forward<Args>(args)...); };
				using FuncType = std::invoke_result_t<decltype(lambda)>();
				using CallEventType = Tkapp_CallEvent<FuncType>;

				if (!WaitForMainloop(self))
					throw detail::construct_exception<std::runtime_error>("impossible");

				auto buffer = attemptckalloc(sizeof(CallEventType));
				if (buffer == nullptr)
					throw detail::construct_exception<TclError>("attemptckalloc failed to allocate memory");

				Tcl_Condition cond = NULL;
				auto&& ev = *std::construct_at(static_cast<CallEventType*>(buffer), call_proc<FuncType>, std::packaged_task(std::move(lambda)), &cond);
				auto future = ev.task.get_future();

				Tkapp_ThreadSend(self, &ev, ev.cond, &call_mutex);
				Tcl_ConditionFinalize(&cond);

				ANNOTATION_WARNING("maybe the usage of Tcl_Condition and mutex is unnecessary because future.get() waits anyways");
				return future.get();
			}
			else
			{
				TRACE(self, ("(O)", args...));
				auto raii = Tkapp_CallArgs(std::forward<Args>(args)...);

				ENTER_TCL;

				auto i = Tcl_EvalObjv(self->interp, raii, flags);

				ENTER_OVERLAP;

				if (i == TCL_ERROR)
					throw Tkinter_Error(self);
				else
					return Tkapp_ObjectResult<R>(self);

				LEAVE_OVERLAP_TCL;

				DEVIATING_IMPLEMENTATION_WARNING("original calls Tkapp_CallDeallocArgs, raii's dtor handles that for us");
			}
		}
		void eval();
		void evalfile();
		void record();
		void adderrorinfo();

		void setvar(const std::string& arg1, const detail::AsObjConcept auto& arg2)
		{
			var_invoke([&]() { SetVar(this, arg1, arg2, TCL_LEAVE_ERR_MSG); }, this);
		}
		void setvar(const std::string& arg1, const std::string& arg2, const detail::AsObjConcept auto& arg3)
		{
			var_invoke([&]() { SetVar(this, arg1, arg2, arg3, TCL_LEAVE_ERR_MSG); }, this);
		}
		void globalsetvar(const std::string& arg1, const detail::AsObjConcept auto& arg2)
		{
			var_invoke([&]() { SetVar(this, arg1, arg2, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}
		void globalsetvar(const std::string& arg1, const std::string& arg2, const detail::AsObjConcept auto& arg3)
		{
			var_invoke([&]() { SetVar(this, arg1, arg2, arg3, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}
		template<detail::FromObjConcept R>
		R getvar(const std::string& arg1, const std::string& arg2 = {})
		{
			return var_invoke([&]()->decltype(auto) { return GetVar<R>(this, arg1, arg2, TCL_LEAVE_ERR_MSG); }, this);
		}
		template<detail::FromObjConcept R>
		R getvar(const Tcl_Obj& arg1, const std::string& arg2 = {})
		{
			return var_invoke([&]()->decltype(auto) { return GetVar<R>(this, arg1, arg2, TCL_LEAVE_ERR_MSG); }, this);
		}
		template<detail::FromObjConcept R>
		R globalgetvar(const std::string& arg1, const std::string& arg2 = {})
		{
			return var_invoke([&]()->decltype(auto) { return GetVar<R>(this, arg1, arg2, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}
		template<detail::FromObjConcept R>
		R globalgetvar(const Tcl_Obj& arg1, const std::string& arg2 = {})
		{
			return var_invoke([&]()->decltype(auto) { return GetVar<R>(this, arg1, arg2, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}
		void unsetvar(const std::string& arg1, const std::string& arg2 = {})
		{
			var_invoke([&]() { UnsetVar(this, arg1, arg2, TCL_LEAVE_ERR_MSG); }, this);
		}
		void globalunsetvar(const std::string& arg1, const std::string& arg2 = {})
		{
			var_invoke([&]() { UnsetVar(this, arg1, arg2, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}

		void getint();
		void getdouble();
		bool getboolean();

		void exprstring();
		void exprlong();
		void exprdouble();
		void exprboolean();
		std::vector<std::string> splitlist(const std::string& s)
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
		template<detail::createcommand_concept Func>
		void createcommand(const std::string& name, Func&& func)
		{
			DEVIATING_IMPLEMENTATION_WARNING("merges _tkinter_tkapp_createcommand and _tkinter_tkapp_createcommand_impl");

			auto self = this;

			if (self->threaded && self->thread_id != Tcl_GetCurrentThread() && !WaitForMainloop(self))
				throw detail::construct_exception<TclError>("an unknown error occured");

			TRACE(self, ("((ss()O))", "proc", name, func));

			auto data = detail::new_PythonCmd_ClientData(utility::callable_to_std_function(std::forward<Func>(func)), self, name);
			DEVIATING_IMPLEMENTATION_WARNING("in the original data 'holds a reference' to self keeping it from being destructed. i doubt that this is actually necessary");

			if (self->threaded && self->thread_id != Tcl_GetCurrentThread())
			{
				auto buffer = (CommandEvent*)attemptckalloc(sizeof(CommandEvent));
				if (buffer == nullptr)
					throw detail::construct_exception<TclError>("attemptckalloc failed to allocate memory");

				int err = 0;
				Tcl_Condition cond = NULL;
				auto&& ev = *std::construct_at(static_cast<CommandEvent*>(buffer));

				ev.proc = data->Tkapp_CommandProc;
				ev.interp = self->interp;
				ev.create = 1;
				ev.name = name;
				ev.data = data;
				ev.status = &err;
				ev.done = &cond;
				Tkapp_ThreadSend(self, &ev, &cond, &command_mutex);
				Tcl_ConditionFinalize(&cond);

				if (err)
					throw detail::construct_exception<TclError>("can't create Tcl command");
			}
			else
			{
				ENTER_TCL;
				if (Tcl_CreateObjCommand(Tkapp_Interp(self), name.data(), data->PythonCmd, data, data->PythonCmdDelete) == nullptr)
					throw detail::construct_exception<TclError>("can't create Tcl command");
				LEAVE_TCL;
			}
		}
		void deletecommand(const std::string& name)
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
		void createfilehandler();
		void deletefilehandler();
		void createtimerhandler();
		void mainloop(int threshold = 0)
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
		void dooneevent();
		void quit()
		{
			quitMainLoop = 1;
		}
		void interpaddr();
		void loadtk()
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
	};
}

////////////////////////////////
// function template definitions
////////////////////////////////

namespace cpptkinter::_cpptkinter::detail
{
	template<std::derived_from<Misc> T>
	Tcl_Obj AsObjImpl(const std::shared_ptr<T>& value)
	{
		return AsObjImpl(value->operator std::string());
	}
	template<typename...Args>
		requires (AsObjImplTrait<Args>::value && ...)
	Tcl_Obj AsObjImpl(const std::variant<Args...>& value)
	{
		return std::visit([]<typename T2>(const T2 & arg) { return AsObjImpl(arg); }, value);
	}
	template<typename T>
		requires (hhh::meta::tuple_like<T>&& hhh::meta::tuple_elements_satisfy<T, AsObjImplTrait>::value)
	|| (utility::is_vector<T> && AsObjImplTrait<typename T::value_type>::value)
		Tcl_Obj AsObjImpl(const T & value)
	{
		std::vector<Tcl_Obj> raii{};
		utility::visit_range_or_tuple([&raii]<typename T2>(const T2 & elem) { raii.emplace_back(AsObjImpl(elem)); }, value);
		return Tcl_NewListObj(raii);
	}
	template<typename T>
		requires AsObjImplTrait<typename T::type>::value
	&& (hhh::meta::is_template_instance<T, std::reference_wrapper> || hhh::meta::is_template_instance<T, utility::ref_wrapper>)
		Tcl_Obj AsObjImpl(const T& value)
	{
		return AsObjImpl(value.get());
	}

	template<typename T>
	T FromObjImplListQuery(Tcl_Interp* interp, TkappObject* tkapp, const Tcl_Obj& value, Tcl_Size i)
	{
		::Tcl_Obj* tcl_elem{};
		if (Tcl_ListObjIndex(interp, value, i, &tcl_elem) == TCL_ERROR)
			throw Tkinter_Error(tkapp);

		return FromObj<T>(tkapp, Tcl_Obj(tcl_elem));
	}

	template<utility::is_vector T>
		requires FromObjImplTrait<typename T::value_type>::value
	std::optional<T> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<T>)
	{
		using value_type = typename T::value_type;

		if (value->typePtr == nullptr)
			return std::optional<T>{ std::in_place };
		else if (value->typePtr == tkapp->ListType && tkapp->ListType)
		{
			Tcl_Interp* interp = Tkapp_Interp(tkapp);
			Tcl_Size size{};
			if (Tcl_ListObjLength(interp, value, &size) == TCL_ERROR)
				throw Tkinter_Error(tkapp);

			std::optional<T> result{ std::in_place };
			auto&& vec = *result;
			vec.reserve(size);
			for (Tcl_Size i = 0; i < size; i++)
				vec.emplace_back(FromObjImplListQuery<value_type>(interp, tkapp, value, i));

			return result;
		}
		return {};
	}
	template<hhh::meta::is_template_instance<std::map> T>
		requires FromObjImplTrait<typename T::key_type>::value&& FromObjImplTrait<typename T::mapped_type>::value
	std::optional<T> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<T>)
	{
		using key_type = typename T::key_type;
		using mapped_type = typename T::mapped_type;

		if (value->typePtr == nullptr)
			return std::optional<T>{ std::in_place };
		else if (value->typePtr == tkapp->DictType && tkapp->DictType)
		{
			std::optional<T> result{ std::in_place };
			auto&& map = *result;

			Tcl_Interp* interp = Tkapp_Interp(tkapp);
			Tcl_DictSearch search{};
			::Tcl_Obj* keyPtr, * valuePtr;
			int done{};

			if (Tcl_DictObjFirst(interp, value, &search, &keyPtr, &valuePtr, &done) != TCL_OK)
				throw Tkinter_Error(tkapp);

			while (!done)
			{
				auto&& [it, success] = map.emplace(FromObj<key_type>(tkapp, keyPtr), FromObj<mapped_type>(tkapp, valuePtr));
				if (!success)
					throw detail::construct_exception<TclError>("duplicate key in dict");

				Tcl_DictObjNext(&search, &keyPtr, &valuePtr, &done);
			}

			Tcl_DictObjDone(&search);
			return result;
		}
		return {};
	}
	template<typename T>
		requires (hhh::meta::tuple_like<T>&& hhh::meta::tuple_elements_satisfy<T, FromObjImplTrait>::value)
	std::optional<T> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<T>)
	{
		if (std::tuple_size_v<T> == 0 && value->typePtr == nullptr)
			return std::optional<T>{ std::in_place };
		else if (value->typePtr == tkapp->ListType && tkapp->ListType)
		{
			Tcl_Interp* interp = Tkapp_Interp(tkapp);
			Tcl_Size size{};
			if (Tcl_ListObjLength(interp, value, &size) == TCL_ERROR)
				throw Tkinter_Error(tkapp);

			if (size != std::tuple_size_v<T>)
				throw detail::construct_exception<TclError>(std::format("expected {} elements but got {}", std::tuple_size_v<T>, size));

			return[&]<size_t...I>(std::index_sequence<I...>) {
				return T{ FromObjImplListQuery<std::tuple_element_t<I, T>>(interp, tkapp, value, I)... };
			} (std::make_index_sequence<std::tuple_size_v<T>>{});
		}
		return {};
	}
	template<typename...Args>
		requires ((FromObjImplTrait<Args>::value && ...) && sizeof...(Args) != 0)
	std::optional<std::variant<Args...>> FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<std::variant<Args...>>)
	{
		auto lambda = [&]<typename First, typename...Other>(auto & self) -> std::optional<std::variant<Args...>>
		{
			auto intermediate = FromObjImpl(tkapp, value, std::type_identity<First>{});
			if (intermediate.has_value())
				return std::move(*intermediate);
			else if constexpr (sizeof...(Other) != 0)
				return self.template operator() < Other... > (self);
			else
				return {};
		};

		return lambda.template operator() < Args... > (lambda);
	}
}

cpptkinter::_cpptkinter::Tcl_Obj::Tcl_Obj(::Tcl_Obj* ptr) : ptr{ ptr }
{
	if (!this->ptr)
		throw detail::construct_exception<std::invalid_argument>("nullptr in Tcl_Obj");
	Tcl_IncrRefCount(this->ptr);
}

template<cpptkinter::_cpptkinter::detail::FromObjConcept R>
R cpptkinter::_cpptkinter::GetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, int flags)
{
	const char* name1 = varname_converter(arg1);
	const char* name2 = arg2.empty() ? nullptr : arg2.data();

	ENTER_TCL;
	auto tres = Tcl_GetVar2Ex(Tkapp_Interp(self), name1, name2, flags);
	ENTER_OVERLAP;
	if (tres == NULL)
		throw Tkinter_Error(self);
	else
		return FromObj<R>(self, Tcl_Obj(tres));

	LEAVE_OVERLAP_TCL;
}
template<cpptkinter::_cpptkinter::detail::FromObjConcept R>
R cpptkinter::_cpptkinter::GetVar(TkappObject* self, const Tcl_Obj& arg1, const std::string& arg2, int flags)
{
	const char* name1 = varname_converter(arg1);
	const char* name2 = arg2.empty() ? nullptr : arg2.data();

	ENTER_TCL;
	auto tres = Tcl_GetVar2Ex(Tkapp_Interp(self), name1, name2, flags);
	ENTER_OVERLAP;
	if (tres == NULL)
		throw Tkinter_Error(self);
	else
		return FromObj<R>(self, Tcl_Obj(tres));

	LEAVE_OVERLAP_TCL;
}

void cpptkinter::_cpptkinter::SetVar(TkappObject* self, const std::string& arg1, const detail::AsObjConcept auto& arg2, int flags)
{
	auto name1 = arg1.data();

	if (flags & TCL_GLOBAL_ONLY)
		TRACE(self, ("((ssssO))", "uplevel", "#0", "set", name1, arg2));
	else
		TRACE(self, ("((ssO))", "set", name1, arg2));

	// XXX Acquire tcl lock ? ? ?
	auto newval = AsObj(arg2);

	ENTER_TCL;
	auto ok = Tcl_SetVar2Ex(Tkapp_Interp(self), name1, nullptr, newval, flags);
	ENTER_OVERLAP;
	if (!ok)
		throw Tkinter_Error(self);
	LEAVE_OVERLAP_TCL;
}
void cpptkinter::_cpptkinter::SetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, const detail::AsObjConcept auto& arg3, int flags)
{
	auto name1 = arg1.data();
	auto name2 = arg2.data();

	// XXX must hold tcl lock already ? ? ?
	if (self->trace)
	{
		auto fmt_str = std::format("{}({})", name1, name2);
		if (flags & TCL_GLOBAL_ONLY)
			TRACE(self, ("((sssNO))", "uplevel", "#0", "set", fmt_str, arg3));
		else
			TRACE(self, ("((sNO))", "set", fmt_str, arg3));
	}

	auto newval = AsObj(arg3);

	ENTER_TCL;
	auto ok = Tcl_SetVar2Ex(Tkapp_Interp(self), name1, name2, newval, flags);
	ENTER_OVERLAP;
	if (!ok)
		throw Tkinter_Error(self);
	LEAVE_OVERLAP_TCL;
}

template<cpptkinter::_cpptkinter::detail::FromObjConcept R>
R cpptkinter::_cpptkinter::Tkapp_ObjectResult(TkappObject* self)
{
	auto ptr = Tcl_Obj(Tcl_GetObjResult(self->interp));
	return FromObj<R>(self, ptr);
}

template<cpptkinter::_cpptkinter::detail::FromObjConcept R, cpptkinter::_cpptkinter::detail::call_argument_concept...Args>
R cpptkinter::_cpptkinter::Tkapp_CallProc(TkappObject* self, int flags, Args&&...args)
{
	std::vector<Tcl_Obj> raii{};

	ENTER_PYTHON;
	Tkapp_Trace(self, args...);
	raii = Tkapp_CallArgs(std::forward<Args>(args)...);
	LEAVE_PYTHON;

	auto i = Tcl_EvalObjv(self->interp, raii, flags);

	ENTER_PYTHON;
	if (i == TCL_ERROR)
		throw Tkinter_Error(self);
	else
		return Tkapp_ObjectResult<R>(self);
	LEAVE_PYTHON;

	DEVIATING_IMPLEMENTATION_WARNING("original calls Tkapp_CallDeallocArgs, raii's dtor handles that for us");
}

std::string cpptkinter::_cpptkinter::Tcl_Obj::_repr_() const
{
	if (!this->ptr->typePtr->name)
		throw detail::construct_exception<TclError>(std::format("Tcl_Obj->typePtr->name is nullptr (Tcl_GetString: {})", Tcl_GetString(this->ptr)));
	return std::format("<{} object: {}>", this->ptr->typePtr->name, Tcl_GetString(this->ptr));
}

void cpptkinter::_cpptkinter::Tkapp_Trace(TkappObject* self, const auto&...args)
{
	if (self->trace)
	{
		std::vector<std::string> ret{};
		(ret.emplace_back(detail::Tkapp_Trace_to_string(args)), ...);
		self->trace(std::move(ret));
	}
}

template<typename Func>
std::invoke_result_t<Func&&> cpptkinter::_cpptkinter::var_invoke(Func&& func, TkappObject* self)
{
	DEVIATING_IMPLEMENTATION_WARNING("major changes");

	if (self->threaded && self->thread_id != Tcl_GetCurrentThread())
	{
		using FuncType = std::invoke_result_t<Func&&>();
		using VarEventType = VarEvent<FuncType>;

		// The current thread is not the interpreter thread. Marshal the call to the interpreter thread, then wait for completion.
		if (!WaitForMainloop(self))
			throw detail::construct_exception<std::runtime_error>("impossible");

		auto buffer = attemptckalloc(sizeof(VarEventType));
		if (buffer == NULL)
			throw detail::construct_exception<TclError>("attemptckalloc failed to allocate memory");

		Tcl_Condition cond = NULL;
		auto&& ev = *std::construct_at(static_cast<VarEventType*>(buffer), var_proc<FuncType>, std::packaged_task(std::move(func)), &cond);
		auto future = ev.task.get_future();

		Tkapp_ThreadSend(self, &ev, ev.cond, &var_mutex);
		Tcl_ConditionFinalize(ev.cond);

		ANNOTATION_WARNING("maybe the usage of Tcl_Condition and mutex is unnecessary because future.get() waits anyways");
		return future.get();
	}
	else
	{
		return func();
	}
}

std::string cpptkinter::_cpptkinter::detail::Tcl_Obj_to_string(TkappObject* tkapp, ::Tcl_Obj* value)
{
	auto type_string = Tcl_obj_type_string(value);

	if (!value)
		throw construct_exception<TclError>(std::format("Tcl_Obj* is nullptr"));
	else if (!value->typePtr)
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

			result += type_string;
			result += "{ ";

			while (!done) {
				result += Tcl_Obj_to_string(tkapp, keyPtr) + " : " + Tcl_Obj_to_string(tkapp, valuePtr) + ", ";

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

			result += type_string;
			result += "[ ";

			for (Tcl_Size i = 0; i < size; i++)
			{
				::Tcl_Obj* tcl_elem{};
				if (Tcl_ListObjIndex(interp, value, i, &tcl_elem) == TCL_ERROR)
					throw Tkinter_Error(tkapp);
				result += Tcl_Obj_to_string(tkapp, tcl_elem);
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

bool cpptkinter::_cpptkinter::fromBoolean(TkappObject* tkapp, const Tcl_Obj& value)
{
	int boolValue{};
	if (Tcl_GetBooleanFromObj(Tkapp_Interp(tkapp), value, &boolValue) == TCL_ERROR)
		throw Tkinter_Error(tkapp);

	return bool(boolValue);
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

long long cpptkinter::_cpptkinter::fromWideIntObj(TkappObject* tkapp, const Tcl_Obj& value)
{
	long long wideValue;
	if (Tcl_GetWideIntFromObj(Tkapp_Interp(tkapp), value, &wideValue) == TCL_OK)
		return wideValue;

	throw Tkinter_Error(tkapp);
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

std::optional<std::string> cpptkinter::_cpptkinter::detail::FromObjImpl(TkappObject* tkapp, const Tcl_Obj& value, std::type_identity<std::string>)
{
	if (value->typePtr == nullptr
		|| (tkapp->StringType && value->typePtr == tkapp->StringType)
		|| (tkapp->UTF32StringType && value->typePtr == tkapp->UTF32StringType)
		|| (value->typePtr && value->typePtr->name && value->typePtr->name == "parsedVarName"sv)
		)
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


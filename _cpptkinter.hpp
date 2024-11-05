#pragma once
#include "global.hpp"
#include "utility.hpp"
#include "constants.hpp"


/// @file _cpptkinter.hpp
/// @brief Implements _tkinter.c, _tkinter.c.h and tkappinit.c.


/// If Tcl is compiled for threads, we must also define TCL_THREAD. We define it always; if Tcl is not threaded, the thread functions in Tcl are empty.
#define TCL_THREADS

#ifdef MS_WINDOWS
#define USE_TCL_UNICODE 1
#include <windows.h>
//#include <conio.h>
#define WAIT_FOR_STDIN
#else
#define USE_TCL_UNICODE 0
#endif

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




namespace cpptkinter::_cpptkinter
{
	namespace detail
	{
		struct Tcl_Obj_deleter
		{
			void operator()(Tcl_Obj* ptr) const noexcept;
		};
		using Tcl_Obj_Raii = std::unique_ptr<Tcl_Obj, Tcl_Obj_deleter>;
	}

	using byte_array = std::vector<std::byte>;
	using ssize_t = std::make_signed<size_t>::type;

	struct TkappObject;

	/// @brief Exception class for Tcl errors.
	struct TclError : std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	/// @brief Represents a tcl object of type 'window' which is a type introduced by tk.
	struct tk_window_type : detail::Tcl_Obj_Raii
	{
		using detail::Tcl_Obj_Raii::Tcl_Obj_Raii;
	};
}

/// @brief Implementation details of _cpptkinter.
/// 
/// This namespace contains implementation details of _cpptkinter as well as the implementation of _tkinter entities prefixed with an underscore (e.g. _tkinter._get_tcl_lib_path()).
namespace cpptkinter::_cpptkinter::detail
{
	constexpr auto _TK_VERSION = TK_VERSION;
	constexpr auto _TCL_VERSION = TCL_VERSION;
#undef TK_VERSION
#undef TCL_VERSION

	inline std::optional<std::string> _tcl_lib_path{};
	std::string _get_tcl_lib_path();

	/// Set by cpptkinter::init()
	inline int argc;
	/// Set by cpptkinter::init()
	inline char** argv;

	void log_error(const std::string_view message, const std::source_location location = std::source_location::current());

	template<typename T>
	std::string Tkapp_Trace_to_string(const T& t)
	{
		// string like
		if constexpr (std::same_as<T, std::string>)
			return t;
		else if constexpr (std::same_as<T, std::string_view>)
			return std::string(t);

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

		// stringstream
		else if constexpr (requires { std::ostringstream() << t; })
			return (std::ostringstream() << t).str();

		else
			return typeid(t).name();
	}

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

#ifdef NDEBUG
	template<typename T>
	T construct_exception(const std::string& str)
	{
		return T(str);
	}
#else
	template<typename T>
	T construct_exception(const std::string& str, const std::stacktrace& tr = std::stacktrace::current())
	{
		return T("'" + str + "' at:\n" + std::to_string(tr));
	}
#endif // NDEBUG

	std::string Tcl_Obj_to_string(TkappObject* tkapp, Tcl_Obj* obj);

	/// @brief RAII wrapper for std::vector of Tcl_Obj*.
	class Tcl_Obj_vector_raii
	{
		std::vector<Tcl_Obj*> vec;
	public:
		Tcl_Obj_vector_raii() = default;
		Tcl_Obj_vector_raii(Tcl_Obj_vector_raii&&) = default;
		Tcl_Obj_vector_raii(const Tcl_Obj_vector_raii&) = delete;
		Tcl_Obj_vector_raii& operator=(Tcl_Obj_vector_raii&&) = default;
		Tcl_Obj_vector_raii& operator=(const Tcl_Obj_vector_raii&) = delete;

		~Tcl_Obj_vector_raii() noexcept;

		void emplace_back(Tcl_Obj* obj);
		void append(Tcl_Obj_vector_raii other);

		int Tcl_EvalObjv(Tcl_Interp* interp, int flags);

		Tcl_Obj* Tcl_NewListObj();
	};

	template<typename T>
	struct AsObjImplTrait;

	/// @brief Try to convert a byte_array to Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	Tcl_Obj* AsObjImpl(const byte_array& value);
	/// @brief Try to convert a double to Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	Tcl_Obj* AsObjImpl(const double& value);
	/// @brief Try to convert a std::string to Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	Tcl_Obj* AsObjImpl(const std::string& value);
	/// @brief Try to convert an integral type to Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<std::integral T>
	Tcl_Obj* AsObjImpl(const T& value)
	{
		DEVIATING_IMPLEMENTATION_WARNING("original differentiates between long and long long, we treat all as long long");
		DEVIATING_IMPLEMENTATION_WARNING("python's int can represent more values than long long, in case of overflow the original handles it as tcl bignum");
		if constexpr (std::same_as<T, bool>)
			return Tcl_NewBooleanObj(value);
		else
			return Tcl_NewWideIntObj(value);
	}
	/// @brief Try to convert a std::shared_ptr<Misc> (or derived) to Tcl_Obj* using the widget's string representation.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<std::derived_from<Misc> T>
	Tcl_Obj* AsObjImpl(const std::shared_ptr<T>& value);
	/// @brief Try to convert a std::variant to Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<typename T>
		requires hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::variant>&& hhh::meta::apply_conjunction<std::remove_cvref_t<T>, AsObjImplTrait>::value
	Tcl_Obj* AsObjImpl(T&& value);
	/// @brief Try to convert a tuple-like or a container to Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<typename T>
		requires (hhh::meta::tuple_like<std::remove_cvref_t<T>>&& hhh::meta::tuple_elements_satisfy<std::remove_cvref_t<T>, AsObjImplTrait>::value)
	|| (utility::is_vector<std::remove_cvref_t<T>> && AsObjImplTrait<typename std::remove_cvref_t<T>::value_type>::value)
		Tcl_Obj * AsObjImpl(T && value);
	/// @brief Try to convert a std::reference_wrapper or cpptkinter::utility::ref_wrapper to Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<typename T>
		requires AsObjImplTrait<typename std::remove_cvref_t<T>::type>::value
	&& (hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::reference_wrapper> || hhh::meta::is_template_instance<std::remove_cvref_t<T>, utility::ref_wrapper>)
		Tcl_Obj* AsObjImpl(T&& value);

	template<typename T>
	struct AsObjImplTrait : std::bool_constant < requires { AsObjImpl(std::declval<T>()); } > { };

	
	struct ignore {};

	template<typename T>
	struct FromObjImplTrait;

	/// @brief Do nothing with Tcl_Obj*.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<ignore> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<ignore>);
	/// @brief Try to convert Tcl_Obj* to std::string.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<std::string> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<std::string>);
	/// @brief Try to convert Tcl_Obj* to bool.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<bool> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<bool>);
	/// @brief Try to convert Tcl_Obj* to byte_array.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<byte_array> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<byte_array>);
	/// @brief Try to convert Tcl_Obj* to double.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<double> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<double>);
	/// @brief Try to convert Tcl_Obj* to long long.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<long long> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<long long>);
	/// @brief Try to convert Tcl_Obj* to tcl_window_type.
	///
	/// If the return value is not empty **ptr's** reference count is incremented exactly once.
	/// @returns A smart pointer holding **ptr**.
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<tk_window_type> FromObjImpl(TkappObject* tkapp, Tcl_Obj* ptr, std::type_identity<tk_window_type>);
	/// @brief Try to convert Tcl_Obj* to std::vector.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<utility::is_vector T>
		requires FromObjImplTrait<typename T::value_type>::value
	std::optional<T> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj* to std::map.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<hhh::meta::is_template_instance<std::map> T>
		requires FromObjImplTrait<typename T::key_type>::value&& FromObjImplTrait<typename T::mapped_type>::value
	std::optional<T> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<T>);

	/// @brief Try to convert Tcl_Obj* to tuple-like.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<typename T>
		requires (hhh::meta::tuple_like<T>&& hhh::meta::tuple_elements_satisfy<T, FromObjImplTrait>::value)
	std::optional<T> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj* to std::variant.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<typename...Args>
		requires ((FromObjImplTrait<Args>::value && ...) && sizeof...(Args) != 0)
	std::optional<std::variant<Args...>> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<std::variant<Args...>>);

	template<typename T>
	struct FromObjImplTrait : std::bool_constant < requires { FromObjImpl(nullptr, nullptr, std::type_identity<T>{}); } > { };


	/// @brief The constraint for the argument type of cpptkinter::_cpptkinter::AsObj().
	/// 
	/// This concept is satisfied if there exists an overload of cpptkinter::_cpptkinter::detail::AsObjImpl() for type T.
	template<typename T>
	concept AsObjConcept = AsObjImplTrait<T>::value;

	/// @brief The constraint for the return type of cpptkinter::_cpptkinter::FromObj().
	/// 
	/// This concept is satisfied if there exists an overload of cpptkinter::_cpptkinter::detail::FromObjImpl() for type R.
	template<typename R>
	concept FromObjConcept = std::same_as<R, void> || FromObjImplTrait<R>::value;

	template<typename T>
	concept GetVarConcept = std::convertible_to<T, std::string> || std::convertible_to<T, Tcl_Obj*>;

	template<typename T>
	concept call_argument_concept = std::same_as<T, detail::Tcl_Obj_vector_raii> || AsObjConcept<T>;

	/// @brief The concept for cpptkinter::_cpptkinter::PythonCmd_ClientData type parameter Args.
	/// 
	/// Specifies that
	/// 1. void(Args...) can be invoked with values of type cpptkinter::_cpptkinter::FromObj<Args>()...\n 
	/// or
	/// 2. void(Args...) can be invoked with a single argument of type std::vector<Tcl_Obj*>.
	template<typename...Args>
	concept PythonCmd_ClientDataArgsConcept = std::invocable<std::function<void(Args...)>, std::vector<Tcl_Obj*>>
		|| (std::invocable<std::function<void(Args...)>, std::remove_cvref_t<Args>...> && (FromObjConcept<std::remove_cvref_t<Args>> && ...));
	/// @brief The concept for cpptkinter::_cpptkinter::PythonCmd_ClientData type parameter R.
	/// 
	/// Specifies that
	/// 1. R can be passed to cpptkinter::_cpptkinter::AsObj()\n 
	/// or
	/// 2. R is void.
	template<typename R>
	concept PythonCmd_ClientDataReturnConcept = std::same_as<R, void> || AsObjConcept<R>;
}

/// @brief Implementation of the Python module _tkinter in C++.
namespace cpptkinter::_cpptkinter
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

	inline std::optional<std::mutex> tcl_lock;
	inline Tcl_Mutex var_mutex;
	inline Tcl_Mutex call_mutex;
	inline Tcl_Mutex command_mutex;
	DEVIATING_IMPLEMENTATION_WARNING("TCL_DECLARE_MUTEX(..._mutex);");

	inline int quitMainLoop = 0;
	inline int errorInCmd = 0;
	inline std::exception_ptr excInCmd;
	inline int Tkinter_busywaitinterval = 20;

#ifndef MS_WINDOWS
	/// @brief Millisecond Sleep() for Unix platforms.
	inline void Sleep(int milli)
	{
		// XXX Too bad if you don't have select().
		struct timeval t;
		t.tv_sec = milli / 1000;
		t.tv_usec = (milli % 1000) * 1000;
		select(0, (fd_set*)0, (fd_set*)0, (fd_set*)0, &t);
	}
#endif // MS_WINDOWS

	int Tcl_AppInit(Tcl_Interp* interp);

	void init(const std::string& tcl_library);

	/// @brief Convert a Tcl_Obj* to a bool.
	///
	/// @param value The Tcl_Obj* to convert. Its reference count will not be modify.
	bool fromBoolean(TkappObject* tkapp, Tcl_Obj* value);
	/// @brief Convert a Tcl_Obj* to a long long.
	///
	/// @param value The Tcl_Obj* to convert. Its reference count will not be modify.
	long long fromWideIntObj(TkappObject* tkapp, Tcl_Obj* value);

	/// @brief Convert a Tcl_Obj* to a string.
	///
	/// @param value The Tcl_Obj* to convert. Its reference count will not be manipulated in any way.
	std::string unicodeFromTclObj(TkappObject* tkapp, Tcl_Obj* value);
	std::string Tkapp_UnicodeResult(TkappObject* self);

#ifdef NDEBUG
	inline TclError Tkinter_Error(TkappObject* self)
	{
		return detail::construct_exception<TclError>(Tkapp_UnicodeResult(self));
	}
#else
	inline TclError Tkinter_Error(TkappObject* self, const std::stacktrace& tr = std::stacktrace::current())
	{
		return detail::construct_exception<TclError>(Tkapp_UnicodeResult(self), tr);
	}
#endif // NDEBUG

	void Tkapp_ThreadSend(TkappObject* self, Tcl_Event* ev, Tcl_Condition* cond, Tcl_Mutex* mutex) noexcept;
	void EnableEventHook();
	void DisableEventHook();
	int WaitForMainloop(TkappObject* self);

	std::shared_ptr<TkappObject> create(const std::string& screenName = {}, std::string_view baseName = {}, std::string_view className = "Tk",
		bool interactive = false, bool wantTk = true, bool sync = false, std::string_view use = "");

	void Tkapp_Trace(TkappObject* self, const auto&...args);

	/// @brief Convert a c++ value to a tcl object.
	///
	/// This function is used to convert a value to a Tcl_Obj*. Throws if the conversion fails. Never returns nullptr.
	/// @param value The c++ value to convert.
	/// @return A Tcl_Obj* with reference count 0 representing the value. Never nullptr;
	Tcl_Obj* AsObj(detail::AsObjConcept auto&& value)
	{
		Tcl_Obj* result = detail::AsObjImpl(std::forward<decltype(value)>(value));

		if (!result)
			throw detail::construct_exception<TclError>("nullptr in AsObj");
		return result;
	}

	/// @brief Convert a tcl object to a c++ value.
	/// 
	/// @tparam T Specifies the return type. Constrained by cpptkinter::_cpptkinter::detail::FromObjConcept.
	/// @param ptr The Tcl_Obj* to convert. If this functions succeeds and the return value is a smart pointer (e.g. tk_window_type) which points to **ptr**,
	/// its reference count is incremented once (and decremented once whenever the smart pointer gets destroyed). Otherwise **ptr's** reference count will not be modified.
	/// @return A c++ value of type R.
	template<detail::FromObjConcept R>
	R FromObj(TkappObject* tkapp, Tcl_Obj* ptr)
	{
		if constexpr (std::same_as<R, void>)
			return;
		else
		{
			auto opt_result = detail::FromObjImpl(tkapp, ptr, std::type_identity<R>{});
			if (opt_result)
				return std::move(*opt_result);

			std::string error_string = std::format("Got tcl object {}.\nExpected c++ type '{}'",
				detail::Tcl_Obj_to_string(tkapp, ptr),
				typeid(R).name());
			throw detail::construct_exception<TclError>(error_string);
		}
	}

	/// @brief Retrieve the current value of a tcl variable.
	template<detail::FromObjConcept R, detail::GetVarConcept A1>
	R GetVar(TkappObject* self, const A1& arg1, const std::string& arg2, int flags);

	/// @brief Modify the value of a tcl variable. If the variable doesn't exist, it will be created.
	void SetVar(TkappObject* self, const std::string& arg1, detail::AsObjConcept auto&& arg2, int flags);
	/// @brief Modify the value of a tcl variable. If the variable doesn't exist, it will be created.
	void SetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, detail::AsObjConcept auto&& arg3, int flags);

	void UnsetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, int flags);

	template<typename Func>
		requires requires { typename std::packaged_task<Func>; }
	using Tkapp_CallEvent = detail::TclBaseEvent<Func>;

	/// @brief Convert c++ values to a vector of Tcl_Obj* with reference count 1.
	template<detail::call_argument_concept...Args>
	detail::Tcl_Obj_vector_raii Tkapp_CallArgs(Args&&...args)
	{
		DEVIATING_IMPLEMENTATION_WARNING("major changes");

		detail::Tcl_Obj_vector_raii raii{};

		auto lambda = [&raii]<typename T>(T && arg)
		{
			if constexpr (std::same_as<T, detail::Tcl_Obj_vector_raii>)
				raii.append(std::forward<T>(arg));
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

	const char* varname_converter(const std::string& arg);
	const char* varname_converter(Tcl_Obj* arg);

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
	/// Constrained by detail::PythonCmd_ClientDataReturnConcept and detail::PythonCmd_ClientDataArgsConcept.
	/// @tparam R The return type of the command.
	/// @tparam Args The argument types of the command.
	template<typename R, typename...Args>
		requires detail::PythonCmd_ClientDataReturnConcept<R>&& detail::PythonCmd_ClientDataArgsConcept<Args...>
	struct PythonCmd_ClientData
	{
		std::function<R(Args...)> func;
		TkappObject* self;

		static void PythonCmdImpl(PythonCmd_ClientData& data, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[])
		{
			if (objc < 1)
				throw detail::construct_exception<TclError>("this should never get triggered. objc should never be < 1 but is " + std::to_string(objc));

			auto args = std::vector(objv + 1, objv + objc);

			if (sizeof...(Args) != args.size())
				throw detail::construct_exception<TclError>(std::format("expected {} but got {} arguments", sizeof...(Args), args.size()));

			ENTER_PYTHON;

			auto begin = args.begin();

			// necessary bc c++ doesn't define function argument evaluation order :(
			auto caller = [&data, &args]<size_t...I>(std::index_sequence<I...>) {
				return data.func(FromObj<std::remove_cvref_t<Args>>(data.self, args[I])...);
			};

			if constexpr (std::same_as<R, void>)
			{
				caller(std::make_index_sequence<sizeof...(Args)>{});
				Tcl_ResetResult(interp);
			}
			else
			{
				auto obj_res = AsObj(caller(std::make_index_sequence<sizeof...(Args)>{}));
				if (obj_res == nullptr)
					throw detail::construct_exception<TclError>("AsObj returned nullptr");
				Tcl_SetObjResult(interp, obj_res);
			}

			LEAVE_PYTHON;
		}

		static int PythonCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) noexcept
		{
			try
			{
				PythonCmdImpl(*static_cast<PythonCmd_ClientData*>(clientData), interp, objc, objv);
			}
			catch (...)
			{
				errorInCmd = 1;
				excInCmd = std::current_exception();
				return TCL_ERROR;
			}

			return TCL_OK;
		}

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
		concept createcommand_concept = requires (Func && func) { decltype(PythonCmd_ClientData{ std::function(std::forward<Func>(func)) }){ std::forward<Func>(func) }; };
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
		[[deprecated]] const Tcl_ObjType* OldBooleanType;
		[[deprecated]] const Tcl_ObjType* WideIntType;
		[[deprecated]] const Tcl_ObjType* BignumType;
		[[deprecated]] const Tcl_ObjType* UTF32StringType;

		//TkappObject() = default;
		TkappObject(const std::string& screenName, std::string className, int interactive, int wantTk, int sync, std::string_view use);

		void wilddispatch();
		//void wantobjects();

		template<typename Func>
			requires requires (Func&& func) { trace = std::forward<Func>(func); }
		void settrace(Func&& func)
		{
			this->trace = std::forward<Func>(func);
		}
		decltype(trace)& gettrace();

		/// @brief This is the main entry point for calling a Tcl command.
		/// 
		/// It supports three cases, with regard to threading:
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
				detail::Tcl_Obj_vector_raii raii = Tkapp_CallArgs(std::forward<Args>(args)...);

				ENTER_TCL;

				auto i = raii.Tcl_EvalObjv(self->interp, flags);

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

		void setvar(const std::string& arg1, detail::AsObjConcept auto&& arg2)
		{
			var_invoke([&]() { SetVar(this, arg1, std::forward<decltype(arg2)>(arg2), TCL_LEAVE_ERR_MSG); }, this);
		}
		void setvar(const std::string& arg1, const std::string& arg2, detail::AsObjConcept auto&& arg3)
		{
			var_invoke([&]() { SetVar(this, arg1, arg2, std::forward<decltype(arg3)>(arg3), TCL_LEAVE_ERR_MSG); }, this);
		}
		void globalsetvar(const std::string& arg1, detail::AsObjConcept auto&& arg2)
		{
			var_invoke([&]() { SetVar(this, arg1, std::forward<decltype(arg2)>(arg2), TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}
		void globalsetvar(const std::string& arg1, const std::string& arg2, detail::AsObjConcept auto&& arg3)
		{
			var_invoke([&]() { SetVar(this, arg1, arg2, std::forward<decltype(arg3)>(arg3), TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}
		template<detail::FromObjConcept R>
		R getvar(const detail::GetVarConcept auto& arg1, const std::string& arg2 = {})
		{
			return var_invoke([&]()->decltype(auto) { return GetVar<R>(this, arg1, arg2, TCL_LEAVE_ERR_MSG); }, this);
		}
		template<detail::FromObjConcept R>
		R globalgetvar(const detail::GetVarConcept auto& arg1, const std::string& arg2 = {})
		{
			return var_invoke([&]()->decltype(auto) { return GetVar<R>(this, arg1, arg2, TCL_LEAVE_ERR_MSG | TCL_GLOBAL_ONLY); }, this);
		}
		void unsetvar(const std::string& arg1, const std::string& arg2 = {});
		void globalunsetvar(const std::string& arg1, const std::string& arg2 = {});

		void getint();
		void getdouble();
		bool getboolean();

		void exprstring();
		void exprlong();
		void exprdouble();
		void exprboolean();
		std::vector<std::string> splitlist(const std::string& s);
		template<detail::createcommand_concept Func>
		void createcommand(const std::string& name, Func&& func)
		{
			DEVIATING_IMPLEMENTATION_WARNING("merges _tkinter_tkapp_createcommand and _tkinter_tkapp_createcommand_impl");

			auto self = this;

			if (self->threaded && self->thread_id != Tcl_GetCurrentThread() && !WaitForMainloop(self))
				throw detail::construct_exception<TclError>("an unknown error occured");

			TRACE(self, ("((ss()O))", "proc", name, func));

			auto data = new decltype(PythonCmd_ClientData{ std::function(std::forward<Func>(func)) }){ std::forward<Func>(func), self };
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
		void deletecommand(const std::string& name);
		void createfilehandler();
		void deletefilehandler();
		void createtimerhandler();
		void mainloop(int threshold = 0);
		void dooneevent();
		void quit();
		void interpaddr();
		void loadtk();
	};


	////////////////////////////////
	// function template definitions
	////////////////////////////////

	void Tkapp_Trace(TkappObject* self, const auto&...args)
	{
		if (self->trace)
		{
			std::vector<std::string> ret{};
			(ret.emplace_back(detail::Tkapp_Trace_to_string(args)), ...);
			self->trace(std::move(ret));
		}
	}

	template<detail::FromObjConcept R, detail::GetVarConcept A1>
	R GetVar(TkappObject* self, const A1& arg1_, const std::string& arg2, int flags)
	{
		const std::conditional_t<std::convertible_to<R, std::string>, std::string, Tcl_Obj*>& arg1 = arg1_;

		const char* name1 = varname_converter(arg1);
		const char* name2 = arg2.empty() ? nullptr : arg2.data();

		ENTER_TCL;
		auto tres = Tcl_GetVar2Ex(Tkapp_Interp(self), name1, name2, flags);
		ENTER_OVERLAP;
		if (tres == NULL)
			throw Tkinter_Error(self);
		else
			return FromObj<R>(self, tres);

		LEAVE_OVERLAP_TCL;
	}

	void SetVar(TkappObject* self, const std::string& arg1, detail::AsObjConcept auto&& arg2, int flags)
	{
		auto name1 = arg1.data();

		if (flags & TCL_GLOBAL_ONLY)
			TRACE(self, ("((ssssO))", "uplevel", "#0", "set", name1, arg2));
		else
			TRACE(self, ("((ssO))", "set", name1, arg2));

		// XXX Acquire tcl lock ? ? ?
		auto newval = AsObj(std::forward<decltype(arg2)>(arg2));

		ENTER_TCL;
		auto ok = Tcl_SetVar2Ex(Tkapp_Interp(self), name1, nullptr, newval, flags);
		ENTER_OVERLAP;
		if (!ok)
			throw Tkinter_Error(self);
		LEAVE_OVERLAP_TCL;
	}
	void SetVar(TkappObject* self, const std::string& arg1, const std::string& arg2, detail::AsObjConcept auto&& arg3, int flags)
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

		auto newval = AsObj(std::forward<decltype(arg3)>(arg3));

		ENTER_TCL;
		auto ok = Tcl_SetVar2Ex(Tkapp_Interp(self), name1, name2, newval, flags);
		ENTER_OVERLAP;
		if (!ok)
			throw Tkinter_Error(self);
		LEAVE_OVERLAP_TCL;
	}

	template<detail::FromObjConcept R>
	R Tkapp_ObjectResult(TkappObject* self)
	{
		auto ptr = detail::Tcl_Obj_Raii(Tcl_GetObjResult(self->interp));
		// We need to increase the reference count so FromObj won't delete value in case it overwrites self->interp's result.
		Tcl_IncrRefCount(ptr);
		return FromObj<R>(self, ptr.get());
	}

	template<detail::FromObjConcept R, detail::call_argument_concept...Args>
	R Tkapp_CallProc(TkappObject* self, int flags, Args&&...args)
	{
		detail::Tcl_Obj_vector_raii raii{};

		ENTER_PYTHON;
		Tkapp_Trace(self, args...);
		raii = Tkapp_CallArgs(std::forward<Args>(args)...);
		LEAVE_PYTHON;

		auto i = raii.Tcl_EvalObjv(self->interp, flags);

		ENTER_PYTHON;
		if (i == TCL_ERROR)
			throw Tkinter_Error(self);
		else
			return Tkapp_ObjectResult<R>(self);
		LEAVE_PYTHON;

		DEVIATING_IMPLEMENTATION_WARNING("original calls Tkapp_CallDeallocArgs, raii's dtor handles that for us");
	}

	template<typename Func>
	std::invoke_result_t<Func&&> var_invoke(Func&& func, TkappObject* self)
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
}

namespace cpptkinter::_cpptkinter::detail
{
	template<std::derived_from<Misc> T>
	Tcl_Obj* AsObjImpl(const std::shared_ptr<T>& value)
	{
		return AsObjImpl(value->operator std::string());
	}
	template<typename T>
		requires hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::variant>&& hhh::meta::apply_conjunction<std::remove_cvref_t<T>, AsObjImplTrait>::value
	Tcl_Obj* AsObjImpl(T&& value)
	{
		return std::visit([]<typename T2>(T2 && arg) { return AsObjImpl(std::forward<T2>(arg)); }, std::forward<T>(value));
	}
	template<typename T>
		requires (hhh::meta::tuple_like<std::remove_cvref_t<T>>&& hhh::meta::tuple_elements_satisfy<std::remove_cvref_t<T>, AsObjImplTrait>::value)
	|| (utility::is_vector<std::remove_cvref_t<T>> && AsObjImplTrait<typename std::remove_cvref_t<T>::value_type>::value)
		Tcl_Obj * AsObjImpl(T && value)
	{
		detail::Tcl_Obj_vector_raii raii{};
		utility::visit_container_or_tuple([&raii]<typename T2>(T2 && elem) { raii.emplace_back(AsObj(std::forward<T2>(elem))); }, std::forward<T>(value));
		auto res = raii.Tcl_NewListObj();
		return res;
	}
	template<typename T>
		requires AsObjImplTrait<typename std::remove_cvref_t<T>::type>::value
	&& (hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::reference_wrapper> || hhh::meta::is_template_instance<std::remove_cvref_t<T>, utility::ref_wrapper>)
		Tcl_Obj* AsObjImpl(T&& value)
	{
		return AsObjImpl(value.get());
	}

	template<typename T>
	T FromObjImplListQuery(Tcl_Interp* interp, TkappObject* tkapp, Tcl_Obj* value, Tcl_Size i)
	{
		Tcl_Obj* tcl_elem{};
		if (Tcl_ListObjIndex(interp, value, i, &tcl_elem) == TCL_ERROR)
			throw Tkinter_Error(tkapp);

		if (!tcl_elem)
			throw detail::construct_exception<TclError>("nullptr in FromObjImplListQuery");

		return FromObj<T>(tkapp, tcl_elem);
	}

	template<utility::is_vector T>
		requires FromObjImplTrait<typename T::value_type>::value
	std::optional<T> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<T>)
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
	std::optional<T> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<T>)
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
			Tcl_Obj* keyPtr, * valuePtr;
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
	std::optional<T> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<T>)
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

			return[&interp, &tkapp, &value]<size_t...I>(std::index_sequence<I...>) {
				return T{ FromObjImplListQuery<std::tuple_element_t<I, T>>(interp, tkapp, value, I)... };
			} (std::make_index_sequence<std::tuple_size_v<T>>{});
		}
		return {};
	}
	template<typename...Args>
		requires ((FromObjImplTrait<Args>::value && ...) && sizeof...(Args) != 0)
	std::optional<std::variant<Args...>> FromObjImpl(TkappObject* tkapp, Tcl_Obj* value, std::type_identity<std::variant<Args...>>)
	{
		auto lambda = [&tkapp, &value]<typename First, typename...Other>(auto & self) -> std::optional<std::variant<Args...>>
		{
			auto intermediate = FromObjImpl(tkapp, value, std::type_identity<First>{});
			if (intermediate)
				return std::move(*intermediate);
			else if constexpr (sizeof...(Other) != 0)
				return self.template operator() < Other... > (self);
			else
				return {};
		};

		return lambda.template operator() < Args... > (lambda);
	}
}




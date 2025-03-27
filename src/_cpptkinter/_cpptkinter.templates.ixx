module;
#include <tk.h>
#include "../global.hpp"
export module cpptkinter:_cpptkinter.templates;
import std;
import :utility;
import :_cpptkinter.functions;
import :_cpptkinter.tcl_obj;
import reflect;

export namespace cpptkinter::detail
{
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

	template<typename Func>
	concept createcommand_concept = requires { utility::callable_to_std_function(std::declval<Func>()); };
}

export namespace cpptkinter::_cpptkinter
{
	const char* varname_converter(const std::string& arg)
	{
		return arg.data();
	}
	const char* varname_converter(const Tcl_Obj& arg)
	{
		return Tcl_GetString(arg);
	}

	/// @brief Retrieve the current value of a tcl variable.
	template<detail::FromObjConcept R>
	R GetVar(TkappObjectImpl* self, const std::string& arg1, const std::string& arg2, int flags)
	{
		const char* name1 = varname_converter(arg1);
		const char* name2 = arg2.empty() ? nullptr : arg2.data();

		ENTER_TCL;
		auto tres = Tcl_GetVar2Ex(self->interp, name1, name2, flags);
		ENTER_OVERLAP;
		if (tres == NULL)
			throw Tkinter_Error(self);
		else
			return FromObj<R>(self, Tcl_Obj(tres));

		LEAVE_OVERLAP_TCL;
	}
	/// @copydoc GetVar
	template<detail::FromObjConcept R>
	R GetVar(TkappObjectImpl* self, const Tcl_Obj& arg1, const std::string& arg2, int flags)
	{
		const char* name1 = varname_converter(arg1);
		const char* name2 = arg2.empty() ? nullptr : arg2.data();

		ENTER_TCL;
		auto tres = Tcl_GetVar2Ex(self->interp, name1, name2, flags);
		ENTER_OVERLAP;
		if (tres == NULL)
			throw Tkinter_Error(self);
		else
			return FromObj<R>(self, Tcl_Obj(tres));

		LEAVE_OVERLAP_TCL;
	}

	/// @brief Modify the value of a tcl variable. If the variable doesn't exist, it will be created.
	void SetVar(TkappObjectImpl* self, const std::string& arg1, const detail::AsObjConcept auto& arg2, int flags)
	{
		auto name1 = arg1.data();

		if (flags & TCL_GLOBAL_ONLY)
			TRACE(self, ("((ssssO))", "uplevel", "#0", "set", name1, arg2));
		else
			TRACE(self, ("((ssO))", "set", name1, arg2));

		// XXX Acquire tcl lock ? ? ?
		auto newval = AsObj(arg2);

		ENTER_TCL;
		auto ok = Tcl_SetVar2Ex(self->interp, name1, nullptr, newval, flags);
		ENTER_OVERLAP;
		if (!ok)
			throw Tkinter_Error(self);
		LEAVE_OVERLAP_TCL;
	}
	/// @brief Modify the value of a tcl variable. If the variable doesn't exist, it will be created.
	void SetVar(TkappObjectImpl* self, const std::string& arg1, const std::string& arg2, const detail::AsObjConcept auto& arg3, int flags)
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
		auto ok = Tcl_SetVar2Ex(self->interp, name1, name2, newval, flags);
		ENTER_OVERLAP;
		if (!ok)
			throw Tkinter_Error(self);
		LEAVE_OVERLAP_TCL;
	}

	void UnsetVar(TkappObjectImpl* self, const std::string& arg1, const std::string& arg2, int flags)
	{
		auto name1 = arg1.data();
		const char* name2 = arg2.empty() ? nullptr : arg2.data();

		if (self->trace)
		{
			if (flags & TCL_GLOBAL_ONLY)
			{
				if (name2)
					TRACE(self, ("((sssN))", "uplevel", "#0", "unset", std::format("{}({})", name1, name2)));
				else
					TRACE(self, ("((ssss))", "uplevel", "#0", "unset", name1));
			}
			else
			{
				if (name2)
					TRACE(self, ("((sN))", "unset", std::format("{}({})", name1, name2)));
				else
					TRACE(self, ("((ss))", "unset", name1));
			}
		}

		ENTER_TCL;
		int code = Tcl_UnsetVar2(self->interp, name1, name2, flags);
		ENTER_OVERLAP;
		if (code == TCL_ERROR)
			throw Tkinter_Error(self);
		LEAVE_OVERLAP_TCL;
	}

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
			if constexpr (hhh::meta::range_of<T, Tcl_Obj>)
				raii.append_range(std::forward<T>(arg));
			else
				raii.emplace_back(AsObj(std::forward<T>(arg)));
		};

		(lambda(std::forward<Args>(args)), ...);

		return raii;
	}

	/// @brief Retrieve the interpreter's current result.
	template<detail::FromObjConcept R>
	R Tkapp_ObjectResult(TkappObjectImpl* self)
	{
		auto ptr = Tcl_Obj(Tcl_GetObjResult(self->interp));
		return FromObj<R>(self, ptr);
	}

	/// @brief Execute a tcl command represented by args.
	template<detail::FromObjConcept R, detail::call_argument_concept...Args>
	R Tkapp_CallProc(TkappObjectImpl* self, int flags, Args&&...args)
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
	std::invoke_result_t<Func&&> var_invoke(Func&& func, TkappObjectImpl* self)
	{
		DEVIATING_IMPLEMENTATION_WARNING("major changes");

		if (self->threaded && self->thread_id != Tcl_GetCurrentThread())
		{
			using FuncType = std::invoke_result_t<Func&&>();
			using VarEventType = VarEvent<FuncType>;

			// The current thread is not the interpreter thread. Marshal the call to the interpreter thread, then wait for completion.
			if (!WaitForMainloop(self))
				throw utility::construct_exception<std::runtime_error>("impossible");

			auto buffer = attemptckalloc(sizeof(VarEventType));
			if (buffer == NULL)
				throw utility::construct_exception<TclError>("attemptckalloc failed to allocate memory");

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
		TkappObjectImpl* self;
#ifndef NDEBUG 
		std::string name;
#endif

		std::string get_error_string_header(::Tcl_Obj* objv0)
		{
			std::string tcl_invocation_name = Tcl_Obj(objv0);
			std::string error_string = std::format("In tcl procedure {}", tcl_invocation_name);
#ifndef NDEBUG 
			error_string += std::format(" ({})", this->name);
#endif
			return error_string;
		}

		void PythonCmdImpl(Tcl_Interp* interp, int objc, ::Tcl_Obj* const objv[])
		{
			if (objc < 1)
				throw utility::construct_exception<TclError>("this should never get triggered. objc should never be < 1 but is " + std::to_string(objc));

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
					throw utility::construct_exception<TclError>(error_string);
				}
			}

			ENTER_PYTHON;

			auto inner_caller = [&]<typename T, size_t I>() {
				auto opt_result = detail::FromObjImpl(this->self, args[I], std::type_identity<std::remove_cvref_t<T>>{});
				if (opt_result.has_value())
					return std::move(*opt_result);

				std::string error_string = std::format("{}\ntcl argument at position {} was {}\nexpected c++ argument type {}",
					this->get_error_string_header(objv[0]),
					I + 1,
					detail::Tcl_Obj_to_string(this->self, args[I]),
					reflect::type_name<T>());
				throw utility::construct_exception<TclError>(error_string);
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
			try
			{
				delete static_cast<PythonCmd_ClientData*>(clientData);
			}
			catch (...)
			{
				errorInCmd = 1;
				excInCmd = std::current_exception();
			}
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
}

namespace cpptkinter::detail
{
	template<typename Func>
	auto new_PythonCmd_ClientData(Func&& func, TkappObjectImpl* self, const std::string& name) -> decltype(PythonCmd_ClientData{ std::forward<Func>(func) })*
	{
		return new decltype(PythonCmd_ClientData{ std::function(std::forward<Func>(func)) }){
			std::forward<Func>(func),
			self
#ifndef NDEBUG 
				,name
#endif
		};
	}
}
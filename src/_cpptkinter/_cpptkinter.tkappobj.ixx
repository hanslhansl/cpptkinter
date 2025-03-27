module;
#include "../global.hpp"
#include <tk.h>
#ifdef _WIN32
#include <windows.h>
#endif
export module cpptkinter:_cpptkinter.tkappobj;
import :_cpptkinter.functions;
import :_cpptkinter.tcl_obj;
import :_cpptkinter.templates;
import std;


#define CHECK_TCL_APPARTMENT  if (self->threaded && self->thread_id != Tcl_GetCurrentThread()) throw utility::construct_exception<std::runtime_error>("Calling Tcl from different apartment")

export namespace cpptkinter::_cpptkinter
{
	/// @brief This class allows running Tcl code. Cpptkinter uses it internally a lot.
	struct TkappObject : TkappObjectImpl
	{
		TkappObject(const std::string& screenName, std::string className, int interactive, int wantTk, int sync, const std::string& use)
		{
			this->interp = Tcl_CreateInterp();
			this->threaded = Tcl_GetVar2Ex(this->interp, "tcl_platform", "threaded", TCL_GLOBAL_ONLY) != nullptr;
			this->thread_id = Tcl_GetCurrentThread();

#ifndef TCL_THREADS
			if (this->threaded)
				throw utility::construct_exception<TclError>("Tcl is threaded but _cpptkinter is not");
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

		static std::shared_ptr<TkappObject> create(const std::string& screenName, const std::string& className, bool interactive, bool wantTk, bool sync, const std::string& use)
		{
			return std::make_shared<TkappObject>(screenName, className, interactive, wantTk, sync, use);
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
					throw utility::construct_exception<std::runtime_error>("impossible");

				auto buffer = attemptckalloc(sizeof(CallEventType));
				if (buffer == nullptr)
					throw utility::construct_exception<TclError>("attemptckalloc failed to allocate memory");

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

			if (Tcl_SplitList(self->interp, s.c_str(), &argc, &argv) == TCL_ERROR)
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
				throw utility::construct_exception<TclError>("an unknown error occured");

			TRACE(self, ("((ss()O))", "proc", name, func));

			auto data = detail::new_PythonCmd_ClientData(utility::callable_to_std_function(std::forward<Func>(func)), self, name);
			DEVIATING_IMPLEMENTATION_WARNING("in the original data 'holds a reference' to self keeping it from being destructed. i doubt that this is actually necessary");

			if (self->threaded && self->thread_id != Tcl_GetCurrentThread())
			{
				auto buffer = (CommandEvent*)attemptckalloc(sizeof(CommandEvent));
				if (buffer == nullptr)
					throw utility::construct_exception<TclError>("attemptckalloc failed to allocate memory");

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
					throw utility::construct_exception<TclError>("can't create Tcl command");
			}
			else
			{
				ENTER_TCL;
				if (Tcl_CreateObjCommand(self->interp, name.data(), data->PythonCmd, data, data->PythonCmdDelete) == nullptr)
					throw utility::construct_exception<TclError>("can't create Tcl command");
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
					throw utility::construct_exception<TclError>("attemptckalloc failed to allocate memory");

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
				throw utility::construct_exception<TclError>(std::format("can't delete Tcl command '{}'", name));
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
						std::this_thread::sleep_for(std::chrono::milliseconds(Tkinter_busywaitinterval));
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

			Tcl_Interp* interp = self->interp;
			const char* _tk_exists;

			/* We want to guard against calling Tk_Init() multiple times */
			CHECK_TCL_APPARTMENT;
			ENTER_TCL;
			int err = Tcl_Eval(self->interp, "info exists     tk_version");
			ENTER_OVERLAP;
			if (err == TCL_ERROR)
				throw Tkinter_Error(self);
			else
				_tk_exists = Tcl_GetStringResult(self->interp);

			LEAVE_OVERLAP_TCL;

			if (_tk_exists == nullptr || _tk_exists != std::string_view("1"))
				if (Tk_Init(interp) == TCL_ERROR)
					throw Tkinter_Error(self);
		}
	};
}
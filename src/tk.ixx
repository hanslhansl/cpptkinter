/// @file tk.ixx
/// @brief Contains tcl/tk as a c++ module.


export module cpptkinter:tk;

#include "tk.h"
export
{
#ifdef Tcl_GetByteArrayFromObj
	EXTERN inline unsigned char* (Tcl_GetByteArrayFromObj)(Tcl_Obj* objPtr, Tcl_Size* sizePtr)
	{
		return Tcl_GetByteArrayFromObj(objPtr, sizePtr);
	}
#endif

	decltype(auto) (Tcl_GetStringResult)(Tcl_Interp* interp)
	{
		return Tcl_GetStringResult(interp);
	}
	decltype(auto) (Tcl_GetVar)(Tcl_Interp* interp, const char* varName, int flags)
	{
		return Tcl_GetVar(interp, varName, flags);
	}
	decltype(auto) (Tcl_SetVar)(Tcl_Interp* interp, const char* varName, const char* newValue, int flags)
	{
		return Tcl_SetVar(interp, varName, newValue, flags);
	}
	decltype(auto) (Tcl_NewIntObj)(int value)
	{
		return Tcl_NewIntObj(value);
	}
	decltype(auto) (Tcl_NewBooleanObj)(int intValue)
	{
		return Tcl_NewBooleanObj(intValue);
	}
	decltype(auto) (Tcl_Eval)(Tcl_Interp* interp, const char* objPtr)
	{
		return Tcl_Eval(interp, objPtr);
	}

#undef ckfree
	decltype(auto) ckfree(void* ptr)
	{
		return Tcl_Free(ptr);
	}
#undef attemptckalloc
	decltype(auto) attemptckalloc(size_t size)
	{
		return Tcl_AttemptAlloc(size);
	}
}



#define EXPAND2(x) x
#define EXPAND(x) EXPAND2(x)

constexpr auto TCL_VERSION_ = TCL_VERSION;
#undef TCL_VERSION
constexpr auto TCL_READABLE_ = TCL_READABLE;
#undef TCL_READABLE
constexpr auto TCL_WRITABLE_ = TCL_WRITABLE;
#undef TCL_WRITABLE
constexpr auto TCL_EXCEPTION_ = TCL_EXCEPTION;
#undef TCL_EXCEPTION
constexpr auto TCL_WINDOW_EVENTS_ = TCL_WINDOW_EVENTS;
#undef TCL_WINDOW_EVENTS
constexpr auto TCL_FILE_EVENTS_ = TCL_FILE_EVENTS;
#undef TCL_FILE_EVENTS
constexpr auto TCL_TIMER_EVENTS_ = TCL_TIMER_EVENTS;
#undef TCL_TIMER_EVENTS
constexpr auto TCL_IDLE_EVENTS_ = TCL_IDLE_EVENTS;
#undef TCL_IDLE_EVENTS
constexpr auto TCL_ALL_EVENTS_ = TCL_ALL_EVENTS;
#undef TCL_ALL_EVENTS
constexpr auto TCL_DONT_WAIT_ = TCL_DONT_WAIT;
#undef TCL_DONT_WAIT
constexpr auto TCL_ERROR_ = TCL_ERROR;
#undef TCL_ERROR
constexpr auto TCL_GLOBAL_ONLY_ = TCL_GLOBAL_ONLY;
#undef TCL_GLOBAL_ONLY
constexpr auto TCL_OK_ = TCL_OK;
#undef TCL_OK
constexpr auto TCL_EVAL_DIRECT_ = TCL_EVAL_DIRECT;
#undef TCL_EVAL_DIRECT
constexpr auto TCL_EVAL_GLOBAL_ = TCL_EVAL_GLOBAL;
#undef TCL_EVAL_GLOBAL
constexpr auto TCL_LEAVE_ERR_MSG_ = TCL_LEAVE_ERR_MSG;
#undef TCL_LEAVE_ERR_MSG

constexpr auto TK_VERSION_ = TK_VERSION;
#undef TK_VERSION

export
{
	constexpr auto TCL_VERSION = TCL_VERSION_;
	constexpr auto TCL_READABLE = TCL_READABLE_;
	constexpr auto TCL_WRITABLE = TCL_WRITABLE_;
	constexpr auto TCL_EXCEPTION = TCL_EXCEPTION_;
	constexpr auto TCL_WINDOW_EVENTS = TCL_WINDOW_EVENTS_;
	constexpr auto TCL_FILE_EVENTS = TCL_FILE_EVENTS_;
	constexpr auto TCL_TIMER_EVENTS = TCL_TIMER_EVENTS_;
	constexpr auto TCL_IDLE_EVENTS = TCL_IDLE_EVENTS_;
	constexpr auto TCL_ALL_EVENTS = TCL_ALL_EVENTS_;
	constexpr auto TCL_DONT_WAIT = TCL_DONT_WAIT_;
	constexpr auto TCL_ERROR = TCL_ERROR_;
	constexpr auto TCL_GLOBAL_ONLY = TCL_GLOBAL_ONLY_;
	constexpr auto TCL_OK = TCL_OK_;
	constexpr auto TCL_EVAL_DIRECT = TCL_EVAL_DIRECT_;
	constexpr auto TCL_EVAL_GLOBAL = TCL_EVAL_GLOBAL_;
	constexpr auto TCL_LEAVE_ERR_MSG = TCL_LEAVE_ERR_MSG_;

	constexpr auto TK_VERSION = TK_VERSION_;
}
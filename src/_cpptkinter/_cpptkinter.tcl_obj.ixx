module;
#include <tk.h>
#include "../global.hpp"
export module cpptkinter:_cpptkinter.tcl_obj;
import std;
import :utility;
import :_cpptkinter1;
import :reflect;
using namespace std::literals;

export namespace cpptkinter::_cpptkinter
{
	using byte_array = std::vector<std::byte>;

	/// @brief Represents a Tcl object.
	class Tcl_Obj : public utility::enable_operator_string_formatting
	{
		::Tcl_Obj* ptr;

	public:
		explicit Tcl_Obj(::Tcl_Obj* ptr) : ptr{ ptr }
		{
			if (!this->ptr)
				throw utility::construct_exception<std::invalid_argument>("nullptr in Tcl_Obj");
			Tcl_IncrRefCount(this->ptr);
		}
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

		std::string _repr_() const
		{
			if (!this->ptr->typePtr || !this->ptr->typePtr->name)
				throw utility::construct_exception<TclError>(std::format("Tcl_Obj->typePtr->name is nullptr (Tcl_GetString: {})", Tcl_GetString(this->ptr)));
			return std::format("<{} object: {}>", this->ptr->typePtr->name, Tcl_GetString(this->ptr));
		}
		operator std::string() const
		{
			return Tcl_GetString(this->ptr);
		}
	};

	/// @brief Represents a tcl object of type 'window' which is a type introduced by Tk.
	struct tk_window_type : Tcl_Obj
	{
		using Tcl_Obj::Tcl_Obj;
	};

	Tcl_Obj Tcl_NewListObj(const std::vector<Tcl_Obj>& objects)
	{
		auto objs = objects | std::views::transform(&Tcl_Obj::get) | std::ranges::to<std::vector>();
		return Tcl_Obj(::Tcl_NewListObj(objs.size(), objs.data()));
	}

	/// @brief Convert a Tcl_Obj to a long long.
	///
	/// @param value The Tcl_Obj to convert.
	long long fromWideIntObj(TkappObjectImpl* self, const Tcl_Obj& value)
	{
		long long wideValue;
		if (Tcl_GetWideIntFromObj(self->interp, value, &wideValue) == TCL_OK)
			return wideValue;

		throw Tkinter_Error(self);
	}

	/// @brief Convert a Tcl_Obj to a bool.
	///
	/// @param value The Tcl_Obj to convert.
	bool fromBoolean(TkappObjectImpl* self, const Tcl_Obj& value)
	{
		int boolValue{};
		if (Tcl_GetBooleanFromObj(self->interp, value, &boolValue) == TCL_ERROR)
			throw Tkinter_Error(self);

		return bool(boolValue);
	}

	/// @brief Convert a Tcl_Obj to a string.
	///
	/// @param value The Tcl_Obj to convert.
	std::string unicodeFromTclObj(TkappObjectImpl* self, const Tcl_Obj& value)
	{
		DEVIATING_IMPLEMENTATION_WARNING("original converts to and returns python unicode string using unicodeFromTclStringAndSize");
		const char* str = Tcl_GetString(value);
		if (str == nullptr)
			throw Tkinter_Error(self);
		return str;
	}

	int Tcl_EvalObjv(Tcl_Interp* interp, const std::vector<Tcl_Obj>& objects, int flags)
	{
		auto objs = objects | std::views::transform(&Tcl_Obj::get) | std::ranges::to<std::vector>();
		return ::Tcl_EvalObjv(interp, objs.size(), objs.data(), flags);
	}
}

export namespace cpptkinter::detail
{
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
		requires (hhh::meta::tuple_like<T>&& hhh::meta::tuple_elements_satisfy<T, AsObjImplTrait>::value)
	|| (utility::is_vector<T> && AsObjImplTrait<typename T::value_type>::value)
		Tcl_Obj AsObjImpl(const T & value);
	/// @brief Try to convert a std::reference_wrapper to Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::AsObj()
	template<typename T>
		requires AsObjImplTrait<typename T::type>::value
	&& (hhh::meta::is_template_instance<T, std::reference_wrapper>)
		Tcl_Obj AsObjImpl(const T& value);

	template<typename T>
	struct AsObjImplTrait : std::bool_constant < requires { AsObjImpl(std::declval<T>()); } > {};

	/// @brief The constraint for the argument type of cpptkinter::_cpptkinter::AsObj().
	/// 
	/// T is intended to come from const T& as AsObjImpl doesn't make use of rvalues.
	/// This concept is satisfied if there exists an overload of cpptkinter::detail::AsObjImpl() for type T.
	template<typename T>
	concept AsObjConcept = AsObjImplTrait<T>::value;

	/// @brief Functor type of AsObj. operator() calls AsObjImpl.
	struct AsObjFunctorType
	{
		/// @brief cant be static bc of msvc bug
		/*static*/ Tcl_Obj operator()(const AsObjConcept auto& value) const
		{
			return AsObjImpl(value);
		}
	};

	struct ignore {};

	template<typename T>
	struct FromObjImplTrait;

	/// @brief Do nothing with Tcl_Obj.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<ignore> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<ignore>)
	{
		return ignore{};
	}
	/// @brief Try to convert Tcl_Obj to std::string.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<std::string> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<std::string>)
	{
		if (value->typePtr == nullptr
			|| (self->StringType && value->typePtr == self->StringType)
			|| (self->UTF32StringType && value->typePtr == self->UTF32StringType)
			|| (value->typePtr && value->typePtr->name && value->typePtr->name == "parsedVarName"sv)
			)
			return unicodeFromTclObj(self, value);
		return {};
	}
	/// @brief Try to convert Tcl_Obj to bool.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<bool> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<bool>)
	{
		if ((value->typePtr == self->BooleanType && self->BooleanType)
			|| (value->typePtr == self->OldBooleanType && self->OldBooleanType))
			return fromBoolean(self, value);

		return {};
	}
	/// @brief Try to convert Tcl_Obj to byte_array.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<byte_array> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<byte_array>)
	{
		if (value->typePtr == self->ByteArrayType && self->ByteArrayType)
		{
			Tcl_Size size{};
			auto data = Tcl_GetByteArrayFromObj(value, &size);
			return byte_array(reinterpret_cast<std::byte*>(data), reinterpret_cast<std::byte*>(data + size));
		}
		return {};
	}
	/// @brief Try to convert Tcl_Obj to double.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<double> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<double>)
	{
		if (value->typePtr == self->DoubleType && self->DoubleType)
			return value->internalRep.doubleValue;
		return {};
	}
	/// @brief Try to convert Tcl_Obj to long long.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<long long> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<long long>)
	{
		DEVIATING_IMPLEMENTATION_WARNING("original has special handling for self->BignumType");

		if ((value->typePtr == self->IntType && self->IntType)
			|| (value->typePtr == self->WideIntType && self->WideIntType)
			|| (value->typePtr == self->BignumType && self->BignumType))
			return fromWideIntObj(self, value);

		return {};
	}
	/// @brief Try to convert Tcl_Obj to tk_window_type.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<tk_window_type> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& ptr, std::type_identity<tk_window_type>)
	{
		if (ptr->typePtr == self->WindowType && self->WindowType)
			return { tk_window_type(ptr) };
		return {};
	}
	/// @brief Try to convert Tcl_Obj to Tcl_Obj (i.e. do nothing).
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	std::optional<Tcl_Obj> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& ptr, std::type_identity<Tcl_Obj>)
	{
		return ptr;
	}
	/// @brief Try to convert Tcl_Obj to std::vector.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<utility::is_vector T>
		requires FromObjImplTrait<typename T::value_type>::value
	std::optional<T> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj to std::map.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<hhh::meta::is_template_instance<std::map> T>
		requires FromObjImplTrait<typename T::key_type>::value&& FromObjImplTrait<typename T::mapped_type>::value
	std::optional<T> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj to tuple-like.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<typename T>
		requires (hhh::meta::tuple_like<T>&& hhh::meta::tuple_elements_satisfy<T, FromObjImplTrait>::value)
	std::optional<T> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<T>);
	/// @brief Try to convert Tcl_Obj to std::variant.
	///
	/// @see cpptkinter::_cpptkinter::FromObj()
	template<typename...Args>
		requires ((FromObjImplTrait<Args>::value && ...) && sizeof...(Args) != 0)
	std::optional<std::variant<Args...>> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<std::variant<Args...>>);

	template<typename T>
	struct FromObjImplTrait : std::bool_constant < requires { FromObjImpl({}, std::declval<Tcl_Obj>(), std::type_identity<T>{}); } > {};

	/// @brief The constraint for the return type of cpptkinter::_cpptkinter::FromObj().
	/// 
	/// This concept is satisfied if 
	/// 1. there exists an overload of cpptkinter::detail::FromObjImpl() for type R.
	/// or 
	/// 2. R is void (trivial case).
	template<typename R>
	concept FromObjConcept = std::same_as<R, void> || FromObjImplTrait<R>::value;

	template<typename T>
	concept call_argument_concept = hhh::meta::range_of<T, Tcl_Obj> || AsObjConcept<T>;

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
	&& (hhh::meta::is_template_instance<T, std::reference_wrapper>)
	Tcl_Obj AsObjImpl(const T& value)
	{
		return AsObjImpl(value.get());
	}

	template<typename T>
	T FromObjImplListQuery(Tcl_Interp* interp, TkappObjectImpl* self, const Tcl_Obj& value, Tcl_Size i)
	{
		::Tcl_Obj* tcl_elem{};
		if (Tcl_ListObjIndex(interp, value, i, &tcl_elem) == TCL_ERROR)
			throw Tkinter_Error(self);

		return FromObj<T>(self, Tcl_Obj(tcl_elem));
	}

	template<utility::is_vector T>
		requires FromObjImplTrait<typename T::value_type>::value
	std::optional<T> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<T>)
	{
		using value_type = typename T::value_type;

		if (value->typePtr == nullptr)
			return std::optional<T>{ std::in_place };
		else if (value->typePtr == self->ListType && self->ListType)
		{
			Tcl_Interp* interp = self->interp;
			Tcl_Size size{};
			if (Tcl_ListObjLength(interp, value, &size) == TCL_ERROR)
				throw Tkinter_Error(self);

			std::optional<T> result{ std::in_place };
			auto&& vec = *result;
			vec.reserve(size);
			for (Tcl_Size i = 0; i < size; i++)
				vec.emplace_back(FromObjImplListQuery<value_type>(interp, self, value, i));

			return result;
		}
		return {};
	}
	template<hhh::meta::is_template_instance<std::map> T>
		requires FromObjImplTrait<typename T::key_type>::value&& FromObjImplTrait<typename T::mapped_type>::value
	std::optional<T> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<T>)
	{
		using key_type = typename T::key_type;
		using mapped_type = typename T::mapped_type;

		if (value->typePtr == nullptr)
			return std::optional<T>{ std::in_place };
		else if (value->typePtr == self->DictType && self->DictType)
		{
			std::optional<T> result{ std::in_place };
			auto&& map = *result;

			Tcl_Interp* interp = self->interp;
			Tcl_DictSearch search{};
			::Tcl_Obj* keyPtr, * valuePtr;
			int done{};

			if (Tcl_DictObjFirst(interp, value, &search, &keyPtr, &valuePtr, &done) != TCL_OK)
				throw Tkinter_Error(self);

			while (!done)
			{
				auto&& [it, success] = map.emplace(FromObj<key_type>(self, keyPtr), FromObj<mapped_type>(self, valuePtr));
				if (!success)
					throw utility::construct_exception<TclError>("duplicate key in dict");

				Tcl_DictObjNext(&search, &keyPtr, &valuePtr, &done);
			}

			Tcl_DictObjDone(&search);
			return result;
		}
		return {};
	}
	template<typename T>
		requires (hhh::meta::tuple_like<T>&& hhh::meta::tuple_elements_satisfy<T, FromObjImplTrait>::value)
	std::optional<T> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<T>)
	{
		if (std::tuple_size_v<T> == 0 && value->typePtr == nullptr)
			return std::optional<T>{ std::in_place };
		else if (value->typePtr == self->ListType && self->ListType)
		{
			Tcl_Interp* interp = self->interp;
			Tcl_Size size{};
			if (Tcl_ListObjLength(interp, value, &size) == TCL_ERROR)
				throw Tkinter_Error(self);

			if (size != std::tuple_size_v<T>)
				throw utility::construct_exception<TclError>(std::format("expected {} elements but got {}", std::tuple_size_v<T>, size));

			return[&]<size_t...I>(std::index_sequence<I...>) {
				return T{ FromObjImplListQuery<std::tuple_element_t<I, T>>(interp, self, value, I)... };
			} (std::make_index_sequence<std::tuple_size_v<T>>{});
		}
		return {};
	}
	template<typename...Args>
		requires ((FromObjImplTrait<Args>::value && ...) && sizeof...(Args) != 0)
	std::optional<std::variant<Args...>> FromObjImpl(TkappObjectImpl* self, const Tcl_Obj& value, std::type_identity<std::variant<Args...>>)
	{
		auto lambda = [&]<typename First, typename...Other>(this auto & lambda) -> std::optional<std::variant<Args...>>
		{
			auto intermediate = FromObjImpl(self, value, std::type_identity<First>{});
			if (intermediate.has_value())
				return std::move(*intermediate);
			else if constexpr (sizeof...(Other) != 0)
				return lambda.template operator() < Other... > ();
			else
				return {};
		};

		return lambda.template operator() < Args... > ();
	}
}

export namespace cpptkinter::_cpptkinter
{
	/// @brief Convert a c++ value to a tcl object.
	///
	/// Throws if the conversion fails. Implemented as a functor to allow for use with std::views::transform.
	/// @param value The c++ value to convert.
	/// @return A Tcl_Obj representing the value.
	/// @see detail::AsObjImpl()
	constexpr detail::AsObjFunctorType AsObj{};

	/// @brief Convert a tcl object to a c++ value.
	/// 
	/// @tparam T Specifies the return type. Constrained by cpptkinter::detail::FromObjConcept.
	/// @param ptr The Tcl_Obj to convert. If this functions succeeds and the return value is a smart pointer (e.g. tk_window_type) which points to **ptr**,
	/// its reference count is incremented once (and decremented once whenever the smart pointer gets destroyed). Otherwise **ptr's** reference count will not be modified.
	/// @return A c++ value of type R.
	template<detail::FromObjConcept R>
	R FromObj(TkappObjectImpl* self, const Tcl_Obj& ptr)
	{
		if constexpr (std::same_as<R, void>)
			return;
		else
		{
			auto opt_result = detail::FromObjImpl(self, ptr, std::type_identity<R>{});
			if (opt_result.has_value())
				return std::move(*opt_result);

			std::string error_string = std::format("Got tcl object {}.\nExpected c++ type {}.", detail::Tcl_Obj_to_string(self, ptr), reflect::type_name<R>());
			throw utility::construct_exception<TclError>(error_string);
		}
	}
}
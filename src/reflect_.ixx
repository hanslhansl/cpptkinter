module;
#include <reflect/reflect.hpp>
export module reflect;

export namespace reflect
{
	using reflect::type_name;
	using reflect::size;

	using reflect::member_name;
	using reflect::member_type;
	using reflect::for_each;

	using reflect::get;
	using reflect::enum_name;
}
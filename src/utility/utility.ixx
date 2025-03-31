/// @file utility.ixx
/// @brief Contains utility functions and classes which are not present in tkinter or _tkinter.
module;
#include "../global.hpp"
//#include <vector>   // to get __cpp_lib_stacktrace to work on msvc
#include <range/v3/all.hpp>
export module cpptkinter:utility;
import std;
import hhh;
import reflect;
import :utility.detail;
import aggregate_formatter;
import variant_formatter;
import optional_formatter;


export {
#if defined(__cpp_lib_ranges_stride) && defined(__cpp_lib_ranges_to_container) && defined(__cpp_lib_ranges) && defined(__cpp_lib_ranges_zip) && defined(__cpp_lib_ranges_join_with) && defined(__cpp_lib_ranges_enumerate)
    using std::views::stride;
    using std::ranges::to;
    using std::views::drop;
    using std::views::zip;
    using std::views::transform;
    using std::ranges::join_with_view;
    using std::views::enumerate;
#ifdef __clang__
    static_assert(false, "this means that clang/libc++ finally supports all these and range v3 can be removed from the project");
#endif
#else
    using ranges::views::stride;
    using ranges::to;
    using ranges::views::drop;
    using ranges::views::zip;
    using ranges::views::transform;
    using ranges::join_with_view;
	using ranges::views::enumerate;
#endif
}

/// @brief Utilities that aren't related to Python's tkinter or _tkinter.
export namespace cpptkinter::utility
{
    /// @brief Checks if T is std::vector.
    template<typename T>
    concept is_vector = hhh::meta::is_template_instance<T, std::vector>;

    /// @brief Concept for ranges of types convertible to T.
    template<typename R, typename To>
    concept range_of_convertible_to = std::ranges::range<R> && std::convertible_to<std::ranges::range_value_t<R>, To>;

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

    /// @brief Invokes a callable with a value (like std::invoke) or a variant's currently held alternative (like std::visit).
    /// 
    /// @param callable a functor.
    /// @param value a variant or an arbitrary value.
    template<typename Callable, typename T>
        requires requires { detail::visit_or_invoke_impl(std::declval<Callable>(), std::declval<T>()); }
    decltype(auto) visit_or_invoke(Callable&& callable, T&& value)
    {
        return detail::visit_or_invoke_impl(std::forward<Callable>(callable), std::forward<T>(value));
    }

    /// @brief Calls a callable on every element of a range or a tuple-like.
    ///
    /// @tparam T Needs to satisfy @ref container or @ref is_tuple.
    template<typename Visitor, typename T> requires std::ranges::range<T> || hhh::meta::tuple_like<T>
    void visit_range_or_tuple(Visitor && vis, T&& value)
    {
        return detail::visit_range_or_tuple_impl(std::forward<Visitor>(vis), std::forward<T>(value));
    }

    /// @brief Invokes a callable with an optional's value (if present).
    /// 
    /// @param callable a functor.
    /// @param optional an optional.
    /// @param args additional arguments to pass to the callable.
    template<typename Callable, typename Optional, typename...Args>
        requires (hhh::meta::is_template_instance<std::remove_cvref_t<Optional>, std::optional>&& std::invocable<Callable, decltype(std::declval<Optional>().value()), Args...>)
    void invoke_or_and_then(Callable&& callable, Optional&& optional, Args&&...args)
    {
        if (optional)
            std::invoke(std::forward<Callable>(callable), std::forward<Optional>(optional).value(), std::forward<Args>(args)...);
    }
    /// @brief Invokes a callable with a value (equal to std::invoke).
    /// 
    /// @param callable a functor.
    /// @param value a value.
    /// @param args additional arguments to pass to the callable.
    template<typename Callable, typename T, typename...Args>
        requires (!hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::optional>&& std::invocable<Callable, T, Args...>)
    void invoke_or_and_then(Callable&& callable, T&& value, Args&&...args)
    {
        std::invoke(std::forward<Callable>(callable), std::forward<T>(value), std::forward<Args>(args)...);
    }

	/// @brief A weak reference to a widget.
    ///
	/// Holds a weak reference to a widget of type T (e.g. Tk, Button, etc.). Basically works like std::weak_ptr.
    template<std::derived_from<Misc> T>
    class weak
    {
		using impl_type = typename detail::widget_friend::impl_type<T>;
        std::weak_ptr<impl_type> pimpl;
    public:
        weak() = default;
        weak(const T& obj) : pimpl(std::static_pointer_cast<impl_type>(detail::widget_friend::get_widget_pimpl(obj)))
        {

        }

		/// @brief Creates a strong reference to the widget.
        /// 
		/// Equivalent to std::weak_ptr::lock except that this function throws if the widget has already been destroyed.
        T lock() const
        {
            return detail::widget_friend::construct_widget_from_weak_pimpl<T>(this->pimpl);
        }
        bool expired() const noexcept
        {
            return this->pimpl.expired();
        }
        long use_count() const noexcept
        {
            return this->pimpl.use_count();
        }
    };

    template<typename...Args>
    struct reference_variant
    {
        using variant_type = std::variant<std::reference_wrapper<Args>...>;
        variant_type v;

        template<typename T>
            requires requires (T& ref) { variant_type(std::in_place_type_t<std::reference_wrapper<T>>(), std::reference_wrapper<T>(ref)); }
        reference_variant(T& ref) : v(std::in_place_type_t<std::reference_wrapper<T>>(), std::reference_wrapper<T>(ref))
        {

        }

        template<typename T>
        reference_variant& operator=(T&& t)
        {
            auto lambda = [&t](auto& val) {
                if constexpr (requires { val.get() = std::forward<T>(t); })
                    val.get() = std::forward<T>(t);
                else
                    throw std::runtime_error("Invalid assignment");
                };

            std::visit(lambda, this->v);

            return *this;
        }
    };

    class optional_mutex_adaptor
    {
        std::optional<std::mutex>& m;

    public:
        optional_mutex_adaptor(std::optional<std::mutex>& m) noexcept : m(m)
        {

        }
        void lock()
        {
            if (m.has_value())
                m.value().lock();
        }
        void unlock()
        {
            if (m.has_value())
                m.value().unlock();
        }
    };
    class optional_inverse_mutex_adaptor
    {
        std::optional<std::mutex>& m;

    public:
        optional_inverse_mutex_adaptor(std::optional<std::mutex>& m) : m(m)
        {

        }
        void lock()
        {
            if (m.has_value())
                m.value().unlock();
        }
        void unlock()
        {
            if (m.has_value())
                m.value().lock();
        }
    };

	template<typename T, typename Base>
	concept is_derived_shared_ptr = hhh::meta::is_template_instance<T, std::shared_ptr> && std::is_base_of_v<Base, typename T::element_type>;

    template<typename V, typename...Args>
    struct extend_variants;
	template<typename...VArgs, typename...Args>
	struct extend_variants<std::variant<VArgs...>, Args...>
	{
		using type = std::variant<VArgs..., Args...>;
	};

    /// @brief Base class for member functors.
    template<typename T>
    class member_functor
    {
    protected:
        T& self;
    public:
        using container_type = T;

        member_functor(T& t) : self(t) {}
    };

    template<typename Func>
    decltype(std::function(std::declval<Func>())) callable_to_std_function(Func&& func)
    {
		return std::forward<Func>(func);
    }
    template<typename Func> requires requires {
        typename std::remove_cvref_t<Func>::decays_to;
        typename std::remove_cvref_t<Func>::container_type;
        requires std::derived_from<std::remove_cvref_t<Func>, member_functor<typename std::remove_cvref_t<Func>::container_type>>;
    }
    decltype(std::function(std::declval<typename std::remove_cvref_t<Func>::decays_to>())) callable_to_std_function(Func&& func)
    {
        return std::forward<Func>(func);
    }

    template<typename...Args>
    constexpr detail::union_arg_overload<Args...> to_union_arg{};

	template<typename T, typename...Args>
	concept union_arg = requires(T&& t) { to_union_arg<Args...>(std::forward<T>(t)); };

    /// @brief Concept for ranges of types convertible to T.
    template<typename R, typename...Args>
    concept range_of_union_arg = std::ranges::range<R> && union_arg<std::ranges::range_value_t<R>, Args...> && (!union_arg<R, Args...>);

    /// @brief static_cast a widget to a child (or base) class.
    template<std::derived_from<Misc> To, std::derived_from<Misc> From>
    To static_widget_cast(const From& from) requires requires { detail::widget_friend::static_widget_cast_down<To>(from); }
    {
        return detail::widget_friend::static_widget_cast_down<To>(from);
    }

    /// @brief dynamic_cast a widget to a child (or base) class.
    template<std::derived_from<Misc> To, std::derived_from<Misc> From>
    To dynamic_widget_cast(const From& from) requires requires { detail::widget_friend::dynamic_widget_cast_down<To>(from); }
    {
        return detail::widget_friend::dynamic_widget_cast_down<To>(from);
    }

    /// @brief Publicly inerihiting this struct enables formatting based on operator string().
    struct enable_operator_string_formatting : detail::enable_operator_string_formatting_impl
    {

    };
}


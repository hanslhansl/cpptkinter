module;
#include "../global.hpp"
export module cpptkinter:utility.detail;
import std;
import hhh;
import reflect;


export namespace cpptkinter
{
    class Misc;
}

namespace cpptkinter::detail
{
    template<typename T>
    struct is_std_array : std::false_type
    {

    };
    template<typename T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type
    {

    };

    template<typename Visitor, typename Variant, std::size_t...I>
    consteval bool invokeable_with_variant_types(std::index_sequence<I...>)
    {
        return (std::invocable<Visitor, decltype(std::get<I>(std::declval<Variant>()))> && ...);
    }

    template<typename Callable, typename Variant>
        requires (hhh::meta::is_template_instance<std::remove_cvref_t<Variant>, std::variant>&& invokeable_with_variant_types<Callable, Variant>(std::make_index_sequence<std::variant_size_v<std::remove_cvref_t<Variant>>>{}))
    decltype(auto) visit_or_invoke_impl(Callable&& callable, Variant&& variant)
    {
        return std::visit(std::forward<Callable>(callable), std::forward<Variant>(variant));
    }
    template<typename Callable, typename T>
        requires (!hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::variant>&& std::invocable<Callable, T>)
    decltype(auto) visit_or_invoke_impl(Callable&& callable, T&& value)
    {
        return std::invoke(std::forward<Callable>(callable), std::forward<T>(value));
    }

    /// @brief Calls *vis* on every element of *val*.
    ///
    /// @tparam T Needs to satisfy @ref container or @ref is_tuple.
    template<typename Visitor, typename T>
        requires std::ranges::range<T> || hhh::meta::tuple_like<T>
    void visit_range_or_tuple_impl(Visitor && vis, T && val)
    {
        if constexpr (std::ranges::range<T>)
        {
            for (auto begin = hhh::misc::forward_begin<T>(val), end = hhh::misc::forward_end<T>(val); begin != end; ++begin)
                std::invoke(std::forward<Visitor>(vis), *begin);
        }
        else if constexpr (hhh::meta::tuple_like<T>)
            std::apply([&]<typename...Args>(Args&&...elem) { (std::invoke(std::forward<Visitor>(vis), std::forward<Args>(elem)), ...); }, std::forward<T>(val));
    }

    template<typename T>
    struct union_arg_overload_base
    {
        static constexpr const T& operator()(const T& t)
        {
            return t;
        }
        static constexpr T& operator()(T& t)
        {
            return t;
        }
    };

    template<typename...Args>
    struct union_arg_overload : union_arg_overload_base<Args>...
    {
        using union_arg_overload_base<Args>::operator()...;

        template<typename T>
        static constexpr auto operator()(T& t) -> std::remove_reference_t<decltype(operator()(t))>
        {
            return t;
        }
    };

    /// @brief Befriended by all widget classes.
    /// 
    /// Used to implement many utility functions that require access to private/protected widget members.
    struct widget_friend
    {
        template<typename T>
        using impl_type = typename T::impl;

        template<typename T>
        static const auto& get_widget_pimpl(const T& widget)
        {
            return widget.pimpl;
        }

        template<typename T>
        static T construct_widget_from_weak_pimpl(const std::weak_ptr<typename T::impl>& pimpl)
        {
            return std::shared_ptr(pimpl);
        }

        template<typename To, typename From>
        static To static_widget_cast_down(const From& from) requires requires { static_cast<typename To::impl*>(from.pimpl.get()); }
        {
            return std::static_pointer_cast<typename To::impl>(from.pimpl);
        }

        template<typename To, typename From>
        static To dynamic_widget_cast_down(const From& from) requires requires { dynamic_cast<typename To::impl*>(from.pimpl.get()); }
        {
            return std::dynamic_pointer_cast<typename To::impl>(from.pimpl);
        }

        template<std::same_as<Misc> Misc, typename...Args>
        static auto misc_bind_class_impl(Misc* self, const std::string& className, Args&&...args)
        {
            return self->_root()._bind({ "bind", className }, std::forward<Args>(args)..., true);
        }
    };

    struct enable_operator_string_formatting_impl
    {

    };

    /// @brief For some very strange reason msvc fails if calling std::format on this tuple from inside cpptkinter.ixx file
    std::string format_tuple(const std::tuple<std::string, long long, std::string, long long, std::string, std::string, std::string, long long>& tup)
    {
        return std::format("{}", tup);
    }
}

template<std::derived_from<cpptkinter::detail::enable_operator_string_formatting_impl> T>
    requires requires(const T& t) { static_cast<std::string>(t); }
struct std::formatter<T>
{
    // Parses the format specifier (optional)
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    // Formats the object
    template<typename FormatContext>
    auto format(const T& value, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", static_cast<std::string>(value));
    }
};
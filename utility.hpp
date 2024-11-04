#pragma once
#include "global.hpp"


/// @brief Implementation of the Python module tkinter in C++.
namespace cpptkinter
{
    class Misc;
}

/// @brief Utilities that aren't related to Python's tkinter or _tkinter.
namespace cpptkinter::utility
{
    namespace detail
    {
        template<typename T>
        struct is_std_array : std::false_type
        {

        };
        template<typename T, size_t N>
        struct is_std_array<std::array<T, N>> : std::true_type
        {

        };
    }

    /// @brief Checks if T is std::vector.
    template<typename T>
    concept is_vector = hhh::meta::is_template_instance<T, std::vector>;

    namespace detail
    {
        template<typename Aggr, template<typename...> typename Trait, typename...Args, std::size_t...I>
			requires std::is_aggregate_v<Aggr>
        consteval bool check_trait_for_all_aggregate_members(std::index_sequence<I...>) {
            return (Trait<decltype(reflect::get<I>(std::declval<Aggr>())), Args...>::value && ...);
        }
    }

    template<typename Aggr, template<typename...> typename Trait, typename...Args>
    concept aggregate_members_satisfy = std::is_aggregate_v<Aggr> && detail::check_trait_for_all_aggregate_members<Aggr, Trait, Args...>(std::make_index_sequence<reflect::size<Aggr>()>{});

    namespace detail
    {
        template<typename Visitor, typename Variant, size_t...I>
        consteval bool invokeable_with_variant_types(std::index_sequence<I...>)
        {
			return (std::invocable<Visitor, decltype(std::get<I>(std::declval<Variant>()))> && ...);
        }
    }

	/// @brief Invokes a callable with a variant's currently held alternative (equal to std::visit).
	/// 
	/// @param callable a functor.
	/// @param variant a variant.
	template<typename Callable, typename Variant>
        requires (hhh::meta::is_template_instance<std::remove_cvref_t<Variant>, std::variant> && detail::invokeable_with_variant_types<Callable, Variant>(std::make_index_sequence<std::variant_size_v<std::remove_cvref_t<Variant>>>{}))
    decltype(auto) visit_or_invoke(Callable&& callable, Variant&& variant)
    {
        return std::visit(std::forward<Callable>(callable), std::forward<Variant>(variant));
    }
    /// @brief Invokes a callable with a value (equal to std::invoke).
    /// 
    /// @param callable a functor.
    /// @param value a value.
    template<typename Callable, typename T>
        requires (!hhh::meta::is_template_instance<std::remove_cvref_t<T>, std::variant> && std::invocable<Callable, T>)
    decltype(auto) visit_or_invoke(Callable&& callable, T&& value)
    {
        return std::invoke(std::forward<Callable>(callable), std::forward<T>(value));
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

	/// @brief Calls *vis* on every element of *val*.
    ///
	/// @tparam T Needs to satisfy @ref container or @ref is_tuple.
    template<typename Visitor, typename T>
        requires hhh::meta::container<std::remove_cvref_t<T>> || hhh::meta::tuple_like<std::remove_cvref_t<T>>
    void visit_container_or_tuple(Visitor&& vis, T&& val)
    {
        if constexpr (hhh::meta::container<std::remove_cvref_t<T>>)
        {
            for (auto begin = hhh::misc::forward_begin<T>(val), end = hhh::misc::forward_end<T>(val); begin != end; ++begin)
                std::invoke(std::forward<Visitor>(vis), *begin);
        }
        else if constexpr (hhh::meta::tuple_like<std::remove_cvref_t<T>>)
            std::apply([&]<typename...Args>(Args&&...elem) { (std::invoke(std::forward<Visitor>(vis), std::forward<Args>(elem)), ...); }, std::forward<T>(val));
    }

    namespace detail
    {
        struct container_or_tuple_to_string_visitor
        {
            std::ostringstream oss{};

			template<typename T>
			void operator()(const T& val)
			{
                if constexpr ((hhh::meta::container<T> && !std::same_as<T, std::string>) || hhh::meta::tuple_like<T>)
                {
                    this->oss << "( ";
                    visit_container_or_tuple(*this, val);
                    this->oss << ")";
                }
                else
                {
                    this->oss << "'";
                    visit_or_invoke([this](const auto& val) { this->oss << val; }, val);
                    this->oss << "'";
                }
                this->oss << ", ";
			}
        };
    }

    template<typename T>
        requires hhh::meta::container<std::remove_cvref_t<T>> || hhh::meta::tuple_like<std::remove_cvref_t<T>>
    std::string container_or_tuple_to_string(const T& val)
    {
        detail::container_or_tuple_to_string_visitor vis{};
        vis(val);
        return vis.oss.str();
    }

    template<typename T>
        requires std::is_aggregate_v<T>
    std::string aggregate_to_string(const T & val)
    {
        detail::container_or_tuple_to_string_visitor visitor{};

        visitor.oss << "{ ";
        reflect::for_each<T>([&visitor, &val](auto I) { visitor.oss << rfl::fields<T>()[I].name()/*reflect::member_name<I, T>()*/ << " : "; visitor(reflect::get<I>(val)); });
        visitor.oss << "}";
        return visitor.oss.str();
    }

	/// @brief A weak reference to a widget.
    ///
	/// Holds a weak reference to a widget of type T (e.g. Tk, Button, etc.). Basically works like std::weak_ptr.
    template<typename T>
    class weak
    {
        std::weak_ptr<typename T::impl> pimpl;
    public:
        weak() = default;
        weak(const T& obj) : pimpl(std::static_pointer_cast<typename T::impl>(obj.pimpl))
        {

        }

		/// @brief Creates a strong reference to the widget.
        /// 
		/// Eequivalent to std::weak_ptr::lock except that this function throws if the widget has already been destroyed.
        T lock() const
        {
            return std::shared_ptr(pimpl);
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

	/// @brief Like std::reference_wrapper but can be constructed from rvalues (temporaries).
    ///
	/// Use with caution, as this can easily lead to dangling references.
    template<typename T>
		requires std::same_as<T, std::remove_cvref_t<T>>
    class ref_wrapper
    {
    public:
        using type = const T;
    private:
        const type* ptr;

        static void FUN(type&) noexcept;
    public:

        template<class U>
            requires (!std::same_as<typename std::decay<U>::type, ref_wrapper>)
        ref_wrapper(U&& x) noexcept(noexcept(FUN(std::declval<U>())))
        {
            type& t = std::forward<U>(x);
			this->ptr = &t;
        }
        ref_wrapper(const ref_wrapper& other) noexcept : ptr(&other.get())
        {

        }

        operator type& () const noexcept
        {
            return *this->ptr;
        }
        type& get() const noexcept
        {
            return *this->ptr;
        }
    };
    template<typename T>
    ref_wrapper(T&&) -> ref_wrapper<std::conditional_t<std::same_as<T, std::remove_cvref_t<T>&>, std::remove_cvref_t<T>, const std::remove_cvref_t<T>>>;

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
}
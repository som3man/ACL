module;

#include <type_traits>
#include <utility>

export module ACL.Main:Traits;

namespace ACL {
    export template <typename TypeA, typename TypeB>
    constexpr bool areSame = std::is_same_v<TypeA, TypeB>;

    export template <typename Type>
    constexpr bool isConst = std::is_const_v<Type>;

    export template <typename Type>
    constexpr bool isReference = std::is_reference_v<Type>;

    export template <typename Type>
    constexpr bool isInteger = std::is_integral_v<Type>;

    export template <typename Type>
    constexpr bool isFloat = std::is_floating_point_v<Type>;

    export template <typename Type>
    constexpr bool isUnsigned = std::is_unsigned_v<Type>;

    export template <typename Type, typename... Args>
    constexpr bool hasConstructor = std::is_constructible_v<Type, Args...>;

    export template <typename Type>
    constexpr bool hasTrivialDestructor = std::is_trivially_destructible_v<Type>;

    export template <typename Type>
    constexpr bool hasVirtualDestructor = std::has_virtual_destructor_v<Type>;

    export template <typename Type, typename Other>
    constexpr bool hasAssignment = std::is_assignable_v<Type&, Other>;

    export template <typename Type, typename Other>
    constexpr bool hasTrivialAssignment = std::is_trivially_assignable_v<Type&, Other>;

    export template <typename Type, typename... Args>
    constexpr bool isInvocable = std::is_invocable_v<Type, Args...>;

    export template <typename Return, typename Type, typename... Args>
    constexpr bool isInvocableNodiscard = std::is_invocable_r_v<Return, Type, Args...>;

    export template <typename Type, typename... Args> requires(isInvocable<Type, Args...>)
    using ReturnType = std::invoke_result_t<Type, Args...>;

    export template <typename Type>
    using NoRef = std::remove_reference_t<Type>;

    export template <typename Type>
    using NoConst = std::remove_const_t<Type>;

    export template <typename Type>
    using NoConstRef = std::remove_cvref_t<Type>;

    export [[nodiscard]] constexpr auto&& Move(auto&& data) noexcept {
        return std::move(data);
    }

    export template <typename Type>
    [[nodiscard]] constexpr Type&& Forward(Type& data) noexcept {
        return std::forward<Type>(data);
    }

    export template <class Type>
	concept Swapable = requires (Type object, Type& ref) {
		{ object.Swap(ref) };
	};

	export template <Swapable Type>
	constexpr void Swap(Type& left, Type& right) noexcept {
		left.Swap(right);
	}

	export template <typename Type>
	constexpr void Swap(Type& left, Type& right) noexcept {
		std::swap(left, right);
	}
}
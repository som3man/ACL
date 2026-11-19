module;

export module ACL.Main:Tuple;

import :Optional;
import :Union;
import :Traits;
import :Types;

namespace ACL {
    template <u32 index, typename Type, typename... Other>
    struct TupleLeaf {
        using Next = TupleLeaf<index + 1, Other...>;

        Type data;

        Next next;

        template <typename Target>
        static consteval bool has() noexcept {
            if (areSame<Target, Type>)
                return true;
            else
                return Next::template has<Target>();
        }

        constexpr TupleLeaf() requires(hasConstructor<Type>) : data(), next() {};

        template <typename Value, typename... OtherValues>
        constexpr TupleLeaf(Value&& value, OtherValues&&... other) : 
            data(Forward(value)), next(Forward<OtherValues>(other)...) {};

        constexpr TupleLeaf(const TupleLeaf& other) : data(other.data), next(other.next) {};

        constexpr TupleLeaf(TupleLeaf&& other) : data(Move(other.data)), next(Move(other.next)) {};

        constexpr ~TupleLeaf() {};

        constexpr TupleLeaf& operator =(const TupleLeaf& other) {
            data = other.data;

            next = other.next;

            return *this;
        }

        constexpr TupleLeaf& operator =(TupleLeaf&& other) {
            data = Move(other.data);

            next = Move(other.next);

            return *this;
        }

        template <typename Target>
        constexpr Target& get() noexcept {
            if constexpr (areSame<Target, Type>)
                return data;
            else
                return next.template get<Target>();
        }

        template <typename Target>
        constexpr const Target& get() const noexcept {
            if constexpr (areSame<Target, Type>)
                return data;
            else
                return next.template get<Target>();
        }

        template <typename... Types>
        constexpr PolyPtr<Types...> getPtr(u32 target) noexcept {
            if (target == index)
                return data;
            else
                return next.template getPtr<Types...>(target);
        }

        template <typename... Types>
        constexpr PolyPtr<Types...> getPtr(u32 target) const noexcept {
            if (target == index)
                return data;
            else
                return next.template getPtr<Types...>(target);
        }
    };

    template <u32 index, typename Type>
    struct TupleLeaf<index, Type> {
        Type data;

        template <typename Target>
        static consteval bool has() noexcept {
            return areSame<Target, Type>;
        }

        constexpr TupleLeaf() requires(hasConstructor<Type>) : data() {};

        template <typename Value>
        constexpr TupleLeaf(Value&& value) : data(Forward(value)) {};

        constexpr TupleLeaf(const TupleLeaf& other) : data(other.data) {};

        constexpr TupleLeaf(TupleLeaf&& other) : data(Move(other.data)) {};

        constexpr ~TupleLeaf() {};

        constexpr TupleLeaf& operator =(const TupleLeaf& other) {
            data = other.data;

            return *this;
        }

        constexpr TupleLeaf& operator =(TupleLeaf&& other) {
            data = Move(other.data);

            return *this;
        }

        template <typename Target>
        constexpr Target& get() noexcept {
            return data;
        }

        template <typename Target>
        constexpr const Target& get() const noexcept {
            return data;
        }

        template <typename... Types>
        constexpr PolyPtr<Types...> getPtr(u32 target) noexcept {
            if (target == index)
                return data;
            else
                return {};
        }

        template <typename... Types>
        constexpr PolyPtr<Types...> getPtr(u32 target) const noexcept {
            if (target == index)
                return data;
            else
                return {};
        }
    };

    export template <typename... Types> requires(sizeof...(Types) > 1 && sizeof...(Types) <= 15)
    class Tuple {
        using Data = TupleLeaf<0, Types...>;

        Data data;

        template <typename Type>
        static consteval bool has() noexcept {
            return Data::template has<Type>();
        }

    public:
        constexpr Tuple() : data() {};

        template <typename... Values> requires(sizeof...(Values) == sizeof...(Types))
        constexpr Tuple(Values&&... values) : data(Forward<Values>(values)...) {};

        constexpr Tuple(const Tuple& other) : data(other.data) {};

        constexpr Tuple(Tuple&& other) : data(Move(other.data)) {};

        constexpr ~Tuple() {};

        constexpr Tuple& operator =(const Tuple& other) {
            data = other.data;

            return *this;
        }

        constexpr Tuple& operator =(Tuple&& other) {
            data = Move(other.data);

            return *this;
        }

        template <typename Type> requires(has<Type>())
        [[nodiscard]] constexpr Type& get() noexcept {
            return data.template get<Type>();
        }

        template <typename Type> requires(has<Type>())
        [[nodiscard]] constexpr const Type& get() const noexcept {
            return data.template get<Type>();
        }

        [[nodiscard]] constexpr PolyPtr<Types...> operator [](u32 index) noexcept {
            return data.template getPtr<Types...>(index);
        }

        [[nodiscard]] constexpr PolyPtr<const Types...> operator [](u32 index) const noexcept {
            return data.template getPtr<const Types...>(index);
        }
    };

    consteval bool TupleTest() {
        Tuple<int, float, bool> tuple = {};

        tuple = {
            0, 5.0f, true
        };

        if (tuple.get<int>() != 0 || tuple.get<float>() != 5.0f || tuple.get<bool>() != true)
            return false;

        if (&*tuple[0].get<int>() != &tuple.get<int>() || &*tuple[2].get<bool>() != &tuple.get<bool>())
            return false;

        return true;
    }

    static_assert(TupleTest());
}
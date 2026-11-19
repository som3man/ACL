module;

#include <memory>
#include <ostream>

export module ACL.Main:Union;

import :Optional;
import :Traits;

namespace ACL {
    template <char index, typename Type, typename... Other> requires(!isConst<Type>)
    union UnionLeaf {
        using Next = UnionLeaf<index + 1, Other...>;

        Type data;

        Next next;

        template <typename Target> requires(areSame<Type, NoConstRef<Target>>)
        constexpr UnionLeaf(Target&& value) : data(Forward(value)) {};

        template <typename Target>
        constexpr UnionLeaf(Target&& value) : next(Forward(value)) {};

        constexpr ~UnionLeaf() noexcept {};

        template <typename Target>
        static consteval char getIndex() noexcept {
            if constexpr (areSame<Target, Type>)
                return index;
            else
                return Next::template getIndex<Target>();
        }

        constexpr void Destruct(char target) {
            if (target == index)
                data.~Type();
            else
                next.Destruct(target);
        }

        template <typename Target>
        constexpr Optional<Target&> get(char typeIndex) noexcept {
            if constexpr (areSame<Target, Type>) {
                if (index == typeIndex)
                    return data;
                else
                    return {};
            }
            else {
                return next.template get<Target>(typeIndex);
            }
        }

        template <typename Target>
        constexpr Optional<const Target&> get(char typeIndex) const noexcept {
            if constexpr (areSame<Target, Type>) {
                if (index == typeIndex)
                    return data;
                else
                    return {};
            }
            else {
                return next.template get<Target>(typeIndex);
            }
        }

        template <typename Return, typename Func, typename... OtherFunc> requires(isInvocableNodiscard<Return, Func, Type&>)
        constexpr Return Visit(char typeIndex, Func&& func, OtherFunc&&... other) {
            if (index == typeIndex)
                return func(data);
            else
                return next.template Visit<Return>(typeIndex, Forward<OtherFunc>(other)...);
        }

        template <typename Return, typename Func, typename... OtherFunc> requires(isInvocableNodiscard<Return, Func, const Type&>)
        constexpr Return Visit(char typeIndex, Func&& func, OtherFunc&&... other) const {
            if (index == typeIndex)
                return func(data);
            else
                return next.template Visit<Return>(typeIndex, Forward<OtherFunc>(other)...);
        }
    };

    template <char index, typename Type> requires(!isConst<Type>)
    union UnionLeaf<index, Type> {
        Type data;

        template <typename Target>
        constexpr UnionLeaf(Target&& value) : data(Forward(value)) {};

        constexpr ~UnionLeaf() noexcept {};

        template <typename Target>
        static consteval char getIndex() noexcept {
            if constexpr (areSame<Target, Type>)
                return index;
            else
                return -1;
        }

        constexpr void Destruct(char target) {
            data.~Type();
        }

        template <typename Target>
        constexpr Optional<Target&> get(char typeIndex) noexcept {
            if (index == typeIndex)
                return data;
            else
                return {};
        }

        template <typename Target>
        constexpr Optional<const Target&> get(char typeIndex) const noexcept {
            if (index == typeIndex)
                return data;
            else
                return {};
        }

        template <typename Return, typename Func> requires(isInvocableNodiscard<Return, Func, Type&>)
        constexpr Return Visit(char typeIndex, Func&& func) {
            if (index == typeIndex)
                return func(data);

            throw 0;
        }

        template <typename Return, typename Func> requires(isInvocableNodiscard<Return, Func, const Type&>)
        constexpr Return Visit(char typeIndex, Func&& func) const {
            if (index == typeIndex)
                return func(data);

            throw 0;
        }
    };

    export template <typename... Types> requires(sizeof...(Types) > 1 && sizeof...(Types) <= 16)
    class Union {
        using Data = UnionLeaf<0, Types...>;

        union {
            Data data;
        };

        char typeIndex;

        template <typename Target>
        static consteval char getIndex() noexcept {
            return Data::template getIndex<NoConstRef<Target>>();
        }

        template <typename Target>
        static consteval bool has() noexcept {
            return getIndex<Target>() != -1;
        }

        constexpr void Destruct() {
            data.Destruct(typeIndex);

            data.~Data();
        }

    public:
        friend Optional<Union<Types...>>;

        Union() = delete;

        template <typename Type>
        constexpr Union(Type&& value) : typeIndex(getIndex<Type>()) {
            std::construct_at(&data, Forward(value));
        }   

        constexpr Union(const Union& other) {
            other.Visit([&](const Types& value) {
                typeIndex = getIndex<Types>();

                std::construct_at(&data, value);
            }...);
        }

        constexpr Union(Union&& other) {
            other.Visit([&](Types& value) {
                typeIndex = getIndex<Types>();

                std::construct_at(&data, Move(value));
            }...);
        }

        constexpr ~Union() {
            Destruct();
        }

        constexpr Union& operator =(const Union& other) {
            other.Visit([&](const Types& value) {
                if (is<Types>())
                    *get<Types>() = value;
                else
                    Set<Types>(value);
            }...);

            return *this;
        }

        constexpr Union& operator =(Union&& other) {
            other.Visit([&](Types& value) {
                if (is<Types>())
                    *get<Types>() = Move(value);
                else
                    Set<Types>(Move(value));
            }...);

            return *this;
        }

        template <typename Type, typename... Args> requires(has<Type>() && hasConstructor<Type, Args...> && hasConstructor<Type, Type&&>)
        constexpr Type& Set(Args&&... args) {
            Destruct();

            typeIndex = getIndex<Type>();

            Type value(Forward<Args>(args)...);

            std::construct_at(&data, Move(value));

            return get<Type>();
        }

        template <typename Type> requires(has<Type>())
        [[nodiscard]] constexpr Optional<Type&> get() noexcept {
            return data.template get<Type>(typeIndex);
        }

        template <typename Type> requires(has<Type>())
        [[nodiscard]] constexpr Optional<const Type&> get() const noexcept {
            return data.template get<Type>(typeIndex);
        }

        template <typename Return = void, typename... Funcs> requires(sizeof...(Funcs) == sizeof...(Types))
        constexpr Return Visit(Funcs&&... funcs) {
            return data.template Visit<Return>(typeIndex, Forward<Funcs>(funcs)...);
        }
        
        template <typename Return = void, typename... Funcs> requires(sizeof...(Funcs) == sizeof...(Types))
        constexpr Return Visit(Funcs&&... funcs) const {
            return data.template Visit<Return>(typeIndex, Forward<Funcs>(funcs)...);
        }

        template <typename Type, typename Func> requires(has<Type>() && isInvocable<Func, Type&>)
        constexpr Union& For(Func&& func) {
            Visit([&](Types& value) {
                if constexpr (areSame<Types, Type>)
                    func(value);
            }...);

            return *this;
        }

        template <typename Type, typename Func> requires(has<Type>() && isInvocable<Func, const Type&>)
        constexpr const Union& For(Func&& func) const {
            Visit([&](const Types& value) {
                if constexpr (areSame<Types, Type>)
                    func(value);
            }...);

            return *this;
        }

        template <typename Type> requires(has<Type>())
        [[nodiscard]] constexpr bool is() const noexcept {
            return typeIndex == getIndex<Type>();
        }

        [[nodiscard]] constexpr bool operator ==(const Union& other) const noexcept {
            return typeIndex == other.typeIndex;
        }

        [[nodiscard]] constexpr bool operator !=(const Union& other) const noexcept {
            return typeIndex != other.typeIndex;
        }
    };

    export template <typename Char, typename... Types>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Union<Types...>& data) {
        data.Visit([&](const Types& value) {
            ostream << value;
        }...);

        return ostream;
    }

    export template <typename Type, typename Error>
    class Result {
        Union<Type, Error> data;

    public:
        constexpr Result(Type&& success) : data(Forward(success)) {};

        constexpr Result(Error&& error) : data(Forward(error)) {};

        constexpr Result(const Result& other) : data(other.data) {};

        constexpr Result(Result&& other) : data(Move(other.data)) {};

        constexpr ~Result() {};

        constexpr Result& operator =(const Result& other) {
            data = other.data;

            return *this;
        }

        constexpr Result& operator =(Result&& other) {
            data = other.data;

            return *this;
        }

        [[nodiscard]] constexpr Optional<Type&> get() noexcept {
            return data.template get<Type>();
        }

        [[nodiscard]] constexpr Optional<const Type&> get() const noexcept {
            return data.template get<Type>();
        }

        [[nodiscard]] constexpr Optional<Error&> error() noexcept {
            return data.template get<Error>();
        }

        [[nodiscard]] constexpr Optional<const Error&> error() const noexcept {
            return data.template get<Error>();
        }

        [[nodiscard]] constexpr bool isSuccess() const noexcept {
            return data.template is<Type>();
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return isSuccess();
        }

        [[nodiscard]] constexpr operator Type& () {
            return *get();
        }

        [[nodiscard]] constexpr operator const Type& () const {
            return *get();
        }

        [[nodiscard]] constexpr Type& operator *() {
            return *get();
        }

        [[nodiscard]] constexpr const Type& operator *() const {
            return *get();
        }

        [[nodiscard]] constexpr Type* operator ->() {
            return &*get();
        }

        [[nodiscard]] constexpr const Type* operator ->() const {
            return &*get();
        }

        template <typename Func> requires(isInvocable<Func, Type&>)
        constexpr Result& Use(Func&& func) {
            if (*this)
                func(*get());

            return *this;
        }

        template <typename Func> requires(isInvocable<Func, const Type&>)
        constexpr const Result& Use(Func&& func) const {
            if (*this)
                func(*get());

            return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Result, Func, Type&>)
        constexpr Result Then(Func&& func) {
            if (*this)
                return func(*get());
            else
                return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Result, Func, const Type&>)
        constexpr Result Then(Func&& func) const {
            if (*this)
                return func(*get());
            else
                return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Result, Func, Error&>)
        constexpr Result Else(Func&& func) {
            if (*this)
                return *this;
            else
                return func(*error());
        }

        template <typename Func> requires(isInvocableNodiscard<Result, Func, const Error&>)
        constexpr Result Else(Func&& func) const {
            if (*this)
                return *this;
            else
                return func(*error());
        }
    };

    export template <typename Char, typename Type, typename Error>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Result<Type, Error>& result) {
        result.Use([&](const Type& value) {
            ostream << value;
        }).Else([&](const Error& error) -> Result<Type, Error> {
            ostream << error;

            return {};
        });

        return ostream;
    }

    export template <typename... Types>
    class PolyPtr {
        struct Nothing {};

        Union<Nothing, Types*...> data;

    public:
        constexpr PolyPtr() noexcept : data(Nothing()) {};

        template <typename Type>
        constexpr PolyPtr(Type& reference) noexcept : data(&reference) {};

        constexpr PolyPtr(const PolyPtr&) noexcept = default;

        constexpr PolyPtr(PolyPtr&&) noexcept = default;

        constexpr ~PolyPtr() noexcept = default;

        constexpr PolyPtr& operator =(const PolyPtr& other) noexcept = default;

        constexpr PolyPtr& operator =(PolyPtr&& other) noexcept = default;

        template <typename Type>
        [[nodiscard]] constexpr Optional<Type&> get() const noexcept {
            if (is<Type>())
                return *static_cast<Type*>(data.template get<Type*>());

            return {};
        }

        template <typename Type>
        [[nodiscard]] constexpr bool is() const noexcept {
            return data.template is<Type*>();
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return data.template is<Nothing>();
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return !isVoid();
        }

        template <typename Return = void, typename Func, typename... Other>
        constexpr Return Visit(Func&& func, Other&&... other) const {
            return data.template Visit<Return>(
                [&](const Nothing&) -> Return {
                    return func();
                },
                Forward<Other>(other)...
            );
        }

        template <typename Type, typename Func> requires(isInvocable<Func, Type&>)
        constexpr PolyPtr& For(Func&& func) {
            if (is<Type>())
                func(*get<Type>());

            return *this;
        }

        template <typename Type, typename Func> requires(isInvocable<Func, Type&>)
        constexpr const PolyPtr& For(Func&& func) const {
            if (is<Type>())
                func(*get<Type>());

            return *this;
        }

        template <typename Func> requires(isInvocable<Func>)
        constexpr PolyPtr& ForVoid(Func&& func) {
            if (isVoid())
                func();

            return *this;
        }

        template <typename Func> requires(isInvocable<Func>)
        constexpr const PolyPtr& ForVoid(Func&& func) const {
            if (isVoid())
                func();

            return *this;
        }
    };

    export template <typename Char, typename... Types>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const PolyPtr<Types...>& polyPtr) {
        polyPtr.Visit(
            [&]() {
                ostream << "void";
            },
            [&](const Types& value) {
                ostream << value;
            }...
        );

        return ostream;
    }

    consteval bool UnionTest() {
        Union<int, float, bool> inst = false;

        if (!inst.is<bool>() || *inst.get<bool>() != false)
            return false;

        inst = 20;

        if (!inst.is<int>() || *inst.get<int>() != 20)
            return false;

        Union<int, float, bool> inst2 = -1.0f;

        inst = inst2;

        if (!inst.is<float>() || *inst.get<float>() != -1.0f)
            return false;

        const Union<int, float, bool> inst3 = false;

        inst3.Visit(
            [&](const int&) {

            },
            [&](float) {

            },
            [&](bool) {

            }
        );

        return true;
    }

    static_assert(UnionTest());
}
module;

#include <memory>
#include <iostream>

export module ACL.Main:Optional;

export import :Traits;

namespace ACL {
    export template <typename Type>
    class Optional;

    export template <typename Type>
    constexpr bool isOptional = false;

    export template <typename Type>
    constexpr bool isOptional<Optional<Type>> = true;

    // Exception type which is thrown trying to access memory by invalid reference.
    export struct AccessError {};

    export template <typename Type>
    class Optional {
        union {
            Type data;
        };

        bool initialized = false;

    public:
        constexpr Optional() noexcept {};

        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Optional(Args&&... args) {
            initialized = true;

            std::construct_at(&data, Forward<Args>(args)...);
        }

        constexpr Optional(const Optional& other) requires(hasConstructor<Type, const Type&>) {
            if (!other)
                return;

            initialized = true;

            std::construct_at(&data, other.data);
        }

        constexpr Optional(Optional&& other) requires(hasConstructor<Type, Type&&>) {
            if (!other)
                return;

            initialized = true;

            std::construct_at(&data, Move(other.data));
        }

        constexpr ~Optional() {
            if (initialized)
                data.~Type();
        }

        constexpr Optional& operator =(const Optional& other) requires(hasConstructor<Type, const Type&> && hasAssignment<Type, const Type&>) {
            other.Use([&](const Type& value) {
                if (initialized) {
                    data = value;
                }
                else {
                    initialized = true;

                    std::construct_at(&data, value);
                }
            }).Else([&]() -> Optional {
                if (initialized) {
                    initialized = false;

                    data.~Type();
                }

                return {};
            });

            return *this;
        }

        constexpr Optional& operator =(Optional&& other) requires(hasConstructor<Type, Type&&> && hasAssignment<Type, Type&&>) {
            other.Use([&](Type& value) {
                if (initialized) {
                    data = Move(value);
                }
                else {
                    initialized = true;

                    std::construct_at(&data, Move(value));
                }
            }).Else([&]() -> Optional {
                if (initialized) {
                    initialized = false;

                    data.~Type();
                }

                return {};
            });

            return *this;
        }

        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Type& Construct(Args&&... args) {
            Destruct();

            initialized = true;

            std::construct_at(&data, Forward<Args>(args)...);

            return data;
        }

        constexpr void Destruct() {
            if (initialized) {
                data.~Type();

                initialized = false;
            }
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return !initialized;
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return initialized;
        }

        // Calls func if Optional is not void.
        template <typename Func> requires(isInvocable<Func, Type&>)
        constexpr Optional& Use(Func&& func) {
            if (initialized)
                func(data);

            return *this;
        }

        // Calls func if Optional is not void.
        template <typename Func> requires(isInvocable<Func, const Type&>)
        constexpr const Optional& Use(Func&& func) const {
            if (initialized)
                func(data);

            return *this;
        }

        // Calls func and returns its result if Optional is not void.
        // Copies Optional if it is void.
        template <typename Func> requires(isInvocableNodiscard<Optional, Func, Type&>)
        constexpr Optional Then(Func&& func) {
            if (initialized)
                return func(data);
            else
                return *this;
        }

        // Calls func and returns its result if Optional is not void.
        // Copies Optional if it is void.
        template <typename Func> requires(isInvocableNodiscard<Optional, Func, const Type&>)
        constexpr Optional Then(Func&& func) const {
            if (initialized)
                return func(data);
            else
                return *this;
        }

        // Calls func and returns its result if Optional is void.
        // Copies Optional if it is not void.
        template <typename Func> requires(isInvocableNodiscard<Optional, Func>)
        constexpr Optional Else(Func&& func) const {
            if (initialized)
                return *this;
            else
                return func();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr Type& get() {
            if (initialized)
                return data;

            throw AccessError();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr const Type& get() const {
            if (initialized)
                return data;

            throw AccessError();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr operator Type& () {
            return get();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr operator const Type& () const {
            return get();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr Type& operator *() {
            return get();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr const Type& operator *() const {
            return get();
        }

        // Returns pointer to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr Type* operator ->() {
            return &get();
        }

        // Returns pointer to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr const Type* operator ->() const {
            return &get();
        }
    };

    export template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Optional<Type>& optional) {
        if (optional)
            return ostream << *optional;
        else
            return ostream << "void";
    }

    template <typename Type>
    class Optional<Type&> {
        Type* pointer = nullptr;

    public:
        constexpr Optional() noexcept {};

        constexpr Optional(const Optional&) noexcept = default;

        constexpr Optional(Optional&&) noexcept = default;

        constexpr ~Optional() noexcept = default;

        constexpr Optional& operator =(const Optional&) noexcept = default;

        constexpr Optional& operator =(Optional&&) noexcept = default;

        [[nodiscard]] constexpr operator Optional<const Type&>() const noexcept requires(!isConst<Type>) {
            if (pointer)
                return *pointer;
            else
                return {};
        }

        constexpr Optional(Type& value) noexcept {
            pointer = &value;
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return !pointer;
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return pointer;
        }

        template <typename Func> requires(isInvocable<Func, Type&>)
        constexpr Optional& Use(Func&& func) {
            if (pointer)
                func(*pointer);

            return *this;
        }

        template <typename Func> requires(isInvocable<Func, Type&>)
        constexpr const Optional& Use(Func&& func) const {
            if (pointer)
                func(*pointer);

            return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Optional, Func, Type&>)
        constexpr Optional Then(Func&& func) const {
            if (pointer)
                return func(*pointer);
            else
                return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Optional, Func>)
        constexpr Optional Else(Func&& func) const {
            if (pointer)
                return *this;
            else
                return func();
        }

        [[nodiscard]] constexpr Type& get() const {
            if (pointer)
                return *pointer;

            throw AccessError();
        }

        [[nodiscard]] constexpr operator Type& () const {
            return get();
        }

        [[nodiscard]] constexpr Type& operator *() const {
            return get();
        }

        [[nodiscard]] constexpr Type* operator ->() const {
            return &get();
        }

        [[nodiscard]] constexpr auto operator <=>(const Optional& other) const noexcept = default;
    };

    consteval bool OptionalTest() {
        class Class {
            int* memory;

        public:
            constexpr Class() : memory(new int(0)) {};

            constexpr Class(int value) : memory(new int(value)) {};

            constexpr Class(const Class& other) : memory(new int(*other.memory)) {};

            constexpr Class(Class&& other) : memory(new int(*other.memory)) {};

            constexpr ~Class() {
                delete memory;
            }

            constexpr Class& operator =(const Class& other) {
                *memory = *other.memory;

                return *this;
            }

            constexpr Class& operator =(Class&& other) {
                *memory = *other.memory;

                return *this;
            }

            constexpr int getValue() const {
                return *memory;
            }
        };

        Optional<Class> optional = {};

        if (optional)
            return false;

        optional.Construct(25);

        if (!optional || optional->getValue() != 25)
            return false;

        optional.Construct(-1);

        if (!optional || optional->getValue() != -1)
            return false;

        optional.Destruct();

        if (optional)
            return false;

        Optional<Class> optional2 = 68;

        optional = optional2;

        if (!optional || optional->getValue() != 68)
            return false;

        return true;
    }

    static_assert(OptionalTest());
}
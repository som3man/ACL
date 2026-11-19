module;

#include <memory>
#include <ostream>

export module ACL.Main:Heap;

import :Optional;
import :Traits;

namespace ACL {
    // Unique heap memory owner.
    export template <typename Type>
    class Heap {
        Type* data = nullptr;

    public:
        constexpr Heap() noexcept {};

        template <typename... Args>
        constexpr Heap(Args&&... args) {
            Allocate(Forward<Args>(args)...);
        }

        constexpr Heap(const Heap& other) {
            if (other)
                Allocate(*other.data);
        }

        constexpr Heap(Heap&& other) noexcept {
            data = other.data;

            other.data = nullptr;
        }

        constexpr ~Heap() {
            Free();
        }

        constexpr Heap& operator =(const Heap& other) {
            if (other) {
                if (data)
                    *data = *other.data;
                else
                    Allocate(*other.data);
            }
            else {
                Free();
            }

            return *this;
        }

        constexpr Heap& operator =(Heap&& other) {
            Free();

            data = other.data;

            other.data = nullptr;

            return *this;
        }

        // Returns true if Heap doesn't own a memory.
        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return !data;
        }

        // Returns true if Heap owns a memory.
        [[nodiscard]] constexpr operator bool() const noexcept {
            return data;
        }

        // Allocates new memory.
        // May reinitialize already allocated memory.
        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Type& Allocate(Args&&... args) {
            if (data)
                data->~Type();
            else {
                std::allocator<Type> alloc = {};

                data = alloc.allocate(1);
            }

            std::construct_at(data, Forward<Args>(args)...);

            return *data;
        }

        // Frees memory.
        constexpr void Free() {
            if (data) {
                data->~Type();

                std::allocator<Type> alloc = {};

                alloc.deallocate(data, 1);

                data = nullptr;
            }
        }

        // Returns Optional of owned memory.
        // Return empty Optional if Heap is void.
        [[nodiscard]] constexpr Optional<Type&> get() const noexcept {
            if (data)
                return *data;
            else
                return {};
        }

        // Returns Optional of owned memory.
        // Return empty Optional if Heap is void.
        [[nodiscard]] constexpr operator Optional<Type&>() const noexcept {
            return get();
        }

        // Returns reference to owned memory.
        // Throws AccessError if Heap is void.
        [[nodiscard]] constexpr operator Type& () const {
            return get();
        }

        // Returns reference to owned memory.
        // Throws AccessError if Heap is void.
        [[nodiscard]] constexpr Type& operator *() const {
            return get();
        }

        // Returns pointer to owned memory.
        // Throws AccessError if Heap is void.
        [[nodiscard]] constexpr Type* operator ->() const {
            return &*get();
        }

        // Compares pointers to memory.
        [[nodiscard]] constexpr auto operator <=>(const Heap& other) const noexcept = default;
    };

    template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Heap<Type>& heap) {
        return ostream << heap.get();
    }

    // Shared heap memory owner.
    export template <typename Type>
    class Shared {
        struct Memory {
            Type data;

            unsigned refCount;

            template <typename... Args>
            constexpr Memory(Args&&... args) : data(Forward<Args>(args)...), refCount(1) {};

            constexpr Memory() noexcept {};
        } *memory = nullptr;

    public:
        constexpr Shared() noexcept {};

        // May share a memory owned by other.
        constexpr Shared(const Shared& other) noexcept {
            if (other) {
                memory = other.memory;

                memory->refCount += 1;
            }
        }

        constexpr Shared(Shared&& other) noexcept {
            memory = other.memory;

            other.memory = nullptr;
        }

        constexpr ~Shared() noexcept {
            Free();
        }

        // May share a memory owned by other.
        // Drops sharing if other is void.
        constexpr Shared& operator =(const Shared& other) {
            Free();

            if (other) {
                memory = other.memory;

                memory->refCount += 1;
            }

            return *this;
        }

        constexpr Shared& operator =(Shared&& other) {
            Free();

            memory = other.memory;

            other.memory = nullptr;

            return *this;
        }

        // Returns true if Shared doesn't own any memory.
        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return !memory;
        }

        // Returns true if Shared owns memory.
        [[nodiscard]] constexpr operator bool() const noexcept {
            return memory;
        }

        template <typename... Args> requires(hasConstructor<Type, Args...>)
        constexpr Type& Allocate(Args&&... args) {
            Free();

            std::allocator<Memory> alloc = {};

            memory = alloc.allocate(1);

            std::construct_at(memory, Forward<Args>(args)...);

            return memory->data;
        }

        constexpr void Free() {
            if (!memory)
                return;

            auto m = memory;

            memory = nullptr;

            m->refCount -= 1;

            if (m->refCount)
                return;

            m->~Memory();

            std::allocator<Memory> alloc = {};

            alloc.deallocate(m, 1);
        }

        // Returns reference to the owned memory.
        // Return empty Optional if Shared is void.
        [[nodiscard]] constexpr Optional<Type&> get() const noexcept {
            if (memory)
                return memory->data;
            else
                return {};
        }

        // Returns reference to the owned memory.
        // Return empty Optional if Shared is void.
        [[nodiscard]] constexpr operator Optional<Type&>() const noexcept {
            return get();
        }

        // Returns reference to the owned memory.
        // throws AccessError if Shared is void.
        [[nodiscard]] constexpr operator Type& () const {
            return get();
        }

        // Returns reference to the owned memory.
        // throws AccessError if Shared is void.
        [[nodiscard]] constexpr Type& operator *() const {
            return get();
        }

        // Returns pointer to the owned memory.
        // throws AccessError if Shared is void.
        [[nodiscard]] constexpr Type* operator ->() const {
            return &*get();
        }

        // Compares pointers to a memory.
        [[nodiscard]] constexpr bool operator ==(const Shared& other) const noexcept {
            return memory == other.memory;
        }

        // Compares pointers to a memory.
        [[nodiscard]] constexpr bool operator !=(const Shared& other) const noexcept {
            return memory != other.memory;
        }
    };

    template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Shared<Type>& shared) {
        return ostream << shared.get();
    }

    consteval bool HeapTest() {
        Heap<int> heap = {};

        heap.Allocate(20);

        if (!heap || *heap.get() != 20)
            return false;

        heap = Heap<int>();

        if (heap || heap.get())
            return false;

        heap = Heap<int>(-1);

        if (!heap || *heap.get() != -1)
            return false;

        return true;
    }

    static_assert(HeapTest());

    consteval bool SharedTest() {
        Shared<int> shared = {};

        if (shared || shared.get())
            return false;

        shared.Allocate(50);

        if (!shared || *shared.get() != 50)
            return false;

        Shared<int> shared2 = shared;

        if (!shared2 || &*shared.get() != &*shared2.get())
            return false;

        shared = Move(shared2);

        if (!shared || shared2)
            return false;

        return true;
    }

    static_assert(SharedTest());
}
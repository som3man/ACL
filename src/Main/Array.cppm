// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

#include <ostream>
#include <memory>

export module ACL.Main:Array;

import :Heap;
import :Optional;
import :Traits;
import :Types;
import :Range;

namespace ACL {
    // Dynamic array.
    export template <typename Type> requires(!isConst<Type>)
    class Array {
        Type* data = nullptr;

        size length = 0;

        size capacity = 0;

        constexpr void ShiftRight(size index, size amount, bool destruct = true) {
            size newLength = length + amount;

            if (capacity * 2 < newLength)
                Expand(newLength - length);
            else if (capacity < newLength)
                Expand();

            for (size i = 1; i <= amount; i++)
                std::construct_at(data + newLength - i, Move(data[length - i]));

            for (size i = length - 1; i >= index + amount; i--) {
                if constexpr (hasAssignment<Type, Type&&>) {
                    data[i] = Move(data[i - amount]);
                }
                else {
                    data[i].~Type();

                    std::construct_at(data + i, Move(data[i - amount]));
                }
            }

            if (destruct)
                for (size i = index; i < index + amount; i++)
                    data[i].~Type();

            length = newLength;
        }

        constexpr void ShiftLeft(size index, size amount, bool destruct = true) {
            size newLength = length - amount;

            size newIndex = index - amount;

            for (size i = newIndex; i < newLength; i++) {
                if constexpr (hasAssignment<Type, Type&&>) {
                    data[i] = Move(data[i + amount]);
                }
                else {
                    data[i].~Type();

                    std::construct_at(data + i, Move(data[i + amount]));
                }
            }

            for (size i = newLength; i < length; i++)
                data[i].~Type();

            length = newLength;
        }

        template <typename Val, typename... Other>
        constexpr void InitItems(size index, Val&& val, Other&&... other) {
            std::construct_at(data + index, Forward(val));

            if constexpr (sizeof...(Other) != 0)
                InitItems(index + 1, Forward<Other>(other)...);
        }

    public:
        friend Optional<Array<Type>>;

        constexpr Array() noexcept {};

        constexpr Array(Type&& value) {
            EmplaceBack(Forward(value));
        }

        template <typename... Vals> requires(sizeof...(Vals) > 1)
        constexpr Array(Vals&&... vals) {
            Allocate(sizeof...(Vals));

            InitItems(0, Forward<Vals>(vals)...);

            length = sizeof...(Vals);
        }

        template <size capacity2>
        constexpr Array(const Type (&array)[capacity2]) {
            Allocate(capacity2);

            while (length < capacity2) {
                std::construct_at(data + length, array[length]);

                length += 1;
            }
        }

        template <size capacity2>
        constexpr Array(Type (&&array)[capacity2]) {
            Allocate(capacity2);

            while (length < capacity2) {
                std::construct_at(data + length, Move(array[length]));

                length += 1;
            }
        }

        constexpr Array(const Range<const Type>& other) {
            Allocate(other.getLength());

            while (length < other.getLength()) {
                std::construct_at(data + length, other.begin()[length]);

                length += 1;
            }
        }

        constexpr Array(const Range<Type>&& other) {
            Allocate(other.getLength());

            while (length < other.getLength()) {
                std::construct_at(data + length, Move(other.begin()[length]));

                length += 1;
            }
        }

        constexpr Array(const Array& other) {
            Allocate(other.length);

            while (length < other.length) {
                std::construct_at(data + length, other.data[length]);

                length += 1;
            }
        }

        constexpr Array(Array&& other) noexcept {
            Swap(data, other.data);

            Swap(capacity, other.capacity);

            Swap(length, other.length);
        }

        constexpr ~Array() {
            Free();
        }

        constexpr Array& operator =(const Array& other) {
            if (capacity * 2 < other.getLength())
                Expand(other.getLength() - capacity);
            else if (capacity < other.getLength())
                Expand();

            if constexpr (!hasAssignment<Type, const Type&>) {
                Clear();

                while (length < other.getLength()) {
                    std::construct_at(data + length, other.begin()[length]);

                    length += 1;
                }
            }
            else {
                if (length < other.getLength()) {
                    for (size i = 0; i < length; i++)
                        data[i] = other.begin()[i];

                    for (size i = length; i < other.getLength(); i++)
                        std::construct_at(data + i, other.begin()[i]);
                }
                else {
                    for (size i = 0; i < other.getLength(); i++)
                        data[i] = other.begin()[i];

                    for (size i = other.getLength(); i < length; i++)
                        data[i].~Type();
                }

                length = other.getLength();
            }

            return *this;
        }

        constexpr Array& operator =(Array&& other) {
            Free();

            Swap(data, other.data);

            Swap(capacity, other.capacity);

            Swap(length, other.length);

            return *this;
        }

        // Frees old and allocates new memory.
        constexpr void Allocate(size capacity) {
            Free();

            if (!capacity)
                return;

            std::allocator<Type> alloc = {};

            data = alloc.allocate(capacity);

            this->capacity = capacity;
        }

        // Increases Arrays capacity.
        constexpr void Expand(size count = 0) requires(hasConstructor<Type, Type&&>) {
            if (!count)
                return Expand(capacity ? capacity : 1);

            if (!data)
                return Allocate(count);

            std::allocator<Type> alloc = {};

            auto newData = alloc.allocate(capacity + count);

            for (size i = 0; i < length; i++) {
                std::construct_at(newData + i, Move(data[i]));

                data[i].~Type();
            }

            alloc.deallocate(data, capacity);

            capacity += count;

            data = newData;
        }

        // Frees memory.
        constexpr void Free() {
            if (!data)
                return;

            Clear();

            std::allocator<Type> alloc = {};

            alloc.deallocate(data, capacity);

            data = nullptr;

            capacity = 0;
        }

        constexpr void ShrinkToFit() requires(hasConstructor<Type, Type&&>) {
            if (length == capacity)
                return;

            std::allocator<Type> alloc = {};

            auto newData = alloc.allocate(length);

            for (size i = 0; i < length; i++)
                std::construct_at(newData + i, Move(data[i]));

            alloc.deallocate(data, capacity);

            data = newData;

            capacity = length;
        }

        template <typename... Args> requires(hasConstructor<Type, Args...> && hasConstructor<Type, Type&&>)
        constexpr Type& EmplaceBack(Args&&... args) {
            if (length == capacity)
                Expand();

            std::construct_at(data + length, Forward<Args>(args)...);

            length += 1;

            return data[length - 1];
        }

        template <typename... Args> requires(hasConstructor<Type, Args...> && hasConstructor<Type, Type&&>)
        constexpr void EmplaceBackMany(u32 count, Args&&... args) {
            if (!count)
                return;

            size newLength = length + count;

            if (capacity * 2 < newLength)
                Expand(newLength - length);
            else if (capacity < newLength)
                Expand();

            while (count) {
                count -= 1;

                std::construct_at(data + length + count, Forward<Args>(args)...);
            }

            length = newLength;
        }

        // Destructs data at the back.
        // Does nothing if count > length.
        constexpr void EraseBack(size count = 1) {
            if (count > length)
                count = length;

            while (count) {
                length -= 1;

                data[length].~Type();

                count -= 1;
            }
        }

        // If index == length, emplaces data at the back.
        // If index < length, emplaces data saving Array's order.
        // If index > length, emplaces data at the index and initializes data with same arguments in between.
        template <typename... Args> requires(hasConstructor<Type, Args...> && hasConstructor<Type, Type&&>)
        constexpr Type& EmplaceStrict(size index, Args&&... args) {
            if (index == length)
                return EmplaceBack(Forward<Args>(args)...);

            if (index < length) {
                ShiftRight(index, 1);

                std::construct_at(data + index, Forward<Args>(args)...);

                return data[index];
            }

            EmplaceBackMany(index + 1 - length, Forward<Args>(args)...);

            return data[index];
        }

        // If index == length, emplaces data at the back.
        // If index < length, emplaces data saving Array's order.
        // If index > length, emplaces data and initializes all data with same arguments in between.
        template <typename... Args> requires(hasConstructor<Type, Args...> && hasConstructor<Type, Type&&>)
        constexpr void EmplaceStrictMany(size index, u32 count, Args&&... args) {
            if (!count)
                return;

            if (index == length)
                return EmplaceBackMany(count, Forward<Args>(args)...);

            if (index < length) {
                ShiftRight(index, count);

                for (size i = index; i < index + count; i++)
                    std::construct_at(data + i, Forward<Args>(args)...);

                return;
            }
            
            EmplaceBackMany(index + count - length, Forward<Args>(args)...);
        }

        // Erases data by index saving Arrays order.
        // Does nothing if count == 0 or index is out of bounds.
        constexpr void EraseStrict(size index, size count = 1) requires(hasConstructor<Type, Type&&>) {
            if (!count || index >= length)
                return;

            if (index + count >= length)
                return EraseBack(count);

            ShiftLeft(index + count, count);
        }

        // Emplaces all data from range at index saving Arrays order.
        // Emplaces all data fat the back if index is out of bounds.
        constexpr void Insert(size index, const Range<const Type>& range) requires(hasConstructor<Type, Type&&> && hasConstructor<Type, const Type&>) {
            if (!range)
                return;

            size newLength = length + range.getLength();

            if (capacity * 2 < newLength)
                Expand(newLength - capacity);
            else if (capacity < newLength)
                Expand();

            if (index >= length) {
                for (size i = 0; i < range.getLength(); i++)
                    std::construct_at(data + length + i, range.begin()[i]);

                length = newLength;

                return;
            }

            if constexpr (hasAssignment<Type, const Type&>) {
                ShiftRight(index, range.getLength(), false);

                for (size i = 0; i < range.getLength(); i++)
                    data[i + index] = range.begin()[i];
            }
            else {
                ShiftRight(index, range.getLength(), true);

                for (size i = 0; i < range.getLength(); i++)
                    std::construct_at(data + index + i, range.begin()[i]);
            }
        }

        // Emplaces all data from range at index saving Arrays order.
        // Emplaces all data fat the back if index is out of bounds.
        constexpr void Insert(size index, const Range<Type>&& range) requires(hasConstructor<Type, Type&&>) {
            if (!range)
                return;

            size newLength = length + range.getLength();

            if (capacity * 2 < newLength)
                Expand(newLength - capacity);
            else if (capacity < newLength)
                Expand();

            if (index >= length) {
                for (size i = 0; i < range.getLength(); i++)
                    std::construct_at(data + length + i, Move(range.begin()[i]));

                length = newLength;

                return;
            }

            if constexpr (hasAssignment<Type, Type&&>) {
                ShiftRight(index, range.getLength(), false);

                for (size i = 0; i < range.getLength(); i++)
                    data[i + index] = Move(range.begin()[i]);
            }
            else {
                ShiftRight(index, range.getLength(), true);

                for (size i = 0; i < range.getLength(); i++)
                    std::construct_at(data + index + i, Move(range.begin()[i]));
            }
        }

        // Destructs all data.
        constexpr void Clear() {
            if constexpr (hasTrivialDestructor<Type>) {
                length = 0;
            }
            else {
                while (length) {
                    length -= 1;

                    data[length].~Type();
                }
            }
        }

        [[nodiscard]] constexpr Range<Type> range() noexcept {
            return Range<Type>(data, length);
        }

        [[nodiscard]] constexpr Range<Type> range(size start, size length) noexcept {
            return range().range(start, length);
        }

        [[nodiscard]] constexpr Range<const Type> range() const noexcept {
            return Range<const Type>(data, length);
        }

        [[nodiscard]] constexpr Range<const Type> range(size start, size length) const noexcept {
            return range().range(start, length);
        }

        // Creates new range from 0 to index (not inclusive).
        [[nodiscard]] constexpr Range<Type> rangeTo(size index) noexcept {
            return range().rangeTo(index);
        }

        // Creates new range from 0 to index (not inclusive).
        [[nodiscard]] constexpr Range<const Type> rangeTo(size index) const noexcept {
            return range().rangeTo(index);
        }

        // Creates new range from index to the end.
        [[nodiscard]] constexpr Range<Type> rangeFrom(size index) noexcept {
            return range().rangeFrom(index);
        }

        // Creates new range from index to the end.
        [[nodiscard]] constexpr Range<const Type> rangeFrom(size index) const noexcept {
            return range().rangeFrom(index);
        }

        // Returns empty optional if index is out of bounds.
        [[nodiscard]] constexpr Optional<Type&> operator [](size index) noexcept {
            if (index < length)
                return data[index];

            return {};
        }

        // Returns empty optional if index is out of bounds.
        [[nodiscard]] constexpr Optional<const Type&> operator [](size index) const noexcept {
            if (index < length)
                return data[index];

            return {};
        }

        [[nodiscard]] constexpr Type* begin() noexcept {
            return data;
        }

        [[nodiscard]] constexpr Type* end() noexcept {
            return data + length;
        }

        [[nodiscard]] constexpr const Type* begin() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr const Type* end() const noexcept {
            return data + length;
        }

        [[nodiscard]] constexpr size getLength() const noexcept {
            return length;
        }

        [[nodiscard]] constexpr size getCapacity() const noexcept {
            return capacity;
        }

        // Returns true if Array is empty.
        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return !length;
        }

        // Returns false if Array is empty.
        [[nodiscard]] constexpr operator bool() const noexcept {
            return length;
        }

        // Compares every data with value.
        [[nodiscard]] constexpr SearchResult SearchSequential(const Type& value) const {
            return range().SearchSequential(value);
        }

        // Compares data with value using binary search.
        // Array must be sorted.
        [[nodiscard]] constexpr SearchResult SearchSorted(const Type& value) const {
            return range().SearchSorted(value);
        }

        // Emplaces new data by binary searching its place.
        // Array must be sorted.
        template <typename... Args> requires(hasConstructor<Type, Args...> && hasConstructor<Type, Type&&>)
        constexpr Type& EmplaceSorted(Args&&... args) {
            Type value(Forward<Args>(args)...);

            auto sr = SearchSorted(value);

            return EmplaceStrict(sr.index, Move(value));
        }

        // Erases data by binary searching it.
        // Array must be sorted.
        constexpr void EraseSorted(const Type& value) {
            auto sr = SearchSorted(value);

            if (sr.found)
                EraseStrict(sr.index);
        }

        // Returns false if Arrays have different lengths.
        // Returns true if all data is equal.
        [[nodiscard]] constexpr bool operator ==(const Array& other) const {
            if (other.length != length)
                return false;

            for (size i = 0; i < length; i++)
                if (data[i] != other.data[i])
                    return false;

            return true;
        }

        // Returns true if Arrays have different lengths.
        // Returns true if any data is not equal.
        [[nodiscard]] constexpr bool operator !=(const Array& other) const {
            return !(*this == other);
        }

        // Returns empty optional if Array is empty.
        [[nodiscard]] constexpr Optional<Type&> front() noexcept {
            if (isVoid())
                return {};

            return data[0];
        }

        // Returns empty optional if Array is empty.
        [[nodiscard]] constexpr Optional<const Type&> front() const noexcept {
            if (isVoid())
                return {};

            return data[0];
        }

        // Returns empty optional if Array is empty.
        [[nodiscard]] constexpr Optional<Type&> back() noexcept {
            if (isVoid())
                return {};

            return data[length - 1];
        }

        // Returns empty optional if Array is empty.
        [[nodiscard]] constexpr Optional<const Type&> back() const noexcept {
            if (isVoid())
                return {};

            return data[length - 1];
        }
    };

    export template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Array<Type>& array) {
        return ostream << array.range();
    }

    // Array specialization of Optional.
    template <typename Type>
    class Optional<Array<Type>> {
        Array<Type> array = {};

    public:
        constexpr Optional() noexcept {
            array.length = sizemax;
        }

        template <typename... Args> requires(hasConstructor<Array<Type>, Args...>)
        constexpr Optional(Args&&... args) : array(Forward<Args>(args)...) {};

        constexpr Optional(const Optional& other) {
            array.length = sizemax;

            other.Use([&](const Array<Type>& value) {
                array.length = 0;

                array = value;
            });
        }

        constexpr Optional(Optional&& other) {
            array.length = sizemax;

            other.Use([&](Array<Type>& value) {
                array.length = 0;

                array = Move(value);
            });
        }

        constexpr ~Optional() {
            if (isVoid())
                array.length = 0;
        }

        constexpr Optional& operator =(const Optional& other) {
            other.Use([&](const Array<Type>& value) {
                if (isVoid())
                    array.length = 0;

                array = value;
            }).Else([&]() -> Optional {
                Destruct();

                return {};
            });

            return *this;
        }

        constexpr Optional& operator =(Optional&& other) {
            other.Use([&](Array<Type>& value) {
                if (isVoid())
                    array.length = 0;

                array = Move(value);
            }).Else([&]() -> Optional {
                Destruct();

                return {};
            });

            return *this;
        }

        template <typename... Args> requires(hasConstructor<Array<Type>, Args...>)
        constexpr Array<Type>& Construct(Args&&... args) {
            Destruct();

            array.length = 0;

            return array = Array<Type>(Forward<Args>(args)...);
        }

        constexpr void Destruct() {
            if (*this) {
                array.Free();

                array.length = sizemax;
            }
        }

        // Calls func if Optional is not void.
        template <typename Func> requires(isInvocable<Func, Array<Type>&>)
        constexpr Optional& Use(Func&& func) {
            if (*this)
                func(array);

            return *this;
        }

        // Calls func if Optional is not void.
        template <typename Func> requires(isInvocable<Func, const Array<Type>&>)
        constexpr const Optional& Use(Func&& func) const {
            if (*this)
                func(array);

            return *this;
        }

        // Calls func if Optional is not void and returns result.
        // Copies Optional if it is void.
        template <typename Func> requires(isInvocableNodiscard<Optional, Func, Array<Type>&>)
        constexpr Optional Then(Func&& func) {
            if (*this)
                return func(array);
            else
                return *this;
        }

        // Calls func if Optional is not void and returns result.
        // Copies Optional if it is void.
        template <typename Func> requires(isInvocableNodiscard<Optional, Func, const Array<Type>&>)
        constexpr Optional Then(Func&& func) const {
            if (*this)
                return func(array);
            else
                return *this;
        }

        // Calls func if Optional is void and returns result.
        // Copies Optional if it is not void.
        template <typename Func> requires(isInvocableNodiscard<Optional, Func>)
        constexpr Optional Else(Func&& func) const {
            if (*this)
                return *this;
            else 
                return func();
        }

        // Returns true if Optional doesn't contain a value.
        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return array.length == sizemax;
        }

        // Returns true if Optional contains a value.
        [[nodiscard]] constexpr operator bool() const noexcept {
            return !isVoid();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr Array<Type>& get() {
            if (isVoid())
                throw AccessError();

            return array;
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr const Array<Type>& get() const {
            if (isVoid())
                throw AccessError();

            return array;
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr operator Array<Type>& () {
            return get();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr operator const Array<Type>& () const {
            return get();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr Array<Type>& operator *() {
            return get();
        }

        // Returns reference to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr const Array<Type>& operator *() const {
            return get();
        }

        // Returns pointer to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr Array<Type>* operator ->() {
            return &get();
        }

        // Returns pointer to the value.
        // Throws AccessError if Optional is void.
        [[nodiscard]] constexpr const Array<Type>* operator ->() const {
            return &get();
        }
    };

    class ArrayTestType {
        int* memory;

    public:
        ArrayTestType() = delete;

        constexpr ArrayTestType(int value) : memory(new int(value)) {};

        constexpr ArrayTestType(const ArrayTestType& other) : memory(new int(*other.memory)) {};

        constexpr ArrayTestType(ArrayTestType&& other) : memory(new int(*other.memory)) {};

        constexpr ~ArrayTestType() {
            delete memory;
        }

        constexpr ArrayTestType& operator =(const ArrayTestType& other) = delete;

        constexpr ArrayTestType& operator =(ArrayTestType&& other) = delete;

        constexpr bool operator ==(int value) const {
            return *memory == value;
        }

        constexpr bool operator !=(int value) const {
            return *memory != value;
        }

        constexpr bool operator ==(const ArrayTestType& other) const {
            return *memory == *other.memory;
        }

        constexpr bool operator !=(const ArrayTestType& other) const {
            return *memory != *other.memory;
        }
    };

    consteval bool ArrayAssignTest() {
        Array<ArrayTestType> array = {
            0, 1, 2, 3, 4
        };

        for (u32 i = 0; i < 5; i++)
            if (array.begin()[i] != i)
                return false;

        if (array.getCapacity() != 5 || array.getLength() != 5)
            return false;

        Array<ArrayTestType> array2 = {
            0, 1, 2
        };

        array = array2;

        if (array.getCapacity() != 5 || array.getLength() != 3)
            return false;

        array2 = {
            0, 1, 2, 3, 4, 5, 6
        };

        array = array2;

        if (array.getCapacity() != 10 || array.getLength() != 7)
            return false;

        array.Free();

        array = {
            0, 1
        };

        array2 = {
            0, 1, 2, 3, 4, 5
        };

        array = array2;

        if (array.getCapacity() != 6 || array.getLength() != 6)
            return false;

        for (u32 i = 0; i < 5; i++)
            if (array.begin()[i] != i)
                return false;

        ArrayTestType carr[] = {
            10, 20, 30, 40
        };

        array = carr;

        if (array.getLength() != 4)
            return false;

        for (int i = 0; i < 4; i++)
            if (*array[i] != 10 * i + 10)
                return false;

        return true;
    }

    static_assert(ArrayAssignTest());

    consteval bool ArrayGettersTest() {
        Array<int> array = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9
        };

        if (*array[0] != 0 || *array[6] != 6 || array[10])
            return false;

        auto range = array.range();

        if (range.begin() != array.begin() || range.getLength() != array.getLength())
            return false;

        range = array.range(0, 4);

        if (range.begin() != array.begin() || range.getLength() != 4)
            return false;

        range = array.range(7, 4);

        if (range.begin() != array.begin() + 7 || range.getLength() != 3)
            return false;

        range = array.range(12, 1);

        if (range)
            return false;

        return true;
    }

    static_assert(ArrayGettersTest());

    consteval bool ArrayEmplaceBackTest() {
        Array<ArrayTestType> array = {};

        array.EmplaceBack(14);

        if (*array[0] != 14 || array.getLength() != 1)
            return false;

        array.EmplaceBack(20);

        if (*array[0] != 14 || *array[1] != 20 || array.getLength() != 2)
            return false;

        array.EmplaceBack(0);

        if (array.getCapacity() != 4)
            return false;

        array.EmplaceBackMany(2, -1);

        if (array.getCapacity() != 8)
            return false;

        array.EmplaceBackMany(2, -1);

        if (array.getCapacity() != 8)
            return false;

        array.EraseBack(4);

        if (array.getLength() != 3)
            return false;

        return true;
    }

    static_assert(ArrayEmplaceBackTest());

    consteval bool ArrayEmplaceStrictTest() {
        Array<ArrayTestType> array = {
            0, 0, 0, 0
        };

        array.EmplaceStrict(0, -1);

        Array<ArrayTestType> array2 = {
            -1, 0, 0, 0, 0
        };

        if (array != array2 || array.getCapacity() != 8)
            return false;

        array.EmplaceStrict(0, -1);

        array2 = {
            -1, -1, 0, 0, 0, 0
        };

        if (array != array2 || array.getCapacity() != 8)
            return false;

        array.EmplaceStrict(2, -2);

        array2 = {
            -1, -1, -2, 0, 0, 0, 0
        };

        if (array != array2 || array.getCapacity() != 8)
            return false;

        array.EmplaceStrict(7, 1);

        array2 = {
            -1, -1, -2, 0, 0, 0, 0, 1
        };

        if (array != array2 || array.getCapacity() != 8)
            return false;

        array.EmplaceStrict(10, 2);

        array2 = {
            -1, -1, -2, 0, 0, 0, 0, 1, 2, 2, 2
        };

        if (array != array2 || array.getCapacity() != 16)
            return false;

        array.EraseStrict(0, 2);

        array2 = {
            -2, 0, 0, 0, 0, 1, 2, 2, 2
        };

        if (array != array2)
            return false;

        array.EraseStrict(1, 5);

        array2 = {
            -2, 2, 2, 2
        };

        if (array != array2)
            return false;

        array.EraseStrict(3);

        array2 = {
            -2, 2, 2
        };

        if (array != array2)
            return false;

        array = {
            0, 1, 2, 3, 4, 5, 6
        };

        array.EraseStrict(2, 3);

        array2 = {
            0, 1, 5, 6
        };

        if (array != array2)
            return false;

        return true;
    }

    static_assert(ArrayEmplaceStrictTest());

    consteval bool ArrayInsertTest() {
        Array<ArrayTestType> array = {
            0, 1, 2, 3, 4, 5
        };

        Array<ArrayTestType> array2 = {
            -1, -2, -3, -4
        };

        array.Insert(1, array2.range());

        array2 = {
            0, -1, -2, -3, -4, 1, 2, 3, 4, 5
        };

        if (array != array2 || array.getCapacity() != 12)
            return false;

        array2 = {
            10, 20, 30
        };

        array.Insert(0, array2.range());

        array2 = {
            10, 20, 30, 0, -1, -2, -3, -4, 1, 2, 3, 4, 5
        };

        if (array != array2 || array.getCapacity() != 24)
            return false;

        return true;
    }

    static_assert(ArrayInsertTest());
}
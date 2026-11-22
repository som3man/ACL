// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

#include <ostream>

export module ACL.Main:Range;

import :Types;
import :Optional;
import :Traits;

namespace ACL {
    export struct SearchResult {
        size index;

        bool found;

        constexpr operator bool() const noexcept {
            return found;
        }
    };

    export template <typename Type>
    class Range {
        Type* data = nullptr;

        size length = 0;

    public:
        friend Optional<Range<Type>>;

        constexpr Range() noexcept {};

        constexpr Range(const Range&) noexcept = default;

        constexpr Range(Range&&) noexcept = default;

        constexpr Range& operator =(const Range&) noexcept = default;

        constexpr Range& operator =(Range&&) noexcept = default;

        constexpr ~Range() noexcept = default;

        [[nodiscard]] constexpr operator Range<const Type>() const noexcept requires(!isConst<Type>) {
            return Range<const Type>(data, 0, length);
        }

        constexpr Range(const Range& range, size start, size length) noexcept {
            if (length == sizemax)
                length = 0;

            if (!range || !length || start >= range.length)
                return;

            if (start + length > range.length)
                length = range.length - start;

            data = range.data + start;

            this->length = length;
        }

        template <size capacity> requires(capacity != 0 && capacity != sizemax)
        constexpr Range(Type (&array)[capacity]) noexcept {
            data = array;

            length = capacity;
        }

        constexpr Range(Type* array, size length) noexcept {
            if (length == sizemax)
                length = 0;

            if (!array || !length)
                return;

            data = array;

            this->length = length;
        }

        constexpr Range& Clear() noexcept {
            length = 0;

            data = nullptr;

            return *this;
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return !data;
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr Optional<Type&> operator [](size index) const noexcept {
            if (index < length)
                return data[index];

            return {};
        }

        [[nodiscard]] constexpr Type* begin() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr Type* end() const noexcept {
            return data + length;
        }

        [[nodiscard]] constexpr size getLength() const noexcept {
            return length;
        }

        [[nodiscard]] constexpr Range range(size start, size length) const noexcept {
            return Range(*this, start, length);
        }

        [[nodiscard]] constexpr Range rangeTo(size index) const noexcept {
            return range(0, index);
        }

        [[nodiscard]] constexpr Range rangeFrom(size index) const noexcept {
            return range(index, length - index);
        }

        [[nodiscard]] constexpr SearchResult SearchSequential(const Type& value) const {
            for (unsigned i = 0; i < length; i++)
                if (data[i] == value)
                    return { i, true };

            return { length, false };
        }

        [[nodiscard]] constexpr SearchResult SearchSorted(const Type& value) const {
            if (isVoid())
                return { 0, false };

            size middle = length / 2;

            if (value == data[middle])
                return { middle, true };
            else if (value < data[middle])
                return range(0, middle).SearchSorted(value);

            auto result = range(middle + 1, length).SearchSorted(value);

            result.index += middle + 1;

            return result;
        }

        [[nodiscard]] constexpr bool operator ==(const Range& other) const noexcept {
            return data == other.data && length == other.length;
        }

        [[nodiscard]] constexpr bool operator !=(const Range& other) const noexcept {
            return data != other.data || length != other.length;
        }

        template <typename Func> requires(isInvocableNodiscard<u32, Func, Type&>)
        constexpr Range& PopFront(Func&& func) {
            if (isVoid())
                return *this;

            while (true) {
                u32 res = func(*front());

                if (!res)
                    return *this;

                if (res >= length)
                    break;

                length -= res;

                data += res;
            }

            return Clear();
        }

        template <typename Func> requires(isInvocableNodiscard<u32, Func, Type&>)
        constexpr Range& PopBack(Func&& func) {
            if (isVoid())
                return *this;

            while (true) {
                u32 res = func(*back());

                if (!res)
                    return *this;

                if (res >= length)
                    break;

                length -= res;
            }

            return Clear();
        }

        [[nodiscard]] constexpr Optional<Type&> front() const noexcept {
            if (isVoid())
                return {};

            return data[0];
        }

        [[nodiscard]] constexpr Optional<Type&> back() const noexcept {
            if (isVoid())
                return {};

            return data[length - 1];
        }
    };

    export template <typename Char, typename Type>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Range<Type>& range) {
        ostream << '[' << range.getLength() << "]{ ";

        if (range) {
            ostream << range.begin()[0];

            for (unsigned i = 1; i < range.getLength(); i++)
                ostream << ", " << range.begin()[i];
        }

        return ostream << '}';
    }

    template <typename Type>
    class Optional<Range<Type>> {
        Range<Type> range = {};

    public:
        constexpr Optional() noexcept {
            range.length = sizemax;
        }

        template <typename... Args> requires(hasConstructor<Range<Type>, Args...>)
        constexpr Optional(Args&&... args) : range(Forward<Args>(args)...) {};

        constexpr Optional(const Optional&) noexcept = default;

        constexpr Optional(Optional&&) noexcept = default;

        constexpr ~Optional() noexcept = default;

        constexpr Optional& operator =(const Optional&) noexcept = default;

        constexpr Optional& operator =(Optional&&) noexcept = default;

        template <typename... Args> requires(hasConstructor<Range<Type>, Args...>)
        constexpr Range<Type>& Construct(Args&&... args) {
            return range = Range<Type>(Forward<Args>(args)...);
        }

        constexpr void Destruct() noexcept {
            range.data = nullptr;

            range.length = sizemax;
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return range.length == sizemax;
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return !isVoid();
        }

        template <typename Func> requires(isInvocable<Func, Range<Type>&>)
        constexpr Optional& Use(Func&& func) {
            if (*this)
                func(range);

            return *this;
        }

        template <typename Func> requires(isInvocable<Func, const Range<Type>&>)
        constexpr const Optional& Use(Func&& func) const {
            if (*this)
                func(range);

            return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Optional, Func, Range<Type>&>)
        constexpr Optional Then(Func&& func) {
            if (*this)
                return func(range);
            else
                return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Optional, Func, const Range<Type>&>)
        constexpr Optional Then(Func&& func) const {
            if (*this)
                return func(range);
            else
                return *this;
        }

        template <typename Func> requires(isInvocableNodiscard<Optional, Func>)
        constexpr Optional Else(Func&& func) const {
            if (*this)
                return *this;
            else
                return func();
        }

        [[nodiscard]] constexpr Range<Type>& get() {
            if (isVoid())
                throw AccessError();

            return range;
        }

        [[nodiscard]] constexpr const Range<Type>& get() const {
            if (isVoid())
                throw AccessError();

            return range;
        }

        [[nodiscard]] constexpr operator Range<Type>& () {
            return get();
        }

        [[nodiscard]] constexpr operator const Range<Type>& () const {
            return get();
        }

        [[nodiscard]] constexpr Range<Type>& operator *() {
            return get();
        }

        [[nodiscard]] constexpr const Range<Type>& operator *() const {
            return get();
        }

        [[nodiscard]] constexpr Range<Type>* operator ->() {
            return &get();
        }

        [[nodiscard]] constexpr const Range<Type>* operator ->() const {
            return &get();
        }
    };

    consteval bool RangeTest() {
        int array[9] = {
            0, 1, 2, 3, 5, 6, 7, 8, 9
        };

        Range<int> range = array;

        if (range.begin() != array || range.getLength() != 9)
            return false;

        range = {
            array,
            0,
            1
        };

        if (range.getLength() != 1)
            return false;

        range = array;

        auto sr = range.SearchSorted(4);

        if (sr.index != 4 || sr.found)
            return false;

        sr = range.SearchSorted(7);

        if (sr.index != 6 || !sr.found)
            return false;

        Range<int> range2 = {
            range,
            0,
            10
        };

        if (range2.getLength() != 9)
            return false;

        range2 = {
            range,
            0,
            230
        };

        if (range2.getLength() != 9)
            return false;

        range2 = {
            range,
            9,
            9
        };

        if (range2 || range2.getLength() != 0)
            return false;

        range2 = {
            range,
            9,
            10
        };

        if (range2 || range2.getLength() != 0)
            return false;

        if (!range[2] || range[10])
            return false;

        return true;
    }

    static_assert(RangeTest());
}
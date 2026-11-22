// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

#include <ostream>

export module ACL.Main:Many;

import :Types;
import :Traits;
import :Range;
import :Optional;

namespace ACL {
    export template <typename Type, size capacity> requires(capacity > 2 && capacity < sizemax)
    class Many {
        Type data[capacity];

    public:
        constexpr Many() : data() {};

        template <typename... Values> requires(sizeof...(Values) == capacity)
        constexpr Many(Values&&... values) : data(Forward<Values>(values)...) {};

        constexpr ~Many() {};

        constexpr Many& operator =(const Many& other) {
            for (size i = 0; i < capacity; i++)
                data[i] = other.data[i];

            return *this;
        }

        constexpr Many& operator =(Many&& other) {
            for (size i = 0; i < capacity; i++)
                data[i] = Move(other.data[i]);

            return *this;
        }

        [[nodiscard]] constexpr Optional<Type&> operator [](size index) noexcept {
            if (index < capacity)
                return data[index];

            return {};
        }

        [[nodiscard]] constexpr Optional<const Type&> operator [](size index) const noexcept {
            if (index < capacity)
                return data[index];

            return {};
        }

        [[nodiscard]] constexpr Type& front() noexcept {
            return data[0];
        }

        [[nodiscard]] constexpr const Type& front() const noexcept {
            return data[0];
        }

        [[nodiscard]] constexpr Type& back() noexcept {
            return data[capacity - 1];
        }

        [[nodiscard]] constexpr const Type& back() const noexcept {
            return data[capacity - 1];
        }

        [[nodiscard]] constexpr Type* begin() noexcept {
            return data;
        }

        [[nodiscard]] constexpr const Type* begin() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr Type* end() noexcept {
            return data + capacity;
        }

        [[nodiscard]] constexpr const Type* end() const noexcept {
            return data + capacity;
        }

        [[nodiscard]] constexpr Range<Type> range() noexcept {
            return Range<Type>(
                data,
                capacity
            );
        }

        [[nodiscard]] constexpr Range<const Type> range() const noexcept {
            return Range<Type>(
                data,
                capacity
            );
        }

        [[nodiscard]] constexpr Range<Type> range(size start, size length) noexcept {
            return range().range(start, length);
        }

        [[nodiscard]] constexpr Range<const Type> range(size start, size length) const noexcept {
            return range().range(start, length);
        }

        [[nodiscard]] constexpr Range<Type> rangeTo(size index) noexcept {
            return range().rangeTo(index);
        }

        [[nodiscard]] constexpr Range<const Type> rangeTo(size index) const noexcept {
            return range().rangeTo(index);
        }

        [[nodiscard]] constexpr Range<Type> rangeFrom(size index) noexcept {
            return range().rangeFrom(index);
        }

        [[nodiscard]] constexpr Range<const Type> rangeFrom(size index) const noexcept {
            return range().rangeFrom(index);
        }

        [[nodiscard]] constexpr SearchResult SearchSequential(const Type& value) const {
            return range().SearchSequential(value);
        }

        [[nodiscard]] constexpr SearchResult SearchSorted(const Type& value) const {
            return range().SearchSorted(value);
        }
    };

    export template <typename Char, typename Type, size capacity>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const Many<Type, capacity>& many) {
        return ostream << many.range();
    }
}
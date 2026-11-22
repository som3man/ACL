// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

#include <algorithm>
#include <ostream>

export module ACL.String:Instance;

import ACL.Main;

namespace ACL {
    export class String {
        Array<char> array = {};

    public:
        class View;

        template <u32 capacity> requires(capacity > 0)
        constexpr String(const char (&cstring)[capacity]) noexcept {
            if (cstring[capacity - 1])
                array.EmplaceBackMany(capacity + 1, '\0');
            else
                array.EmplaceBackMany(capacity, '\0');

            std::copy(cstring, cstring + capacity, array.begin());
        }

        constexpr String(const char* cstring, u32 length) noexcept {
            if (!cstring || !length)
                return;

            array.EmplaceBackMany(length + 1, '\0');

            std::copy(cstring, cstring + length, array.begin());
        }

        constexpr String() noexcept {};

        constexpr String(const String& other) : array(other.array) {};

        constexpr String(String&& other) : array(other.array) {};

        constexpr String(const View& other);

        constexpr ~String() {};

        constexpr String& operator =(const String& other) {
            array = other.array;

            return *this;
        }

        constexpr String& operator =(String&& other) {
            array = Move(other.array);

            return *this;
        }

        constexpr char& EmplaceBack(char symbol) {
            array.EraseBack();

            array.EmplaceBack(symbol);

            array.EmplaceBack('\0');

            return array.end()[-2];
        }

        constexpr void EmplaceBackMany(u32 count, char symbol) {
            if (!count)
                return;

            if (array)
                array.end()[-1] = symbol;
            else {
                array.Expand(count + 1);

                array.EmplaceBack(symbol);
            }

            array.EmplaceBack('\0');
        }

        constexpr void EraseBack(u32 count = 1) {
            if (!count)
                return;

            array.EraseBack(count);

            if (array)
                array.end()[-1] = '\0';
            else
                array.EmplaceBack('\0');
        }

        constexpr char& EmplaceStrict(u32 index, char symbol) {
            array.EraseBack();

            array.EmplaceStrict(index, symbol);

            array.EmplaceBack('\0');

            return array.begin()[index];
        }

        constexpr void EmplaceStrictMany(u32 index, u32 count, char symbol) {
            if (!count)
                return;

            array.EraseBack();

            array.EmplaceStrictMany(index, count, symbol);

            array.EmplaceBack('\0');
        }

        constexpr void EraseStrict(u32 index, u32 count = 1) {
            if (!count)
                return;

            array.EraseBack();

            array.EraseStrict(index, count);

            array.EmplaceBack('\0');
        }

        constexpr void Insert(u32 index, const View& view);

        constexpr void Clear() {
            array.Clear();

            array.EmplaceBack('\0');
        }

        constexpr void ShrinkToFit() {
            array.ShrinkToFit();
        }

        [[nodiscard]] constexpr u32 getCapacity() const noexcept {
            return array.getCapacity();
        }

        [[nodiscard]] constexpr u32 getLength() const noexcept {
            return array ? array.getLength() - 1 : 0;
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return !getLength();
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return getLength();
        }

        [[nodiscard]] constexpr char* begin() noexcept {
            return array.begin();
        }

        [[nodiscard]] constexpr const char* begin() const noexcept {
            return array.begin();
        }

        [[nodiscard]] constexpr char* end() noexcept {
            return begin() + getLength();
        }

        [[nodiscard]] constexpr const char* end() const noexcept {
            return begin() + getLength();
        }

        [[nodiscard]] constexpr Optional<char&> operator [](u32 index) noexcept {
            if (index < getLength())
                return array.begin()[index];

            return {};
        }

        [[nodiscard]] constexpr Optional<const char&> operator [](u32 index) const noexcept {
            if (index < getLength())
                return array.begin()[index];

            return {};
        }

        [[nodiscard]] constexpr View range() const noexcept;

        [[nodiscard]] constexpr View range(u32 start, u32 length) const noexcept;

        [[nodiscard]] constexpr View rangeTo(u32 index) const noexcept;

        [[nodiscard]] constexpr View rangeFrom(u32 index) const noexcept;

        [[nodiscard]] constexpr bool operator ==(const View& other) const noexcept;

        [[nodiscard]] constexpr bool operator !=(const View& other) const noexcept;

        [[nodiscard]] constexpr bool operator >=(const View& other) const noexcept;

        [[nodiscard]] constexpr bool operator <=(const View& other) const noexcept;

        [[nodiscard]] constexpr bool operator >(const View& other) const noexcept;

        [[nodiscard]] constexpr bool operator <(const View& other) const noexcept;

        bool operator ==(bool) const = delete;

        bool operator !=(bool) const = delete;

        bool operator <=(bool) const = delete;

        bool operator >=(bool) const = delete;

        bool operator <(bool) const = delete;

        bool operator >(bool) const = delete;

        constexpr String& operator +=(const View& view) {
            Insert(u32max, view);

            return *this;
        }

        [[nodiscard]] constexpr String operator +(const View& view) const {
            String result = *this;

            result += view;

            return result;
        }
    };

    class String::View {
        Range<const char> data = {};

        constexpr View(const Range<const char>& range) noexcept : data(range) {};

    public:
        friend String;

        constexpr View() noexcept {};

        constexpr View(const View&) noexcept = default;

        constexpr View(View&&) noexcept = default;

        constexpr ~View() noexcept = default;

        constexpr View& operator =(const View& other) noexcept = default;

        constexpr View& operator =(View&& other) noexcept = default;

        template <u32 capacity> requires(capacity > 0)
        constexpr View(const char (&array)[capacity]) noexcept {
            bool nullTerm = true;

            if (array[capacity - 1])
                nullTerm = false;

            data = {
                array,
                nullTerm ? capacity - 1 : capacity
            };
        }

        constexpr View(const char* cstring, u32 length) noexcept {
            data = {
                cstring,
                length
            };
        }

        constexpr View(const String& string) noexcept : data(string.begin(), string.getLength()) {};

        [[nodiscard]] constexpr const char* begin() const noexcept {
            return data.begin();
        }

        [[nodiscard]] constexpr const char* end() const noexcept {
            return data.end();
        }

        [[nodiscard]] constexpr u32 getLength() const noexcept {
            return data.getLength();
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return data.isVoid();
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr Optional<char> operator [](u32 index) const noexcept {
            if (index < getLength())
                return begin()[index];

            return {};
        }

        [[nodiscard]] constexpr View range(u32 start, u32 length) const noexcept {
            return data.range(start, length);
        }

        [[nodiscard]] constexpr View rangeTo(u32 index) const noexcept {
            return data.rangeTo(index);
        }

        [[nodiscard]] constexpr View rangeFrom(u32 index) const noexcept {
            return data.rangeFrom(index);
        }

        [[nodiscard]] constexpr bool operator ==(const View& other) const noexcept {
            if (getLength() != other.getLength())
                return false;

            for (u32 i = 0; i < getLength(); i++)
                if (begin()[i] != other.begin()[i])
                    return false;

            return true;
        }

        [[nodiscard]] constexpr bool operator !=(const View& other) const noexcept {
            return !(*this == other);
        }

        [[nodiscard]] constexpr bool operator >(const View& other) const noexcept {
            if (getLength() > other.getLength())
                return true;

            if (getLength() < other.getLength())
                return false;

            for (u32 i = 0; i < getLength(); i++)
                if (begin()[i] != other.begin()[i])
                    return begin()[i] > other.begin()[i];

            return false;
        }

        [[nodiscard]] constexpr bool operator <=(const View& other) const noexcept {
            return !(*this > other);
        }

        [[nodiscard]] constexpr bool operator <(const View& other) const noexcept {
            if (getLength() < other.getLength())
                return true;

            if (getLength() > other.getLength())
                return false;

            for (u32 i = 0; i < getLength(); i++)
                if (begin()[i] != other.begin()[i])
                    return begin()[i] < other.begin()[i];

            return false;
        }

        [[nodiscard]] constexpr bool operator >=(const View& other) const noexcept {
            return !(*this < other);
        }

        constexpr View& PopBack(auto&& func) {
            data.PopBack(Forward(func));

            return *this;
        }

        constexpr View& PopFront(auto&& func) {
            data.PopFront(Forward(func));

            return *this;
        }

        [[nodiscard]] constexpr Optional<const char&> front() const noexcept {
            return data.front();
        }

        [[nodiscard]] constexpr Optional<const char&> back() const noexcept {
            return data.back();
        }

        [[nodiscard]] constexpr String operator +(const View& other) const noexcept {
            String result = *this;

            result += other;

            return result;
        }

        [[nodiscard]] constexpr Optional<size> NullTermPos() const noexcept {
            for (size i = 0; i < getLength(); i++)
                if (begin()[i] == '\0')
                    return i;

            return {};
        }
    };

    constexpr String::String(const View& other) : array(other.data) {
        if (array && array.end()[-1] != '\0')
            array.EmplaceBack('\0');
    }

    constexpr void String::Insert(u32 index, const View& view) {
        array.EraseBack();

        array.Insert(index, view.data);

        if (array && array.end()[-1] != '\0')
            array.EmplaceBack('\0');
    }

    constexpr String::View String::range() const noexcept {
        return String::View(*this);
    }

    constexpr String::View String::range(u32 start, u32 length) const noexcept {
        return range().range(start, length);
    }

    constexpr String::View String::rangeTo(u32 index) const noexcept {
        return range().rangeTo(index);
    }

    constexpr String::View String::rangeFrom(u32 index) const noexcept {
        return range().rangeFrom(index);
    }

    constexpr bool String::operator ==(const View& other) const noexcept {
        return range() == other;
    }

    constexpr bool String::operator !=(const View& other) const noexcept {
        return range() != other;
    }

    constexpr bool String::operator >=(const View& other) const noexcept {
        return range() >= other;
    }

    constexpr bool String::operator <=(const View& other) const noexcept {
        return range() <= other;
    }

    constexpr bool String::operator >(const View& other) const noexcept {
        return range() > other;
    }

    constexpr bool String::operator <(const View& other) const noexcept {
        return range() < other;
    }

    constexpr String& operator <<(String& string, char symbol) {
        string.EmplaceBack(symbol);

        return string;
    }

    export constexpr String& operator <<(String& string, const String::View& view) {
        return string += view;
    }

    export template <u32 capacity>
    constexpr String& operator <<(String& string, const char (&cstring)[capacity]) {
        return string << String::View(cstring);
    }

    export constexpr String& operator <<(String& string, const String& other) {
        return string << other.range();
    }

    export template <typename Char>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const String::View& view) {
        return ostream.write(view.begin(), view.getLength());
    }

    export template <typename Char>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const String& string) {
        return ostream << string.range();
    }
}
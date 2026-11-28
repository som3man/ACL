// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

#include <algorithm>
#include <ostream>

export module ACL.String:Instance;

import ACL.Main;

import :View;

namespace ACL {
    export class String {
        Array<char> array = {};

    public:
        template <size capacity> requires(capacity > 0)
        constexpr String(const char (&cstring)[capacity]) noexcept {
            if (cstring[capacity - 1])
                array.EmplaceBackMany(capacity + 1, '\0');
            else
                array.EmplaceBackMany(capacity, '\0');

            std::copy(cstring, cstring + capacity, array.begin());
        }

        constexpr String(const char* cstring, size length) noexcept {
            if (!cstring || !length) {
                array.EmplaceBack('\0');

                return;
            }

            array.EmplaceBackMany(length + 1, '\0');

            std::copy(cstring, cstring + length, array.begin());
        }

        constexpr String(StrView<const char> view) noexcept {
            array.EmplaceBackMany(view.getLength() + 1, '\0');

            std::copy(view.begin(), view.end(), array.begin());
        }

        [[nodiscard]] constexpr operator StrView<char>() noexcept {
            return StrView<char>(begin(), getLength());
        }

        [[nodiscard]] constexpr operator StrView<const char>() const noexcept {
            return StrView<const char>(begin(), getLength());
        }

        constexpr String() noexcept {};

        constexpr String(const String& other) : array(other.array) {};

        constexpr String(String&& other) : array(other.array) {};

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

        constexpr void EraseBack(size count = 1) {
            if (!count)
                return;

            array.EraseBack(count);

            if (array)
                array.end()[-1] = '\0';
            else
                array.EmplaceBack('\0');
        }

        constexpr char& EmplaceStrict(size index, char symbol) {
            array.EraseBack();

            array.EmplaceStrict(index, symbol);

            array.EmplaceBack('\0');

            return array.begin()[index];
        }

        constexpr void EmplaceStrictMany(size index, u32 count, char symbol) {
            if (!count)
                return;

            array.EraseBack();

            array.EmplaceStrictMany(index, count, symbol);

            array.EmplaceBack('\0');
        }

        constexpr void EraseStrict(size index, size count = 1) {
            if (!count)
                return;

            array.EraseBack();

            array.EraseStrict(index, count);

            array.EmplaceBack('\0');
        }

        constexpr void Insert(size index, StrView<const char> view) {
            if (index >= getLength()) {
                array.EraseBack();

                auto ol = array.getLength();

                array.EmplaceBackMany(view.getLength() + 1, '\0');

                std::copy(view.begin(), view.end(), array.begin() + ol);
            }
            else {
                array.EmplaceStrictMany(index, view.getLength(), '\0');

                std::copy(view.begin(), view.end(), array.begin() + index);
            }
        }

        constexpr void Clear() {
            array.Clear();

            array.EmplaceBack('\0');
        }

        constexpr void ShrinkToFit() {
            array.ShrinkToFit();
        }

        [[nodiscard]] constexpr size getCapacity() const noexcept {
            return array.getCapacity();
        }

        [[nodiscard]] constexpr size getLength() const noexcept {
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

        [[nodiscard]] constexpr Optional<char&> operator [](size index) noexcept {
            if (index < getLength())
                return array.begin()[index];

            return {};
        }

        [[nodiscard]] constexpr Optional<const char&> operator [](size index) const noexcept {
            if (index < getLength())
                return array.begin()[index];

            return {};
        }

        [[nodiscard]] constexpr StrView<char> view() noexcept {
            return StrView<char>(begin(), getLength());
        }

        [[nodiscard]] constexpr StrView<const char> view() const noexcept {
            return StrView<const char>(begin(), getLength());
        }

        [[nodiscard]] constexpr StrView<char> view(size index, size length) noexcept {
            return view().view(index, length);
        }

        constexpr String& operator +=(StrView<const char> view) {
            Insert(sizemax, view);

            return *this;
        }

        [[nodiscard]] constexpr bool operator ==(const StrView<>& view) const noexcept {
            return this->view() == view;
        }

        [[nodiscard]] constexpr bool operator !=(const StrView<>& view) const noexcept {
            return this->view() != view;
        }
    };

    export [[nodiscard]] constexpr String operator +(StrView<const char> view, StrView<const char> other) {
        String result = view;

        result += other;

        return result;
    }

    export constexpr String& operator <<(String& string, char symbol) {
        string.EmplaceBack(symbol);

        return string;
    }

    export constexpr String& operator <<(String& string, StrView<const char> view) {
        return string += view;
    }

    export template <size capacity>
    constexpr String& operator <<(String& string, const char (&cstring)[capacity]) {
        return string << StrView(cstring);
    }

    export constexpr String& operator <<(String& string, const String& other) {
        return string << other.view();
    }

    export template <typename Char>
    std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& ostream, const String& string) {
        return ostream << string.view();
    }
}
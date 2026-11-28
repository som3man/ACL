// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

#include <ostream>

export module ACL.String:View;

import ACL.Main;

namespace ACL {
    export template <typename Char = const char> 
    class StrView {
        Range<Char> data = {};

    public:
        constexpr StrView() noexcept {};

        constexpr StrView(const StrView&) noexcept = default;

        constexpr StrView(StrView&&) noexcept = default;

        constexpr ~StrView() noexcept = default;

        constexpr StrView& operator =(const StrView&) noexcept = default;

        constexpr StrView& operator =(StrView&&) noexcept = default;

        [[nodiscard]] constexpr operator StrView<const Char>() const noexcept requires(!isConst<Char>) {
            return StrView<const Char>(data.begin(), data.getLength());
        }

        template <size length>
        constexpr StrView(Char (&cstring)[length]) noexcept : data(cstring) {
            if (*data.back() == '\0')
                data = data.range(0, length - 1);
        }

        constexpr StrView(Char* cstring, size length) noexcept : data(cstring, length) {};

        constexpr StrView(const Range<Char>& range) noexcept : data(range) {
            if (data && *data.back() == '\0')
                data = data.range(0, data.getLength() - 1);
        }

        [[nodiscard]] constexpr Optional<Char&> operator [](size index) const noexcept {
            return data[index];
        }

        [[nodiscard]] constexpr Optional<Char&> front() const noexcept {
            return data.front();
        }

        [[nodiscard]] constexpr Optional<Char&> back() const noexcept {
            return data.back();
        }

        [[nodiscard]] constexpr size getLength() const noexcept {
            return data.getLength();
        }

        [[nodiscard]] constexpr bool isVoid() const noexcept {
            return data.isVoid();
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return data;
        }

        [[nodiscard]] constexpr Char* begin() const noexcept {
            return data.begin();
        }

        [[nodiscard]] constexpr Char* end() const noexcept {
            return data.end();
        }

        [[nodiscard]] constexpr StrView view(size index, size length) const noexcept {
            StrView result = {};

            result.data = data.range(index, length);

            return result;
        }

        [[nodiscard]] constexpr StrView viewTo(size index) const noexcept {
            StrView result = {};

            result.data = data.rangeTo(index);

            return result;
        }

        [[nodiscard]] constexpr StrView viewFrom(size index) const noexcept {
            StrView result = {};

            result.data = data.rangeFrom(index);

            return result;
        }

        [[nodiscard]] constexpr Optional<size> NullTermPos() const noexcept {
            for (size i = 0; i < getLength(); i++)
                if (begin()[i] == '\0')
                    return i;

            return {};
        }

        constexpr StrView& PopFront(auto&& func) {
            data.PopFront(Forward(func));

            return *this;
        }

        constexpr StrView& PopBack(auto&& func) {
            data.PopBack(Forward(func));

            return *this;
        }

        [[nodiscard]] constexpr StrView CopyFrontUntil(const StrView& symbols) const noexcept {
            size index = 0;

            while (index < getLength()) {
                auto symbol = begin()[index];

                for (auto s : symbols)
                    if (s == symbol)
                        return viewTo(index);

                index += 1;
            }

            return *this;
        }

        [[nodiscard]] constexpr StrView CopyBackUntil(const StrView& symbols) const noexcept {
            size index = getLength() - 1;

            while (index != sizemax) {
                auto symbol = begin()[index];

                for (auto s : symbols)
                    if (s == symbol)
                        return viewFrom(index + 1);

                index -= 1;
            }

            return *this;
        }

        constexpr StrView CutFront(size count) noexcept {
            StrView copy = *this;

            *this = viewFrom(count);

            return copy.viewTo(count);
        }

        constexpr StrView CutBack(size count) noexcept {
            StrView copy = *this;

            *this = viewTo(getLength() - count);

            return copy.viewFrom(getLength() - count);
        }

        constexpr StrView CutFrontUntil(const StrView& symbols) noexcept {
            StrView copy = *this;

            PopFront([&](char symbol) -> u32 {
                for (auto s : symbols)
                    if (s == symbol)
                        return 0;

                return 1;
            });

            return copy.viewTo(copy.getLength() - getLength());
        }

        constexpr StrView CutBackUntil(const StrView& symbols) noexcept {
            StrView copy = *this;

            PopBack([&](char symbol) -> u32 {
                for (auto s : symbols)
                    if (s == symbol)
                        return 0;

                return 1;
            });

            return copy.viewFrom(copy.getLength() - getLength() + 1);
        }

        [[nodiscard]] constexpr bool operator ==(StrView<const Char> other) const noexcept {
            if (getLength() != other.getLength())
                return false;

            for (size i = 0; i < getLength(); i++)
                if (begin()[i] != other.begin()[i])
                    return false;

            return true;
        }

        [[nodiscard]] constexpr bool operator !=(StrView<const Char> other) const noexcept {
            return !(*this == other);
        }
    };

    export std::ostream& operator <<(std::ostream& ostream, StrView<const char> view) {
        return ostream.write(view.begin(), view.getLength());
    }

    export std::wostream& operator <<(std::wostream& ostream, StrView<const char> view) {
        for (auto s : view)
            ostream << s;

        return ostream;
    }

    export std::wostream& operator <<(std::wostream& ostream, StrView<const wchar_t> view) {
        return ostream.write(view.begin(), view.getLength());
    }
}

export [[nodiscard]] consteval ACL::StrView<const char> operator ""_str(const char* cstring, ACL::size length) {
    return ACL::StrView<const char>(cstring, length);
}
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

export module ACL.String:Conversion;

import ACL.Main;
import :Instance;

namespace ACL {
    export template <typename Type>
    struct ToString {};

    template <typename Type> requires(isInteger<Type> && isUnsigned<Type>)
    struct ToString<Type> {
        Type value;

        u8 base;
    };

    export template <typename Type> requires(isInteger<Type> && isUnsigned<Type>)
    constexpr String& operator <<(String& string, ToString<Type> toString) {
        if (!toString.value)
            return string << '0';

        if (toString.base < 2)
            toString.base = 2;
        else if (toString.base > 16)
            toString.base = 16;

        constexpr char syms[] = "0123456789ABCDEF";

        char rstr[32] = {};

        char* i = rstr;

        while (toString.value && i < rstr + 32) {
            *i = syms[toString.value % toString.base];

            toString.value /= 10;

            i += 1;
        }

        while (i != rstr) {
            i -= 1;

            string.EmplaceBack(*i);
        }

        return string;
    }

    export template <typename Type> requires(isInteger<Type>)
    constexpr String& operator <<(String& string, Type number) {
        if constexpr (isUnsigned<Type>)
            return string << ToString<Type>(number, 10);
        else {
            if (number < 0) {
                number = -number;

                string << '-';
            }

            return string << static_cast<u64>(number);
        }
    }

    export template <typename Type> requires(isInteger<Type>)
    constexpr String::View operator >>(String::View string, Type& number) {
        if (!string)
            return string;

        number = 0;

        bool isNeg = false;

        if (*string.front() == '-') {
            if constexpr (isUnsigned<Type>)
                return string;

            isNeg = true;

            string = string.rangeFrom(1);
        }

        string.PopFront([&](char symbol) -> u32 {
            if (symbol < '0' || symbol > '9')
                return 0;

            number *= 10;

            number += symbol - '0';

            return 1;
        });

        if constexpr (!isUnsigned<Type>) {
            if (isNeg)
                number = -number;
        }

        return string;
    }

    export template <typename Type>
    constexpr String& operator <<(String& string, const Range<Type>& range) {
        string << '[' << range.getLength() << "]{ ";

        if (range) {
            string << range.begin()[0];

            for (size i = 1; i < range.getLength(); i++)
                string << ", " << range.begin()[i];
        }

        return string << " }";
    }

    export template <typename Type, size capacity>
    constexpr String& operator <<(String& string, const Many<Type, capacity>& many) {
        return string << many.range();
    }

    export template <typename Type>
    constexpr String& operator <<(String& string, const Array<Type>& array) {
        return string << array.range();
    }

    export template <typename Type>
    constexpr String& operator <<(String& string, const Optional<Type>& optional) {
        if (optional)
            return string << *optional;
        else
            return string << "void";
    }

    export template <typename... Types>
    constexpr String& operator <<(String& string, const Union<Types...>& data) {
        return data.template Visit<String&>([&](const Types& value) -> String& {
            return string << value;
        }...);
    }

    export template <typename Type, typename Error>
    constexpr String& operator <<(String& string, const Result<Type, Error>& result) {
        result.Use([&](const Type& value) {
            string << value;
        }).Else([&](const Error& error) -> Result<Type, Error> {
            string << error;

            return {};
        });

        return string;
    }

    export template <typename... Types>
    constexpr String& operator <<(String& string, const PolyPtr<Types...>& polyPtr) {
        polyPtr.Visit(
            [&]() {
                string << "void";
            },
            [&](const Types& value) {
                string << value;
            }...
        );

        return string;
    }

    export template <typename Type>
    constexpr String& operator <<(String& string, const Heap<Type>& heap) {
        return string << heap.get();
    }

    export template <typename Type>
    constexpr String& operator <<(String& string, const Shared<Type>& shared) {
        return string << shared.get();
    }
}
// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

module;

export module ACL.String:Conversion;

import ACL.Main;
import :Instance;

namespace ACL {
    export template <typename Type>
    constexpr String& operator <<(String& string, const Range<Type>& range) {
        string << '[' << range.getLength() << "]{ ";

        if (range) {
            string << range.begin()[0];

            for (u32 i = 1; i < range.getLength(); i++)
                string << ", " << range.begin()[i];
        }

        return string << " }";
    }

    export template <typename Type, u32 capacity>
    constexpr String& operator <<(String& string, const Type (&array)[capacity]) {
        return string << Range(array);
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

    export template <typename Type> requires(isInteger<Type>)
    constexpr String& operator <<(String& string, Type number) {
        if (!number)
            return string << '0';

        if (number < 0) {
            number = -number;

            string << '-';
        }

        char res[32] = {};

        char* ptr = res;

        while (number) {
            *ptr = (number % 10) + '0';

            number /= 10;

            ptr += 1;
        }

        while (ptr != res) {
            ptr -= 1;

            string.EmplaceBack(*ptr);
        }

        return string;
    }
}
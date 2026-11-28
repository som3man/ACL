// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

export module ACL.XML:Parsing;

import ACL.Main;
import :Element;

namespace ACL::XML {
    export enum ParsingError : u8 {
        NONE,
        INVALID_STRUCTURE,
        INVALID_PROLOG,
        INVALID_ELEMENT_TAG,
        INVALID_ATTRIBUTE_NAME,
        INVALID_ATTRIBUTE_VALUE,
        INVALID_COMMENT,
        FAIL
    };

    export struct ParsingResult {
        Element root;

        ParsingError error;

        [[nodiscard]] constexpr bool isSuccess() const noexcept {
            return error == ParsingError::NONE;
        }

        [[nodiscard]] constexpr operator bool() const noexcept {
            return isSuccess();
        }
    };

    export [[nodiscard]] constexpr ParsingResult Parse(StrView<> data) noexcept;

    constexpr ParsingError ReadProlog(StrView<>& data) {
        enum {
            READ_START,
            SEARCH_END,
            READ_END
        } state = READ_START;

        constexpr StrView<> xmlStr = "xml";

        ParsingError error = ParsingError::NONE;

        data.PopFront([&](char symbol) -> u32 {
            switch (state) {
            case READ_START:
                if (data.viewTo(xmlStr.getLength()) != xmlStr) {
                    error = ParsingError::INVALID_PROLOG;
                    return 0;
                }
                state = SEARCH_END;
                return xmlStr.getLength();
            case SEARCH_END:
                switch (symbol) {
                case '?':
                    state = READ_END;
                default:
                    break;
                }
                return 1;
            case READ_END:
                if (symbol != '>')
                    error = ParsingError::INVALID_PROLOG;
                return 0;
            }
        });

        return error;
    }

    consteval bool ReadPrologTest() {
        StrView<> str = "xml version=\"1.0\" encoding=\"UTF-8\"?>";

        StrView<> data = str;

        if (ReadProlog(data) != ParsingError::NONE)
            return false;

        if (data.front() != str.back())
            return false;

        str = "xm?>";

        data = str;

        if (ReadProlog(data) != ParsingError::INVALID_PROLOG)
            return false;

        return true;
    }

    static_assert(ReadPrologTest());

    constexpr ParsingError ReadAttributes(StrView<>& data, Array<Attribute>& attributes) {
        enum {
            SEARCH_NAME,
            READ_NAME,
            SEARCH_VALUE,
            READ_VALUE
        } state = SEARCH_NAME;

        ParsingError error = ParsingError::NONE;

        data.PopFront([&](char symbol) -> u32 {
            switch (state) {
            case SEARCH_NAME:
                switch (symbol) {
                default:
                    attributes.EmplaceBack();
                    state = READ_NAME;
                    break;
                case ' ':
                case '\n':
                case '\r':
                    return 1;
                case '/':
                case '>':
                    return 0;
                }
            case READ_NAME:
                switch (symbol) {
                case ' ':
                    if (attributes.back()->name)
                        state = SEARCH_VALUE;
                    break;
                case '=':
                    if (!attributes.back()->name) {
                        error = ParsingError::INVALID_ATTRIBUTE_NAME;
                        return 0;
                    }
                    state = SEARCH_VALUE;
                    break;
                case '\n':
                case '\r':
                    break;
                default:
                    attributes.back()->name.EmplaceBack(symbol);
                    break;
                }
                break;
            case SEARCH_VALUE:
                switch (symbol) {
                case '\"':
                    state = READ_VALUE;
                case '\r':
                case '\n':
                case '=':
                case ' ':
                    break;
                default:
                    error = ParsingError::INVALID_ATTRIBUTE_VALUE;
                    return 0;
                }
                break;
            case READ_VALUE:
                switch (symbol) {
                case '\"':
                    state = SEARCH_NAME;
                    break;
                default:
                    attributes.back()->value.EmplaceBack(symbol);
                    break;
                }
                break;
            }
        
            return 1;
        });

        return error;
    }

    consteval auto ReadAttributesTest() {
        StrView<> str = " a1=\"\" b2=\"0\">";

        StrView<> data = str;

        Array<Attribute> attributes = {};

        if (ReadAttributes(data, attributes) != ParsingError::NONE)
            return false;

        if (data.front() != str.back() || attributes.getLength() != 2)
            return false;

        if (attributes[0]->name.view() != "a1" || attributes[1]->name.view() != "b2")
            return false;

        if (attributes[0]->value || attributes[1]->value.view() != "0")
            return false;

        str = "attr=\"\"/";

        data = str;

        attributes.Clear();

        if (ReadAttributes(data, attributes) != ParsingError::NONE)
            return false;

        if (data.front() != str.back() || attributes.getLength() != 1)
            return false;

        return true;
    }

    static_assert(ReadAttributesTest());

    constexpr ParsingError ReadString(StrView<>& data, String& string) {
        enum {
            READ_STRING,
            READ_ENTITY,
            CHECK_ENTITY,
            READ_CDATA
        } state = READ_STRING;

        ParsingError error = ParsingError::NONE;

        char entity[4] = {};

        u8 symIndex = 0;

        constexpr StrView<> cdataStr = "![CDATA[";

        data.PopFront([&](char symbol) -> u32 {
            switch (state) {
            case CHECK_ENTITY:
                if (StrView<>(entity) == "lt")
                    string.EmplaceBack('<');
                else if (StrView<>(entity) == "gt")
                    string.EmplaceBack('>');
                else if (StrView<>(entity) == "amp")
                    string.EmplaceBack('&');
                else if (StrView<>(entity) == "apos")
                    string.EmplaceBack('\'');
                else if (StrView<>(entity) == "quot")
                    string.EmplaceBack('\"');
                else {
                    string.EmplaceBack('&');
                    string += entity;
                    string.EmplaceBack(';');
                }
                state = READ_STRING;
            case READ_STRING:
                switch (symbol) {
                case '<':
                    if (data.view(1, cdataStr.getLength()) == cdataStr) {
                        state = READ_CDATA;
                        return cdataStr.getLength() + 1;
                    }
                    return 0;
                case '&':
                    state = READ_ENTITY;
                    symIndex = 0;
                    for (auto& s : entity)
                        s = '\0';
                    break;
                default:
                    string.EmplaceBack(symbol);
                case '\r':
                    break;
                }
                break;
            case READ_ENTITY:
                if (symbol == ';') {
                    state = CHECK_ENTITY;
                    break;
                }
                if (symIndex == 4) {
                    string.EmplaceBack('&');
                    string += entity;
                    state = READ_STRING;
                    break;
                }
                entity[symIndex] = symbol;
                symIndex += 1;
                break;
            case READ_CDATA:
                if (symbol == ']' && data.view(1, 2) == "]>") {
                    state = READ_STRING;
                    return 3;
                }
                if (symbol != '\r')
                    string.EmplaceBack(symbol);
                break;
            }

            return 1;
        });

        return error;
    }

    consteval bool ReadStringTest() {
        StrView<> str = "A quick brown fox...<";

        StrView<> data = str;

        String result = {};

        if (ReadString(data, result) != ParsingError::NONE)
            return false;

        if (str.back() != data.front())
            return false;

        if (result.view() != "A quick brown fox...")
            return false;

        str = "amp: &amp; apos: &apos;<";

        data = str;

        result.Clear();

        if (ReadString(data, result) != ParsingError::NONE)
            return false;

        if (str.back() != data.front())
            return false;

        if (result.view() != "amp: & apos: \'")
            return false;

        str = "String. <![CDATA[<tag></tag> & ]]>other<";

        data = str;

        result.Clear();

        if (ReadString(data, result) != ParsingError::NONE)
            return false;

        if (str.back() != data.front())
            return false;

        if (result.view() != "String. <tag></tag> & other")
            return false;

        return true;
    }

    static_assert(ReadStringTest());

    constexpr ParsingError ReadComment(StrView<>& data) {
        enum {
            CHECK_START,
            PASS,
            CHECK_END,
            EXIT
        } state = CHECK_START;

        ParsingError error = ParsingError::NONE;

        data.PopFront([&](char symbol) -> u32 {
            switch (state) {
            case CHECK_START:
                if (data.view(1, 2) != "--") {
                    error = ParsingError::INVALID_COMMENT;
                    return 0;
                }
                state = PASS;
                return 3;
            case PASS:
                if (symbol == '-')
                    state = CHECK_END;
                break;
            case CHECK_END:
                if (data.viewTo(2) == "->")
                    state = EXIT;
                else
                    state = PASS;
                break;
            case EXIT:
                return 0;
            }

            return 1;
        });

        return error;
    }

    consteval bool ReadCommentTest() {
        StrView<> str = "!-- commentary -- <tag></tag> -->";

        StrView<> data = str;

        if (ReadComment(data) != ParsingError::NONE)
            return false;

        if (str.back() != data.front())
            return false;

        return true;
    }

    static_assert(ReadCommentTest());

    constexpr ParsingError ReadElement(StrView<>& data, Element& element) {
        enum {
            READ_NAME,
            READ_ATTRIBUTES,
            READ_STRING,
            READ_CHILD,
            CHECK_END,
            CHECK_SHORT_END,
            EXIT
        } state = READ_NAME;

        ParsingError error = ParsingError::NONE;

        data.PopFront([&](char symbol) -> u32 {
            switch (state) {
            case READ_NAME:
                switch (symbol) {
                case '/':
                    if (!element.name) {
                        error = ParsingError::INVALID_ELEMENT_TAG;
                        return 0;
                    }
                    state = CHECK_SHORT_END;
                    break;
                case '>':
                    if (!element.name) {
                        error = ParsingError::INVALID_ELEMENT_TAG;
                        return 0;
                    }
                    state = READ_STRING;
                    break;
                case ' ':
                    if (!element.name) {
                        error = ParsingError::INVALID_ELEMENT_TAG;
                        return 0;
                    }
                    state = READ_ATTRIBUTES;
                    break;
                default:
                    element.name.EmplaceBack(symbol);
                    break;
                }
                break;
            case READ_ATTRIBUTES:
                error = ReadAttributes(data, element.attributes);
                if (error != ParsingError::NONE)
                    return 0;
                if (data && *data.front() == '/')
                    state = CHECK_SHORT_END;
                else
                    state = READ_STRING;
                break;
            case READ_STRING:
                if (!element.values || !element.values.back()->isString())
                    element.values.EmplaceBack().SetString();
                error = ReadString(data, element.values.back()->string());
                if (error != ParsingError::NONE)
                    return 0;
                if (element.values.back()->string()->isVoid())
                    element.values.EraseBack();
                state = READ_CHILD;
                break;
            case READ_CHILD:
                switch (symbol) {
                case '/':
                    state = CHECK_END;
                    break;
                case '!':
                    error = ReadComment(data);
                    if (error != ParsingError::NONE)
                        return 0;
                    state = READ_STRING;
                    break;
                default:
                    error = ReadElement(data, element.values.EmplaceBack().SetElement());
                    if (error != ParsingError::NONE)
                        return 0;
                    state = READ_STRING;
                    break;
                }
                break;
            case CHECK_END:
                if (data.viewTo(element.name.getLength()) == element.name) {
                    state = EXIT;
                    return element.name.getLength();
                }
                error = ParsingError::INVALID_ELEMENT_TAG;
                return 0;
            case CHECK_SHORT_END:
                if (symbol != '>')
                    error = ParsingError::INVALID_ELEMENT_TAG;
            case EXIT:
                return 0;
            }

            return 1;
        });

        return error;
    }

    consteval int ReadElementTest() {
        StrView<> str = "element>Something.</element>";

        auto data = str;

        Element element = {};

        if (ReadElement(data, element) != ParsingError::NONE)
            return false;

        if (element.name.view() != "element" || str.back() != data.front())
            return false;

        if (element.values[0]->string()->view() != "Something.")
            return false;

        str = "root a=\"\"><!-- comment -->Aa</root>";

        data = str;

        element = {};

        if (ReadElement(data, element) != ParsingError::NONE)
            return false;

        if (element.name.view() != "root" || str.back() != data.front())
            return false;

        if (element.values[0]->string()->view() != "Aa" || element.values.getLength() != 1)
            return false;

        if (element.attributes.getLength() != 1 || element.attributes[0]->name.view() != "a")
            return false;

        return true;
    }

    static_assert(ReadElementTest());

    export constexpr ParsingResult Parse(StrView<> data) noexcept {
        enum {
            CHECK_TAG,
            SEARCH_PROLOG,
            READ_PROLOG,
            SEARCH_ROOT,
            READ_ROOT
        } state = SEARCH_PROLOG;

        ParsingResult result = {};

        data.PopFront([&](char symbol) -> u32 {
            try {
                switch (state) {
                case CHECK_TAG:
                    if (symbol == '?') {
                        state = READ_PROLOG;
                        break;
                    }
                case READ_ROOT:
                    result.error = ReadElement(data, result.root);
                    return 0;
                case SEARCH_PROLOG:
                    switch (symbol) {
                    case '<':
                        state = CHECK_TAG;
                    case ' ':
                    case '\r':
                    case '\n':
                        break;
                    default:
                        result.error = ParsingError::INVALID_STRUCTURE;
                        return 0;
                    }
                    break;
                case READ_PROLOG:
                    result.error = ReadProlog(data);
                    if (result.error != ParsingError::NONE)
                        return 0;
                    state = SEARCH_ROOT;
                    break;
                case SEARCH_ROOT:
                    switch (symbol) {
                    case '<':
                        state = READ_ROOT;
                    case ' ':
                    case '\n':
                    case '\r':
                        break;
                    default:
                        result.error = ParsingError::INVALID_STRUCTURE;
                        return 0;
                    }
                    break;
                }
            }
            catch (...) {
                result.error = ParsingError::FAIL;

                return 0;
            }

            return 1;
        });

        return result;
    }

    consteval bool ParseTest() {
        StrView<> data = "<root attr=\"10\"><element/>k <![CDATA[<tag></tag>]]></root>";

        auto result = Parse(data);

        if (!result || result.root.name.view() != "root" || result.root.values.getLength() != 2)
            return false;

        if (result.root.attributes.getLength() != 1)
            return false;

        auto attr = result.root.attributes[0];

        if (attr->name.view() != "attr" || attr->value.view() != "10")
            return false;

        if (result.root.values[1]->string()->view() != "k <tag></tag>")
            return false;

        auto value = result.root.values[0];

        if (value->element()->name.view() != "element" || value->element()->attributes || value->element()->values)
            return false;

        return true;
    }

    static_assert(ParseTest());
}
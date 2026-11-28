// SPDX-License-Identifier: MIT
// Copyright (c) 2026 som3man

export module ACL.XML:Element;

import ACL.Main;
import ACL.String;

namespace ACL {
    namespace XML {};
}

namespace ACL::XML {
    export struct Attribute {
        String name;

        String value;
    };

    export class Value;

    export struct Element {
        String name;

        Array<Attribute> attributes;

        Array<Value> values;

        [[nodiscard]] constexpr Optional<Attribute&> GetAttribute(StrView<> name) {
            for (auto& attr : attributes)
                if (attr.name.view() == name)
                    return attr;

            return {};
        }

        [[nodiscard]] constexpr Optional<const Attribute&> GetAttribute(StrView<> name) const {
            for (auto& attr : attributes)
                if (attr.name.view() == name)
                    return attr;

            return {};
        }

        [[nodiscard]] constexpr Optional<Element&> FindElement(StrView<> name);

        [[nodiscard]] constexpr Optional<const Element&> FindElement(StrView<> name) const;

        [[nodiscard]] constexpr Array<Optional<Element&>> TraceElement(StrView<> name);

        [[nodiscard]] constexpr Array<Optional<const Element&>> TraceElement(StrView<> name) const;
    };

    export class Value {
        Union<String, Element> data = String();

    public:
        constexpr Value() noexcept {};

        template <u32 capacity>
        constexpr Value(const char (&cstring)[capacity]) : data(String(cstring, capacity)) {};

        constexpr Value(StrView<> string) : data(string) {};

        constexpr Value(const String& string) : data(string) {};

        constexpr Value(String&& string) : data(Move(string)) {};

        constexpr Value(const Element& element) : data(element) {};

        constexpr Value(Element&& element) : data(Move(element)) {};

        constexpr Value(const Value& other) : data(other.data) {};

        constexpr Value(Value&& other) : data(Move(other.data)) {};

        constexpr ~Value() {};

        constexpr Value& operator =(const Value& other) {
            data = other.data;

            return *this;
        }

        constexpr Value& operator =(Value&& other) {
            data = Move(other.data);

            return *this;
        }

        template <typename... Args>
        constexpr String& SetString(Args&&... args) {
            return data.Set<String>(Forward<Args>(args)...);
        }

        template <typename... Args>
        constexpr Element& SetElement(Args&&... args) {
            return data.Set<Element>(Forward<Args>(args)...);
        }

        [[nodiscard]] constexpr Optional<String&> string() noexcept {
            return data.get<String>();
        }

        [[nodiscard]] constexpr Optional<const String&> string() const noexcept {
            return data.get<String>();
        }

        [[nodiscard]] constexpr Optional<Element&> element() noexcept {
            return data.get<Element>();
        }

        [[nodiscard]] constexpr Optional<const Element&> element() const noexcept {
            return data.get<Element>();
        }

        [[nodiscard]] constexpr bool isString() const noexcept {
            return data.is<String>();
        }

        [[nodiscard]] constexpr bool isElement() const noexcept {
            return data.is<Element>();
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& forString, auto&& forElement) {
            return data.Visit<Return>(
                Forward(forString),
                Forward(forElement)
            );
        }

        template <typename Return = void>
        constexpr Return Visit(auto&& forString, auto&& forElement) const {
            return data.Visit<Return>(
                Forward(forString),
                Forward(forElement)
            );
        }

        constexpr Value& ForString(auto&& func) {
            data.For<String>(Forward(func));

            return *this;
        }

        constexpr const Value& ForString(auto&& func) const {
            data.For<String>(Forward(func));

            return *this;
        }

        constexpr Value& ForElement(auto&& func) {
            data.For<Element>(Forward(func));

            return *this;
        }

        constexpr const Value& ForElement(auto&& func) const {
            data.For<Element>(Forward(func));

            return *this;
        }
    };

    constexpr Optional<Element&> Element::FindElement(StrView<> name) {
        for (auto& v : values) {
            if (v.isString())
                continue;

            auto e = v.element();

            if (e->name.view() == name)
                return e;

            e = e->FindElement(name);

            if (e)
                return e;
        }

        return {};
    }

    constexpr Optional<const Element&> Element::FindElement(StrView<> name) const {
        for (auto& v : values) {
            if (v.isString())
                continue;

            auto e = v.element();

            if (e->name.view() == name)
                return e;

            e = e->FindElement(name);

            if (e)
                return e;
        }

        return {};
    }

    constexpr bool TraceElementImpl(Array<Optional<Element&>>& trace, Element& element, StrView<> name) {
        trace.EmplaceBack(element);

        for (auto& v : element.values) {
            if (v.isString())
                continue;

            auto& e = *v.element();

            if (e.name == name) {
                trace.EmplaceBack(e);

                return true;
            }

            if (TraceElementImpl(trace, e, name))
                return true;
        }

        trace.EraseBack();

        return false;
    }

    constexpr bool TraceElementImpl(Array<Optional<const Element&>>& trace, const Element& element, StrView<> name) {
        trace.EmplaceBack(element);

        for (auto& v : element.values) {
            if (v.isString())
                continue;

            auto& e = *v.element();

            if (e.name == name) {
                trace.EmplaceBack(e);

                return true;
            }

            if (TraceElementImpl(trace, e, name))
                return true;
        }

        trace.EraseBack();

        return false;
    }

    constexpr Array<Optional<Element&>> Element::TraceElement(StrView<> name) {
        Array<Optional<Element&>> trace = {};

        TraceElementImpl(trace, *this, name);

        return trace;
    }

    constexpr Array<Optional<const Element&>> Element::TraceElement(StrView<> name) const {
        Array<Optional<const Element&>> trace = {};

        TraceElementImpl(trace, *this, name);

        return trace;
    }

    consteval bool ElementSearchTest() {
        Element root = {
            "root",
            {},
            {
                "hhkh",
                Element {
                    "target",
                    {},
                    {}
                },
                "ooo"
            }
        };

        auto fres = root.FindElement("target");

        if (&*fres != &*root.values[1]->element())
            return false;

        root.values = {
            Value("something")
        };

        fres = root.FindElement("target");

        if (fres)
            return false;

        root.values = {
            Element {
                "w",
                {},
                {
                    Element {
                        "wrong",
                        {},
                        {}
                    }
                }
            },
            Element {
                "s",
                {},
                {
                    Element {
                        "other",
                        {},
                        {}
                    },
                    Element {
                        "target",
                        {},
                        {}
                    }
                }
            }
        };

        fres = root.FindElement("target");

        if (&*fres != &*root.values[1]->element()->values[1]->element())
            return false;

        return true;
    }

    static_assert(ElementSearchTest());
}
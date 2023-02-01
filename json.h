#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cassert>

namespace json
{
    using std::string;
    using std::istream;
    using std::ostream;
    using std::nullptr_t;
    using std::variant;
    using std::ptrdiff_t;

    class Node;
    using Dict = std::map<string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : public variant<nullptr_t, Array, Dict, bool, ptrdiff_t, double, string>
    {
    public:
        using variant = variant;

        Node() = default;
        Node(nullptr_t);
        Node(Array array);
        Node(Dict map);
        Node(ptrdiff_t value);
        Node(double value);
        Node(bool value);
        Node(string value);

        bool IsNull() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;
        bool IsArray() const;
        bool IsMap() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        ptrdiff_t AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const string& AsString() const;

        Array& AsArray();
        Dict& AsMap();
        string& AsString();

        bool operator == (const Node& other) const;
        bool operator != (const Node& other) const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;
        Node& GetRoot();

        bool operator == (const Document& other) const;
        bool operator != (const Document& other) const;

    private:
        Node root_;
    };

    Document Load(istream& input);

    void Print(const Document& doc, ostream& output, int indent_step = 0, int start_indent = 0);

}  // namespace json
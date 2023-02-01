#include "json.h"
#include <optional>
#include <algorithm>
#include <mutex>
#include <cmath>

using namespace std;

namespace json
{
	const bool do_logs = true;
	ostream* /*const*/ log_err_stream = &cerr; // can cause warnings, remove "const" to cancel warnings
	mutex* const log_err_mutex = nullptr;
	void LogWarning(string_view message)
	{
		if constexpr (do_logs)
		{
			assert(do_logs == static_cast<bool>(log_err_stream));
			if (log_err_mutex)
			{
				log_err_mutex->lock();
			}

			*log_err_stream << message << "\n";

			if (log_err_mutex)
			{
				log_err_mutex->unlock();
			}
		}
	}

	//vv####################################################################################################################### inside-used functions and objects:

	//vv==================================================================================== print functions and objects:

	//vv----------------------------------------------------- indent handling objects:

	struct PrintContext
	{
		ostream& out;
		int indent_step = 4;
		int indent = 0;
		bool dont_put_bracket_in_next_line = true;

		void NextLine()
		{
			out.put('\n');
			for (int i = 0; i < indent; ++i)
			{
				out.put(' ');
			}
		}

		void PrepareToOpenBracket()
		{
			if (dont_put_bracket_in_next_line)
			{
				dont_put_bracket_in_next_line = false;
				return;
			}
			NextLine();
		}

		PrintContext Indented()
		{
			return PrintContext{ out, indent_step, indent + indent_step };
		}
	};

	//vv----------------------------------------------------- print functions:

	void PrintValue(const Node& val, PrintContext& context); // declaration is used for recursions that appear in Array and Dict

	void PrintValue(nullptr_t, PrintContext& context)
	{
		auto& out = context.out;
		out << "null"sv;
	}

	void PrintValue(const string& text, PrintContext& context)
	{
		auto& out = context.out;
		out << '"';
		for (char c : text)
		{
			switch (c)
			{
			case '\t':
				out << '\t';
				break;
			case '\n':
				out << R"(\n)";
				break;
			case '\r':
				out << R"(\r)";
				break;
			case '\"':
				out << R"(\")";
				break;
			case '\\':
				out << R"(\\)";
				break;
			default:
				if (isprint(c))
				{
					out << c;
				}
				else
				{
					string what = "algorithn cant handle this case: string contains unprintable char with code = "s + to_string(static_cast<int>(c)) + " dec"s;
					LogWarning(what);
					throw (runtime_error(what));
				}
				break;
			}
		}
		out << '"';
	}

	void PrintValue(bool b, PrintContext& context)
	{
		auto& out = context.out;
		out << boolalpha << b;
	}

	template <typename Num_t>
	void PrintValue(Num_t value, PrintContext& context)
	{
		auto& out = context.out;
		if constexpr (is_same_v<Num_t, double>)
		{
			if (!isfinite(value))
			{
				if (isinf(value))
				{
					LogWarning("atempt to print INF"sv);
					if (value > 0)
					{
						out << "1e309"sv;
					}
					else
					{
						out << "-1e309"sv;
					}
					return;
				}
				assert(isnan(value));
				LogWarning("atempt to print NAN"sv);
				PrintValue(nullptr, context);
			}
		}
		static_assert(is_same_v<Num_t, double> || is_same_v<Num_t, ptrdiff_t>);
		out << value;
	}

	void PrintValue(const Array& arr, PrintContext& context)
	{
		auto& out = context.out;
		context.PrepareToOpenBracket();
		out << '[';
		auto inside_context = context.Indented();
		bool first_loop = true;
		for (const Node& node : arr)
		{
			if (first_loop)
			{
				first_loop = false;
			}
			else
			{
				out << ',';
			}
			inside_context.NextLine();
			PrintValue(node, inside_context);
		}
		context.NextLine();
		out << ']';
	}

	void PrintValue(const Dict& dict, PrintContext& context)
	{
		auto& out = context.out;
		context.PrepareToOpenBracket();
		out << '{';
		auto inside_context = context.Indented();
		inside_context.dont_put_bracket_in_next_line = false;
		bool first_loop = true;
		for (const auto& [name, val] : dict)
		{
			if (first_loop)
			{
				first_loop = false;
			}
			else
			{
				out << ',';
			}
			inside_context.NextLine();
			PrintValue(name, inside_context);
			out << ':' << ' ';
			PrintValue(val, inside_context);
		}
		context.NextLine();
		out << '}';
	}

	void PrintValue(const Node& val, PrintContext& context)
	{
		visit([&context](const auto& value)
			{
				PrintValue(value, context);
			}, dynamic_cast<const Node::variant&>(val));
	}

	//vv====================================================================================== read functions:

	Node LoadNode(istream& input);// declaration is used for recursions that appear in Array and Dict

	// used for bool/nullptr_t
	void SkipText(istream& input, const string& expected_text)
	{
		const auto end = expected_text.end();
		for (auto it = expected_text.cbegin(); it != end; ++it)
		{
			if (input.get() != *it)
			{
				throw ParsingError("Expected text is \""s +
					expected_text +
					"\", but letter #"s +
					to_string(it - expected_text.cbegin()) +
					" is different"s);
			}
		}
	}

	Node LoadNumber(istream& input)
	{
		string parsed_num;

		auto read_char = [&parsed_num, &input]
		{
			parsed_num += static_cast<char>(input.get());
			if (!input)
			{
				throw ParsingError("Failed to read number from stream"s);
			}
		};

		auto read_digits = [&input, read_char]
		{
			if (!isdigit(input.peek()))
			{
				throw ParsingError("A digit is expected"s);
			}
			while (isdigit(input.peek()))
			{
				read_char();
			}
		};

		//vv integer part and sign
		if (input.peek() == '-')
		{
			read_char();
		}
		if (input.peek() == '0')
		{
			read_char();
		}
		else
		{
			read_digits();
		}

		bool is_int = true;

		//vv past-point optional part
		if (input.peek() == '.')
		{
			read_char();
			read_digits();
			is_int = false;
		}

		//vv exponentional optional part
		if (int ch = input.peek(); ch == 'e' || ch == 'E')
		{
			read_char();
			if (ch = input.peek(); ch == '+' || ch == '-')
			{
				read_char();
			}
			read_digits();
			is_int = false;
		}

		try
		{
			if (is_int)
			{
				try
				{
					return static_cast<ptrdiff_t>(stoll(parsed_num));
				}
				catch (...)
				{
					LogWarning("Failed to convert "s + parsed_num + " to int, it will become double"s);
				}
			}
			return stod(parsed_num);
		}
		catch (...)
		{
			throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
		}
	}

	string LoadString(istream& input)
	{
		auto it = istreambuf_iterator<char>(input);
		auto end = istreambuf_iterator<char>();
		string s;

		assert(*it == '"');
		++it;

		while (true)
		{
			if (it == end)
			{
				throw ParsingError("String parsing error, unexpected end of input, closing quotes expected");
			}
			const char ch = *it;
			if (ch == '"')
			{
				++it;
				break;
			}
			else if (ch == '\\')
			{
				++it;
				if (it == end)
				{
					throw ParsingError("String parsing error, unexpected end of input, escape-sequence expected");
				}
				const char escaped_char = *(it);
				switch (escaped_char)
				{
				case 'n':
					s.push_back('\n');
					break;
				case 't':
					s.push_back('\t');
					break;
				case 'r':
					s.push_back('\r');
					break;
				case '"':
					s.push_back('"');
					break;
				case '\\':
					s.push_back('\\');
					break;
				default:
					throw ParsingError("String parsing error, unrecognized escape sequence \\"s + escaped_char);
				}
			}
			else if (ch == '\n' || ch == '\r')
			{
				throw ParsingError("String parsing error, unexpected end of line"s);
			}
			else
			{
				s.push_back(ch);
			}
			++it;
		}

		return s;
	}

	Array LoadArray(istream& input)
	{
		Array result;
		char c;

		assert(input.peek() == '[');
		input.get();

		input >> c;
		if (c == ']')
		{
			return result;
		}
		else
		{
			input.putback(c);
		}

		while (true)
		{
			if (!input)
			{
				throw ParsingError("Array parsing error, unexpected end of file.");
			}
			result.push_back(LoadNode(input));
			input >> c;
			if (c == ']')
			{
				return result;
			}
			if (c != ',')
			{
				throw ParsingError("Array parsing error, ',' or ']' expected.");
			}
		}
	}

	Dict LoadDict(istream& input)
	{
		auto ThrowEndOfFile = []()
		{
			throw ParsingError("Dict parsing error, unexpected end of file.");
		};

		Dict result;
		char c;

		assert(input.peek() == '{');
		input.get();

		input >> c;
		if (c == '}')
		{
			return result;
		}
		else
		{
			if (!input)
			{
				ThrowEndOfFile();
			}
			input.putback(c);
		}

		while (true)
		{
			if (!input)
			{
				ThrowEndOfFile();
			}

			pair<string, Node> loaded_pair;
			auto& [name, node] = loaded_pair;

			input >> c;
			if (c != '"')
			{
				throw ParsingError("Dict parsing error, unexpected char, '\"' was expected"s);
			}
			input.putback('"');
			name = LoadString(input);

			input >> c;
			if (c != ':')
			{
				throw ParsingError("Dict parsing error, unexpected char, ':' was expected"s);
			}

			node = LoadNode(input);

			auto [_, is_unique] = result.insert(move(loaded_pair));
			if (!is_unique)
			{
				throw ParsingError("Dict parsing error, name collision"s);
			}

			input >> c;
			if (c == '}')
			{
				return result;
			}
			if (c != ',')
			{
				throw ParsingError("Dict parsing error, ',' or '}' expected.");
			}
		}
	}

	Node LoadNode(istream& input)
	{
		char c;
		input >> c;
		input.putback(c);

		switch (c)
		{
		case 't':
			SkipText(input, "true"s);
			return Node(true);
			break;
		case 'f':
			SkipText(input, "false"s);
			return Node(false);
			break;
		case 'n':
			SkipText(input, "null"s);
			return Node(nullptr);
			break;
		case '"':
			return Node(LoadString(input));
			break;
		case '[':
			return Node(LoadArray(input));
			break;
		case '{':
			return Node(LoadDict(input));
			break;
		case '-':
			return LoadNumber(input);
			break;
		default:
			if (isdigit(c))
			{
				return LoadNumber(input);
			}
			else
			{
				throw ParsingError("Failed to define node type"s);
			}
			break;
		}
	}

	//vv####################################################################################################################### header-related:

	//vv====================================================================================== class"Node" inplementations

	//vv----------------------- constructors:

	Node::Node(nullptr_t)
		: variant(nullptr)
	{}

	Node::Node(Array array)
		: variant(array)
	{}

	Node::Node(Dict map)
		: variant(map)
	{}

	Node::Node(ptrdiff_t value)
		: variant(value)
	{}

	Node::Node(double value)
		: variant(value)
	{}

	Node::Node(bool value)
		: variant(value)
	{}

	Node::Node(string value)
		: variant(value)
	{}

	//vv-------------------- variable type getters:

	bool Node::IsNull() const
	{
		return holds_alternative<nullptr_t>(*this);
	}

	bool Node::IsBool() const
	{
		return holds_alternative<bool>(*this);
	}

	bool Node::IsInt() const
	{
		return holds_alternative<ptrdiff_t>(*this);
	}

	bool Node::IsDouble() const
	{
		return IsPureDouble() || IsInt();
	}

	bool Node::IsPureDouble() const
	{
		return holds_alternative<double>(*this);
	}

	bool Node::IsString() const
	{
		return holds_alternative<string>(*this);
	}

	bool Node::IsArray() const
	{
		return holds_alternative<Array>(*this);
	}

	bool Node::IsMap() const
	{
		return holds_alternative<Dict>(*this);
	}

	//vv------------------------------------- variable value getters:

	const Array& Node::AsArray() const
	{
		if (IsArray())
		{
			return get<Array>(*this);
		}
		throw logic_error("node doesnt hold array"s);
	}

	Array& Node::AsArray()
	{
		if (IsArray())
		{
			return get<Array>(*this);
		}
		throw logic_error("node doesnt hold array"s);
	}

	const Dict& Node::AsMap() const
	{
		if (IsMap())
		{
			return get<Dict>(*this);
		}
		throw logic_error("node doesnt hold map"s);
	}

	Dict& Node::AsMap()
	{
		if (IsMap())
		{
			return get<Dict>(*this);
		}
		throw logic_error("node doesnt hold map"s);
	}

	ptrdiff_t Node::AsInt() const
	{
		if (IsInt())
		{
			return get<ptrdiff_t>(*this);
		}
		throw logic_error("node doesnt hold int"s);
	}

	double Node::AsDouble() const
	{
		if (IsPureDouble())
		{
			return get<double>(*this);
		}
		if (IsInt())
		{
			return static_cast<double>(get<ptrdiff_t>(*this));
		}
		throw logic_error("node doesnt hold double or int"s);
	}

	bool Node::AsBool() const
	{
		if (IsBool())
		{
			return get<bool>(*this);
		}
		throw logic_error("node doesnt hold bool"s);
	}

	const string& Node::AsString() const
	{
		if (IsString())
		{
			return get<string>(*this);
		}
		throw logic_error("node doesnt hold string"s);
	}

	string& Node::AsString()
	{
		if (IsString())
		{
			return get<string>(*this);
		}
		throw logic_error("node doesnt hold string"s);
	}

	//vv--------------------------------- operators

	bool Node::operator==(const Node& other) const
	{
		return static_cast<const variant&>(*this) == static_cast<const variant&>(other);
	}

	bool Node::operator!=(const Node& other) const
	{
		return !(*this == other);
	}

	//vv===================================================================================================== class"Document" and functions related to it

	//vv------------------------------------- class "Document"

	Document::Document(Node root)
		: root_(move(root))
	{}

	const Node& Document::GetRoot() const
	{
		return root_;
	}

	Node& Document::GetRoot()
	{
		return root_;
	}

	bool Document::operator==(const Document& other) const
	{
		return GetRoot() == other.GetRoot();
	}

	bool Document::operator!=(const Document& other) const
	{
		return !(*this == other);
	}

	//vv------------------------------------------- functions related to "Document"

	Document Load(istream& input)
	{
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, ostream& output, int indent_step, int start_indent)
	{
		PrintContext context = { output, indent_step, start_indent };
		PrintValue(doc.GetRoot(), context);
	}

}
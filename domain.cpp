#include "domain.h"

#include <functional>
#include <cassert>
#include <algorithm>

namespace text_processing
{
	StringSelfView::StringSelfView(const string_view& text)
		: body(nullptr), view(text) {}

	StringSelfView::StringSelfView(string&& text)
		: body(new string(move(text))), view(*body) {}

	StringSelfView::StringSelfView(const StringSelfView& other)
	{
		if (other.IsIndependend())
		{
			body.reset(new string(other.view));
			view = *body;
		}
		else
		{
			view = other.view;
		}
	}

	StringSelfView::StringSelfView(StringSelfView&& other)
		: body(other.body.release()), view(other.view) {}

	StringSelfView& StringSelfView::operator=(const StringSelfView& other)
	{
		if (other.IsIndependend())
		{
			body.reset(new string(other.view));
			view = *body;
		}
		else
		{
			body.reset();
			view = other.view;
		}
		return *this;
	}

	StringSelfView& StringSelfView::operator=(StringSelfView&& other)
	{
		if (other.IsIndependend())
		{
			body.reset(other.body.release());
			view = *body;
		}
		else
		{
			body.reset();
			view = other.view;
		}

		return *this;
	}

	bool StringSelfView::operator==(const StringSelfView& other) const
	{
		return other.view == view;
	}

	bool StringSelfView::operator!=(const StringSelfView& other) const
	{
		return !(*this == other);
	}

	bool StringSelfView::IsIndependend() const
	{
		return static_cast<bool>(body);
	}

	StringSelfView::operator const string_view& () const
	{
		return view;
	}

	StringSelfView::operator const string& () const
	{
		return *body.get();
	}

	const string_view& StringSelfView::GetSV() const
	{
		return *this;
	}

	const string& StringSelfView::GetStr() const
	{
		return *this;
	}

	size_t StringSelfView::hasher::operator()(const StringSelfView& data) const
	{
		return std::hash<string_view>()(data);
	}

	StringSelfView operator""_ssv(const char* str, size_t size)
	{
		return StringSelfView(move(string(str, size)));
	}

	SubstringSuffix::SubstringSuffix(const string& parent, const string_view& data)
		: offset(data.data() - parent.data()), size(data.size())
	{
		assert(offset <= parent.size());
		assert(size <= parent.size());
		assert(offset + size <= parent.size());
	}

	SubstringSuffix::SubstringSuffix(unsigned init_offset, unsigned init_size)
		: offset(init_offset)
		, size(init_size)
	{}

	string_view SubstringSuffix::GetView(string_view parent) const
	{
		return parent.substr(offset, size);
	}

	SubstringBody::SubstringBody(const string& body)
		: body_ptr_(&body) {}

	string_view SubstringBody::operator()(const SubstringSuffix& suff)
	{
		return suff.GetView(*body_ptr_);
	}

	string_view& RemovePrefixSpaces(string_view& str)
	{
		if (!str.empty()) str = str.substr(str.find_first_not_of(' '));
		return str;
	}

	string_view& RemovePostfixSpaces(string_view& str)
	{
		if (!str.empty()) str = str.substr(0, str.find_last_not_of(' ') + 1);
		return str;
	}

	string_view& RemovePrefixPostfixSpaces(string_view& str)
	{
		RemovePrefixSpaces(str);
		RemovePostfixSpaces(str);
		return str;
	}

	deque<string_view> SplitByChar(string_view text, char separator)
	{
		deque<string_view> words;
		while (true)
		{
			text.remove_prefix(std::min(text.find_first_not_of(separator), text.size()));
			if (text.empty()) break;
			size_t wordsize = std::min(text.find_first_of(separator), text.size());
			words.push_back(text.substr(0, wordsize));
			text.remove_prefix(wordsize);
		}
		return words;
	}

	string_view CutPrefixByChar(string_view& text, char separator)
	{
		string_view prefix = text.substr(0, text.find_first_of(separator));
		text.remove_prefix(prefix.size() + 1);
		return prefix;
	}
}

namespace transport_guide
{
	using namespace text_processing;
	using namespace std::string_view_literals;

	string_view string_to_sv_adapter(const string& s)
	{
		return s;
	}

	bool IsSetNotGetComand(string_view comand)
	{
		return std::any_of(comand.begin(), comand.end(), [](char c) {return c == ':'; });
	}

	bool IsStopNotBusComand(string_view text)
	{
		assert([&text]()
			{
				string_view prefix = CutPrefixByChar(text, ' ');
		return (prefix == "Stop"sv) || (prefix == "Bus"sv);
			}());
		return text.front() == 'S';
	}

	double RouteInfo::GetCurvature() const
	{
		return road_lenght / straight_lenght;
	}

	bool StopPair::operator==(const StopPair& stop_pair) const
	{
		return (stop_pair.from == from) && (stop_pair.to == to);
	}

	bool StopPair::operator!=(const StopPair& stop_pair) const
	{
		return !(*this == stop_pair);
	}

	void StopPair::Reverse()
	{
		swap(from, to);
	}
}
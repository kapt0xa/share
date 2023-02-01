#pragma once
#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>
#include <iostream>
#include <memory>
#include <deque>

namespace text_processing
{
	using std::string;
	using std::string_view;
	using std::ostream;
	using std::istream;
	using std::deque;

	class StringSelfView// owns it's string and can be converted to string_view
	{
		std::unique_ptr<string> body;
		string_view view;
	public:
		StringSelfView(const string_view& text);
		StringSelfView(string&& text);
		StringSelfView() = default;
		StringSelfView(const StringSelfView& other);
		StringSelfView(StringSelfView&& other);
		StringSelfView& operator = (const StringSelfView& other);
		StringSelfView& operator = (StringSelfView&& other);
		bool operator == (const StringSelfView& other) const;
		bool operator != (const StringSelfView& other) const;
		bool IsIndependend() const;
		operator const string_view& () const;
		operator const string& () const;
		const string_view& GetSV() const;
		const string& GetStr() const;
		struct hasher
		{
			size_t operator()(const StringSelfView& data) const;
		};
	};

	StringSelfView operator "" _ssv(const char* str, size_t size);

	struct SubstringSuffix // like string_view, but based on indexes, not pointers. cant be converted into string_view without body
	{
		unsigned offset = 0;
		unsigned size = 0;
		SubstringSuffix() = default;
		SubstringSuffix(const string& parent, const string_view& data);
		SubstringSuffix(unsigned init_offset, unsigned init_size);
		string_view GetView(string_view parent) const;
	};

	class SubstringBody // for substring_suffix, operator()(substring_suffix) returns string_view
	{
		const string* const body_ptr_;
	public:
		SubstringBody() = delete;
		SubstringBody(const string& body);
		SubstringBody(const SubstringBody&) = default;
		SubstringBody& operator = (const SubstringBody&) = default;
		SubstringBody(SubstringBody&&) = default;
		SubstringBody& operator = (SubstringBody&&) = default;
		string_view operator()(const SubstringSuffix& suff);
	};

	string_view& RemovePrefixSpaces(string_view& str);

	string_view& RemovePostfixSpaces(string_view& str);

	string_view& RemovePrefixPostfixSpaces(string_view& str);

	deque<string_view> SplitByChar(string_view text, char separator);

	string_view CutPrefixByChar(string_view& text, char separator);
}

namespace transport_guide
{
	using std::string_view;
	using std::unordered_set;
	using std::string;
	using std::vector;
	using std::is_same_v;
	using std::move;

	template <typename StringType>
	struct Route
	{
		Route() = default;

		template <typename It, typename Func>
		Route(It stop_names_begin, size_t route_size, Func string_view_adapter, bool is_line);

		vector<StringType> stop_names;
		bool is_line = false;
	};

	using RouteSV = Route<string_view>;
	using RouteS = Route<string>;

	string_view string_to_sv_adapter (const string& s);

	bool IsSetNotGetComand(string_view comand);

	bool IsStopNotBusComand(string_view text);

	struct RouteInfo
	{
		size_t stop_count;
		size_t unique_stop_count;
		double straight_lenght;
		size_t road_lenght;
		double GetCurvature() const;
	};

	using StopInfo = unordered_set<string_view>;

	struct StopPair
	{
		string_view from;
		string_view to;
		bool operator == (const StopPair& stop_pair) const;
		bool operator != (const StopPair& stop_pair) const;
		void Reverse();
	};

	//template definitions =====================================

	template <typename StringType>
	template<typename It, typename Func>
	inline Route<StringType>::Route(It stop_names_begin, const size_t route_size, Func string_t_adapter /* transforms element form range into string_view */, bool is_line)
	{
		this->is_line = is_line;
		stop_names.reserve(route_size);

		for (; stop_names.size() < route_size; ++stop_names_begin)
		{
			stop_names.push_back(string_t_adapter(*stop_names_begin));
		}
	}
}
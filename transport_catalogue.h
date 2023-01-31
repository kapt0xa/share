#pragma once

#include <string>
#include <string_view>
#include <deque>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>
#include <utility>
#include <algorithm>
#include <numeric>
#include <optional>
#include <functional>
#include <cstdint>

#include "geo.h"
#include "domain.h"

namespace transport_guide
{
	using std::unordered_map;
	using std::unordered_set;
	using std::pair;
	using std::tuple;
	using std::string;
	using std::string_view;
	using std::unique_ptr;
	using std::vector;
	using std::optional;
	using namespace text_processing;

	class Catalogue
	{
	public:

		void AddStop(string_view stop_name, geo::Coordinates cords);
		void AddRoad(StopPair stops, size_t distance);
		template <typename StringType, typename Func>
		void AddBus(string_view bus_name, const Route<StringType>& route, const Func& string_view_adapter);

		size_t GetRoad(StopPair stops) const;
		const RouteInfo* GetBusInfo(string_view bus_name) const;
		const StopInfo* GetStopInfo(string_view stop_name) const;

	private:

		struct pair_string_hasher
		{ size_t operator()(const StopPair& data) const; };

		string_view& RereferenceStringViewToLocalBusName(string_view& unlocaled_view) const;
		string_view& RereferenceStringViewToLocalStopName(string_view& unlocaled_view) const;
		StopPair& RereferenceStringViewPairToLocalStopNames(StopPair& unlocaled_views) const;

		double CalculateDistance(const StopPair& stop_names) const;

		struct RouteData
		{
			RouteSV route;
			unique_ptr<RouteInfo> info;
		};

		struct StopData
		{
			geo::Coordinates cords;
			StopInfo passing_buses;
		};

		unordered_map<StringSelfView, RouteData, StringSelfView::hasher> bus_name_to_route_;
		unordered_map<StringSelfView, StopData, StringSelfView::hasher> stop_name_to_info_;
		unordered_map<StopPair, size_t, pair_string_hasher> stoppair_to_distance_;
	};

	//template definitions =====================================

	template<typename StringType, typename Func>
	inline void Catalogue::AddBus(string_view bus_name, const Route<StringType>& route_input, const Func& string_type_to_string_view_adapter)
	{
		RouteSV& route = bus_name_to_route_[string(bus_name)].route;
		route.is_line = route_input.is_line;
		route.stop_names.reserve(route_input.stop_names.size());
		for (const auto& stopname_raw : route_input.stop_names)
		{
			string_view stopname = string_type_to_string_view_adapter(stopname_raw);
			auto stop_it = stop_name_to_info_.find(stopname);
			if (stop_it != stop_name_to_info_.end())
			{
				auto& [_, set_of_buses] = stop_it->second;
				set_of_buses.insert(RereferenceStringViewToLocalBusName(bus_name));
				route.stop_names.push_back(stopname);
			}
		}
	}
}
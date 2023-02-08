#include "transport_catalogue.h"
#include <cassert>

using namespace std;
using namespace geo;

namespace transport_guide
{
	const size_t large_prime = 19937;
	size_t Catalogue::pair_string_hasher::operator()(const StopPair& data) const
	{
		hash<string_view> hasher;
		return hasher(data.from) ^ hasher(data.to) * large_prime;
	}

	void Catalogue::AddStop(string_view stop_name, geo::Coordinates cords)
	{
		stop_name_to_info_[move(string(stop_name))].cords = cords;
	}

	void Catalogue::AddRoad(StopPair stops, size_t distance)
	{
		RereferenceStringViewPairToLocalStopNames(stops);
		stoppair_to_distance_[stops] = distance;
	}

	double Catalogue::CalculateDistance(const StopPair& stop_names) const
	{
		return ComputeDistance(
			stop_name_to_info_.at(stop_names.from).cords,
			stop_name_to_info_.at(stop_names.to).cords);
	}

	size_t Catalogue::GetRoad(StopPair stops) const
	{
		auto it = stoppair_to_distance_.find(stops);

		if (it == stoppair_to_distance_.end())
		{
			stops.Reverse();
			it = stoppair_to_distance_.find(stops);
		}
		assert(it != stoppair_to_distance_.end());
		return it->second;
	}

	const RouteInfo* Catalogue::GetBusInfo(string_view bus_name) const
	{
		auto bus_it = bus_name_to_route_.find(bus_name);
		if (bus_it == bus_name_to_route_.end()) return nullptr;

		const auto& [route, route_stats] = bus_it->second;

		if (!route_stats)
		{
			const_cast<unique_ptr<RouteInfo>&>(route_stats).reset(new RouteInfo);
			auto& new_stats = const_cast<RouteInfo&>(*route_stats);

			//stop count --vv--
			if (route.is_line)
			{
				new_stats.stop_count = route.stop_names.size() * 2 - 1;
			}
			else
			{
				new_stats.stop_count = route.stop_names.size();
			}

			//unique stops --vv--
			auto route_copy = route.stop_names;
			sort(route_copy.begin(), route_copy.end());
			auto it = unique(route_copy.begin(), route_copy.end());
			new_stats.unique_stop_count = it - route_copy.begin();

			//route lenght --vv--
			new_stats.straight_lenght = 0;
			new_stats.road_lenght = 0;
			for (auto A_it = route.stop_names.begin(), B_it = route.stop_names.begin() + 1;
				B_it != route.stop_names.end(); ++A_it, ++B_it)
			{
				StopPair stop_pair = { *A_it, *B_it };
				new_stats.straight_lenght += CalculateDistance(stop_pair);
				new_stats.road_lenght += GetRoad(stop_pair);
			}
			if (route.is_line)
			{
				new_stats.straight_lenght *= 2;
				for (auto A_it = route.stop_names.rbegin(), B_it = route.stop_names.rbegin() + 1;
					B_it != route.stop_names.rend(); ++A_it, ++B_it)
				{
					StopPair stop_pair = { *A_it, *B_it };
					new_stats.road_lenght += GetRoad(stop_pair);
				}
			}
		}
		return route_stats.get();
	}

	const StopInfo* Catalogue::GetStopInfo(string_view stop_name) const
	{
		auto stop_it = stop_name_to_info_.find(stop_name);
		if (stop_it == stop_name_to_info_.end()) return nullptr;
		return &stop_it->second.passing_buses;
	}

	string_view& Catalogue::RereferenceStringViewToLocalBusName(string_view& unlocaled_view) const
	{
		return unlocaled_view = bus_name_to_route_.find(unlocaled_view)->first;
	}

	string_view& Catalogue::RereferenceStringViewToLocalStopName(string_view& unlocaled_view) const
	{
		return unlocaled_view = stop_name_to_info_.find(unlocaled_view)->first;
	}

	StopPair& Catalogue::RereferenceStringViewPairToLocalStopNames(StopPair& unlocaled_views) const
	{
		RereferenceStringViewToLocalStopName(unlocaled_views.from);
		RereferenceStringViewToLocalStopName(unlocaled_views.to);
		return unlocaled_views;
	}
}
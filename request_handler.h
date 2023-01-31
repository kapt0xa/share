#pragma once
#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"
#include <variant>


namespace transport_guide
{
	using namespace geo;
	using std::variant;
	using std::monostate;
	using namespace std::string_literals;

	const string default_string = "<DEFAULT_STRING>"s;
	const string not_found_error = "not found"s;

	struct BaseRequests
	{
		struct Bus
		{
			string name;
			RouteS route;

			void AddBus(Catalogue& catalogue) const;
		};

		struct Stop
		{
			string name;
			Coordinates cords;
			unordered_map<string, size_t> roads;

			void AddStop(Catalogue& catalogue) const;
			void AddRoads(Catalogue& catalogue) const;
		};

		void UseRequests(Catalogue& catalogue) const;

		deque<Stop> stops;
		deque<Bus> buses;
	};

	struct Reports
	{

		struct Bus
		{
			RouteInfo details;
		};

		struct Stop
		{
			StopInfo details;
		};

		struct Error
		{
			const string& error_message = default_string;
		};

		struct Report
		{
			ptrdiff_t request_id;
			variant<monostate, Bus, Stop, Error> details;
		};

		deque<Report> reports;
	};

	struct StatRequests
	{
		struct BusDetails
		{
			string name;
		};

		struct StopDetails
		{
			string name;
		};

		struct Request
		{
			ptrdiff_t id;
			variant<monostate, BusDetails, StopDetails> details;

			Reports::Report UseRequest(const Catalogue& catalogue) const;
		};

		Reports UseRequests(const Catalogue& catalogue) const;

		deque<Request> requests;
	};

	struct InputFile
	{
		Reports UseInputFile(Catalogue& catalogue) const;

		BaseRequests base_requests;
		StatRequests stat_requests;
	};
}
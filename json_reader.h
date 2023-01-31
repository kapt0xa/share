#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "request_handler.h"

namespace transport_guide
{
	using std::variant;
	using std::monostate;
	using std::string;
	using std::unordered_map;
	using namespace std::string_view_literals;
	using namespace std::string_literals;
	using namespace geo;
	using namespace json;

	//vv===================================================== setting

	namespace json_str_consts
	{
		const string type_key = "type"s;
		const string stop_type = "Stop"s;
		const string bus_type = "Bus"s;
		const string lat_key = "latitude"s;
		const string long_key = "longitude"s;
		const string distences_key = "road_distances"s;
		const string name_key = "name"s;
		const string stops_key = "stops"s;
		const string setting_req_key = "base_requests"s;
		const string getting_req_key = "stat_requests"s;
		const string not_line_route_key = "is_roundtrip"s;
		const string id_key_input = "id"s;
		const string id_key_output = "request_id"s;
		const string curvature_key = "curvature"s;
		const string route_lenght_key = "route_lenght"s;
		const string stop_count_key = "stop_count"s;
		const string unique_stops_key = "unique_stops_count"s;
		const string buses_key = "buses"s;
		const string error_msg_key = "error_message"s;
	}

    BaseRequests ParseBaseRequests(const Array& nodes_with_base_requests);

    StatRequests ParseStatRequests(const Array& nodes_with_stat_requests);

    InputFile ParseInputFile(const Document& input);

	Document ToJSON(const Reports& data);

	InputFile ReadInputFile(istream& in);

	void PrintReports(const Reports& data, ostream& out);
}
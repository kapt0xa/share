#include "json_reader.h"

using namespace std;
using namespace json;

namespace transport_guide
{

    void ParseBaseRequest(const Node& request_node, BaseRequests& storage);

    void ParseBusBaseRequest(const Dict& request, BaseRequests& storage);

    void ParseStopBaseRequest(const Dict& request, BaseRequests& storage);

    void ParseStatRequest(const Node& request_node, StatRequests& storage);

    void ParseBusStatRequest(ptrdiff_t id, const Dict& request, StatRequests& storage);

    void ParseStopStatRequest(ptrdiff_t id, const Dict& request, StatRequests& storage);

    Node ToJSON(const Reports::Report& data);

    void ToJSON(const Reports::Bus& data, Dict& storage);

    void ToJSON(const Reports::Stop& data, Dict& storage);

    void ToJSON(const Reports::Error& data, Dict& storage);

    //--------------------------------------------------------------------------------

    using namespace json_str_consts;

    BaseRequests ParseBaseRequests(const Array& nodes_with_base_requests)
    {
        BaseRequests result;
        for (const Node& node : nodes_with_base_requests)
        {
            ParseBaseRequest(node, result);
        }
        return result;
    }

    void ParseBaseRequest(const Node& request_node, BaseRequests& storage)
    {
        const Dict& request = request_node.AsMap();
        const string& type = request.at(type_key).AsString();
        assert(type == stop_type || type == bus_type);
        assert(stop_type.front() != bus_type.front());
        if (type.front() == stop_type.front())
        {
            ParseStopBaseRequest(request, storage);
        }
        else
        {
            ParseBusBaseRequest(request, storage);
        }
    }

    void ParseBusBaseRequest(const Dict& request, BaseRequests& storage)
    {
        const Array& stops_on_route = request.at(stops_key).AsArray();

        bool is_line = !request.at(not_line_route_key).AsBool();

        RouteS crunch(
            stops_on_route.begin(),
            stops_on_route.size() - (is_line ? 0 : 1),                                              // debug atention! size - 1 here!
            [](const Node& node)-> const string& { return node.AsString(); },
            is_line);

        storage.buses.push_back(
            {
                request.at(name_key).AsString(),
                move(crunch)
            });
    }

    void ParseStopBaseRequest(const Dict& request, BaseRequests& storage)
    {
        storage.stops.push_back(
            {
                request.at(name_key).AsString(),

                {
                    request.at(lat_key).AsDouble(),
                    request.at(long_key).AsDouble()
                },

                [&request]()->unordered_map<string, size_t>
                {
                    auto it = request.find(distences_key);
                    if (it == request.end())
                    {
                        return {};
                    }
                    unordered_map<string, size_t> result;
                    const Dict& dict = it->second.AsMap();
                    for (const auto& [stop_name, distance_node] : dict)
                    {
                        result[stop_name] = static_cast<size_t>(distance_node.AsInt());
                    }
                    return result;
                }()
            });
    }

    StatRequests ParseStatRequests(const Array& nodes_with_stat_requests)
    {
        StatRequests result;
        for (const Node& node : nodes_with_stat_requests)
        {
            ParseStatRequest(node, result);
        }
        return result;
    }

    void ParseStatRequest(const Node& request_node, StatRequests& storage)
    {
        const Dict& request = request_node.AsMap();
        ptrdiff_t id = request.at(id_key_input).AsInt();
        const string& type = request.at(type_key).AsString();
        assert(type == stop_type || type == bus_type);
        assert(stop_type.front() != bus_type.front());
        if (type.front() == stop_type.front())
        {
            ParseStopStatRequest(id, request, storage);
        }
        else
        {
            ParseBusStatRequest(id, request, storage);
        }
    }

    void ParseBusStatRequest(ptrdiff_t id, const Dict& request, StatRequests& storage)
    {
        storage.requests.push_back({ id, StatRequests::BusDetails{request.at(name_key).AsString()} });
    }

    void ParseStopStatRequest(ptrdiff_t id, const Dict& request, StatRequests& storage)
    {
        storage.requests.push_back({ id, StatRequests::StopDetails{request.at(name_key).AsString()} });
    }

    InputFile ParseInputFile(const Document& input)
    {
        const Dict& root = input.GetRoot().AsMap();
        return
        {
            ParseBaseRequests(root.at(setting_req_key).AsArray()),
            ParseStatRequests(root.at(getting_req_key).AsArray())
        };
    }

    Node ToJSON(const Reports::Report& data)
    {
        Dict result;
        result[id_key_output] = Node(data.request_id);
        if (holds_alternative<Reports::Bus>(data.details))
        {
            ToJSON(get<Reports::Bus>(data.details), result);
            return result;
        }
        if (holds_alternative<Reports::Stop>(data.details))
        {
            ToJSON(get<Reports::Stop>(data.details), result);
            return result;
        }
        if (holds_alternative<Reports::Error>(data.details))
        {
            ToJSON(get<Reports::Error>(data.details), result);
            return result;
        }
        assert(false);
    }

    void ToJSON(const Reports::Bus& data, Dict& storage)
    {
        storage[curvature_key] = Node(data.details.GetCurvature());
        storage[route_lenght_key] = Node(static_cast<ptrdiff_t>(data.details.road_lenght));
        storage[stop_count_key] = Node(static_cast<ptrdiff_t>(data.details.stop_count));
        storage[unique_stops_key] = Node(static_cast<ptrdiff_t>(data.details.unique_stop_count));
    }

    void ToJSON(const Reports::Stop& data, Dict& storage)
    {
        Array buses_pre_node;
        buses_pre_node.reserve(data.details.size());
        for (const string_view& bus : data.details)
        {
            buses_pre_node.push_back(string(bus));
        }
        storage[buses_key] = Node(move(buses_pre_node));
    }

    void ToJSON(const Reports::Error& data, Dict& storage)
    {
        storage[error_msg_key] = Node(data.error_message);
    }

    Document ToJSON(const Reports& data)
    {
        Array result;
        result.reserve(data.reports.size());
        for (const Reports::Report& report : data.reports)
        {
            result.push_back(ToJSON(report));
        }
        return Document(move(Node(move(result))));
    }

    InputFile ReadInputFile(istream& in)
    {
        return ParseInputFile(Load(in));
    }

    void PrintReports(const Reports& data, ostream& out)
    {
        Print(ToJSON(data), out);
    }
}
#include "request_handler.h"

using namespace std;

namespace transport_guide
{
    void BaseRequests::UseRequests(Catalogue& catalogue) const
    {
        for (const Stop& request : stops)
        {
            request.AddStop(catalogue);
        }

        for (const Stop& request : stops)
        {
            request.AddRoads(catalogue);
        }

        for (const Bus& request : buses)
        {
            request.AddBus(catalogue);
        }
    }

    void BaseRequests::Bus::AddBus(Catalogue& catalogue) const
    {
        catalogue.AddBus(name, route, [](const string& str)->string_view { return str; });
    }

    void BaseRequests::Stop::AddStop(Catalogue& catalogue) const
    {
        catalogue.AddStop(name, cords);
    }

    void BaseRequests::Stop::AddRoads(Catalogue& catalogue) const
    {
        const string& stop_name_from = name;
        for (const auto& [stop_name_to, distance] : roads)
        {
            catalogue.AddRoad(StopPair{ stop_name_from, stop_name_to }, distance);
        }
    }

    Reports::Report StatRequests::Request::UseRequest(const Catalogue& catalogue) const
    {
        if (holds_alternative<BusDetails>(details))
        {
            const auto& bus_details = get<BusDetails>(details);
            const RouteInfo* raw_report = catalogue.GetBusInfo(bus_details.name);
            if (raw_report == nullptr)
            {
                return { id, Reports::Error{ not_found_error } };
            }
            return { id, Reports::Bus{ *raw_report } };
        }

        if (holds_alternative<StopDetails>(details))
        {
            const auto& stop_details = get<StopDetails>(details);
            const StopInfo* raw_report = catalogue.GetStopInfo(stop_details.name);
            if (raw_report == nullptr)
            {
                return { id, Reports::Error{ not_found_error } };
            }
            return { id, Reports::Stop{*raw_report} };
        }

        assert(false);
    }

    Reports StatRequests::UseRequests(const Catalogue& catalogue) const
    {
        Reports result;
        auto& reports = result.reports;
        for (const Request& request : requests)
        {
            reports.push_back(request.UseRequest(catalogue));
        }
        return result;
    }

    Reports InputFile::UseInputFile(Catalogue& catalogue) const
    {
        base_requests.UseRequests(catalogue);
        return stat_requests.UseRequests(catalogue);
    }
}
#include "geo.h"

namespace geo
{

    const double earth_radius = 6371000;
    const double pi = 3.1415926535897932;
    bool Coordinates::operator==(const Coordinates& other) const
    {
        return lat == other.lat && lng == other.lng;
    }

    bool Coordinates::operator!=(const Coordinates& other) const
    {
        return !(*this == other);
    }

    double ComputeDistance(Coordinates from, Coordinates to)
    {
        using namespace std;
        if (from == to) { return 0.; }
        static const double dr = pi / 180.;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * earth_radius;
    }
}
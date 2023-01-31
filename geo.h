#pragma once

#include <cmath>
#include <cassert>

namespace geo
{
    struct Coordinates 
    {
    private:
        bool operator==(const Coordinates& other) const;

        bool operator!=(const Coordinates& other) const;

        friend double ComputeDistance(Coordinates from, Coordinates to);
    public:
        double lat; //1rst
        double lng; //2nd
    };

    double ComputeDistance(Coordinates from, Coordinates to);
}
//
// Created by PengFei_Zheng on 07/09/2017.
//

#include "geohash.h"

constexpr long double EARTH_RADIUS = 6371.009;
constexpr long double LONGITUDE_CIRCLE = EARTH_RADIUS*2*M_PI;

inline long double radians(long double d){
    return d * M_PI / 180.0;
}

inline long double degrees(long double d){
    return d * 180 /M_PI;
}

inline long double to180(long double d){
    return d-(std::llround(d)/360*360);
}

inline long double lat_circle(long double d){
    return EARTH_RADIUS*std::cos(radians(d)) * 2 * M_PI;
}

inline long double lng_span(long double lat, long double lng1, long double lng2){
    return radians(std::abs(to180(lng1-lng2)))*lat_circle(lat)/2/M_PI;
}

inline long double lat_span(long double lat1, long double lat2){
    return radians(std::abs(to180(lat1-lat2)))*LONGITUDE_CIRCLE/2/M_PI;
}

double getDistance(const location &posa, const location &posb){
    long double lat1 = radians(posa.lat);
    long double lat2 = radians(posb.lat);
    long double lat_degree = radians(posa.lat - posb.lat);
    long double lng_degree = radians(posa.lng - posb.lng);
    long double a = sin((double)lat_degree/2.0) * sin((double)lat_degree/2.0) +
            cos((double)lat1) * cos((double)lat2) * sin((double)lng_degree /2.0) * sin((double)lng_degree /2.0);
    return (double)EARTH_RADIUS * 2.0 * atan2(sqrt((double)a), sqrt((double)(1.0-a)));
}

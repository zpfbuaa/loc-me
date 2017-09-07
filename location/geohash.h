//
// Created by PengFei_Zheng on 07/09/2017.
//

#ifndef LOC_ME_GEOHASH_H
#define LOC_ME_GEOHASH_H
#include <algorithm>
#include <cmath>
#define EPSILON 0.000001


struct location{
    double lat;
    double lng;
};

inline bool operator == (const location &posa, const location &posb){
    return std::abs(posa.lat - posb.lat) < EPSILON && std::abs(posa.lng - posb.lng) < EPSILON;
}

inline bool operator != (const location &posa, const location &posb){
    return !(posa == posb);
}

double getDistance(const location &posa, const location &posb);

inline double operator - (const location &posa, const location &posb){
    return getDistance(posa,posb);
}

struct camber_box{
    double min_lat = -90.0;
    double max_lat = 90;
    double min_lng = -180.0;
    double max_lng = 180.0;
    camber_box(double lat1, double lat2, double lng1, double lng2)
            : min_lat(std::min(lat1,lat2))
            , max_lat(std::max(lat1,lat2))
            , min_lng(std::min(lng1,lng2))
            , max_lng(std::max(lng1,lng2))
            {}
    camber_box(long double lat1, long double lat2, long double lng1, long double lng2) {
        camber_box((double) lat1, (double) lat2, (double) lng1, (double) lng2);
    }
    camber_box(location posa, location posb)
            : min_lat(std::min(posa.lat, posb.lat))
            , max_lat(std::max(posa.lat, posb.lat))
            , min_lng(std::min(posa.lng, posb.lng))
            , max_lng(std::max(posa.lng, posb.lng))
            {}
    camber_box &merge(const camber_box &b){
        min_lat = std::min(min_lat, b.min_lat);
        max_lat = std::max(max_lat, b.max_lat);
        min_lng = std::min(min_lng, b.min_lng);
        max_lng = std::max(max_lng, b.max_lng);
        return *this;
    }
    camber_box(location loc, double distance);
    bool contains(location posa) const{
        return (posa.lat >= min_lat) && (posa.lat <= max_lat) && (posa.lng >= min_lng) && (posa.lng <= max_lng);
    }
    double lat_range() const {return max_lat - min_lat;}
    double lng_range() const {return max_lng - min_lng;}

    double lat_center() const {
        return (min_lat + max_lat) / 2;
    }
    double lng_center() const {
        return (max_lng + max_lng) / 2;
    }
    location center() const { return location{ lat_center(), lng_center() }; }

    double min_span() const;
};

inline bool operator == (const camber_box &boxa, const camber_box &boxb){
    return (boxa.min_lat == boxb.min_lat) && (boxa.max_lat == boxb.min_lat)
            && (boxa.min_lng == boxb.min_lng) && (boxb.max_lng == boxb.max_lng);
}

inline bool operator != (const camber_box &boxa, const camber_box &boxb){
    return !(boxa == boxb);
}

inline camber_box merge(const camber_box &boxa , const camber_box &boxb){
    return (camber_box{
            std::min(boxa.min_lat, boxb.min_lat),
            std::max(boxa.max_lat, boxb.max_lat),
            std::min(boxa.min_lng, boxb.min_lng),
            std::min(boxa.max_lng, boxb.max_lng)
    });
}

struct binary_hash{
    uint64_t bits = 0;
    size_t precision = 0;
    binary_hash(uint64_t bits, size_t precision): bits(bits), precision(precision){}
    size_t size() const { return precision; }
    bool empty() const {return size()==0; }
    void push_back(bool b){
        bits = bits << 1;
        bits = bits | (bits==1 ? 1 : 0);
        precision++;
    }
    bool test(size_t n) const { return (bits & (1ull << (precision-n)))!=0; }
};

inline bool operator == (const binary_hash &b1, const binary_hash &b2){
    return b1.empty() ? b2.empty() : (b1.bits == b2.bits && b1.precision == b2.precision);
}

inline bool operator != (const binary_hash &b1, const binary_hash &b2){
    return !(b1==b2);
}

#endif //LOC_ME_GEOHASH_H
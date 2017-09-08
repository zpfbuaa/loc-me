//
// Created by PengFei_Zheng on 07/09/2017.
//

#ifndef LOC_ME_GEOHASH_H
#define LOC_ME_GEOHASH_H
#include <algorithm>
#include <cmath>
// the min epsilon
#define EPSILON 0.000001
#define LAT_LOW_BOUND -90.0
#define LAT_UPPER_BOUND 90.0
#define LNG_LOW_BOUND -180.0
#define LNG_UPPER_BOUND 180.0
#define MAX_BITS 5

constexpr size_t MAX_GEOHASH_LENGTH = 12;
constexpr size_t MAX_BINHASH_LENGTH = 64;

// the struction to location things
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

//for next decode or encode
struct camber_box{
    camber_box()= default;
    double min_lat = LAT_LOW_BOUND;
    double max_lat = LAT_UPPER_BOUND;
    double min_lng = LNG_LOW_BOUND;
    double max_lng = LNG_UPPER_BOUND;
    void init(){
        min_lat = LAT_LOW_BOUND;
        max_lat = LAT_UPPER_BOUND;
        min_lng = LNG_LOW_BOUND;
        max_lng = LNG_UPPER_BOUND;
    }
    camber_box(double lat1, double lat2, double lng1, double lng2)
            : min_lat(std::min(lat1,lat2))
            , max_lat(std::max(lat1,lat2))
            , min_lng(std::min(lng1,lng2))
            , max_lng(std::max(lng1,lng2))
            {}
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
        return (min_lng + max_lng) / 2;
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
    binary_hash()=default;
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

size_t binary_hash_precision(location loc, double dist);

binary_hash binary_encode(location loc, size_t precision);

camber_box decode(const binary_hash &bits);

inline binary_hash neighbor(const binary_hash &hash, const std::pair<int, int> &direction)
{
    camber_box box = decode(hash);
    location box_center = box.center();
    box_center.lat += direction.first * box.lat_range();
    box_center.lng += direction.second * box.lng_range();
    return binary_encode(box_center, hash.size());
}

std::string encode(location loc, size_t precision);

camber_box decode(const std::string &hash);

inline std::string neighbor(const std::string &hash, const std::pair<int, int> &direction)
{
    camber_box box = decode(hash);
    location box_center = box.center();
    box_center.lat += direction.first * box.lat_range();
    box_center.lng += direction.second * box.lng_range();
    return encode(box_center, hash.size());
}

inline bool hash_contains(const std::string &hash, location loc) {
    return decode(hash).contains(loc);
}

std::string base_hash(location loc, double dist);

template<typename Container>
void hash_codes(location l, double dist, std::back_insert_iterator<Container> item) {
    std::string hash=base_hash(l, dist);
    *item++=neighbor(hash, {-1, -1});
    *item++=neighbor(hash, {-1,  0});
    *item++=neighbor(hash, {-1,  1});
    *item++=neighbor(hash, { 0, -1});
    *item++=neighbor(hash, { 0,  1});
    *item++=neighbor(hash, { 1, -1});
    *item++=neighbor(hash, { 1,  0});
    *item++=neighbor(hash, { 1,  1});
    *item++=std::move(hash);
}

template<typename Container>
void hash_codes(location l, double dist, Container &item) {
    hash_codes(l, dist, std::back_inserter(item));
}
#endif //LOC_ME_GEOHASH_H
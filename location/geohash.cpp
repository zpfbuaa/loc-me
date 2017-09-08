//
// Created by PengFei_Zheng on 07/09/2017.
//

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <string>
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
camber_box::camber_box(location loc, double distance) {
    long double lat_range = degrees(distance/LONGITUDE_CIRCLE);
    long double max_lat = std::max(std::abs(loc.lat - lat_range), std::abs(loc.lat + lat_range));
    long double lng_range = degrees(distance/lat_circle(max_lat));
    *this = camber_box((double)(loc.lat - lat_range),
                       (double)(loc.lat + lat_range),
                       (double)(loc.lng - lng_range),
                       (double)(loc.lng + lng_range));
}
double camber_box::min_span() const {
    return (double)std::min(lat_span(min_lat, max_lat),
                    std::min(lng_span(min_lat, min_lng, max_lng),
                             lng_span(max_lat, min_lng, max_lng)));
}

static const char base32_codes[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'b', 'c', 'd', 'e', 'f', 'g',
        'h', 'j', 'k', 'm', 'n', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
};

static const int base32_indexes[]={
        0,  1,  2,  3,  4,  5,  6,  7,  // 30-37, '0'..'7'
        8,  9, -1, -1, -1, -1, -1, -1,  // 38-2F, '8','9'
        -1, -1, 10, 11, 12, 13, 14, 15, // 40-47, 'B'..'G'
        16, -1, 17, 18, -1, 19, 20, -1, // 48-4F, 'H','J','K','M','N'
        21, 22, 23, 24, 25, 26, 27, 28, // 50-57, 'P'..'W'
        29, 30, 31, -1, -1, -1, -1, -1, // 58-5F, 'X'..'Z'
        -1, -1, 10, 11, 12, 13, 14, 15, // 60-67, 'b'..'g'
        16, -1, 17, 18, -1, 19, 20, -1, // 68-6F, 'h','j','k','m','n'
        21, 22, 23, 24, 25, 26, 27, 28, // 70-77, 'p'..'w'
        29, 30, 31,                     // 78-7A, 'x'..'z'
};

size_t binary_hash_precision(location loc, double dist){
    for(size_t i = MAX_BINHASH_LENGTH; i>=1; i--){
        camber_box box = decode(binary_encode(loc,i));
        if(box.min_span() >= dist*2){
            return i;
        }
    }
    return 0;
}

binary_hash binary_encode(location loc, size_t precision){
    camber_box box;
    box.init();
    bool is_lng = true;
    binary_hash output;

    while(output.size() < precision){
        if(is_lng){
            if(loc.lat > box.lng_center()){
                output.push_back(true);
                box.min_lng = box.lng_center();
            } else{
                output.push_back(false);
                box.max_lat = box.lng_center();
            }
        } else{
            if(loc.lat > box.lat_center()){
                output.push_back(true);
                box.min_lat = box.lat_center();
            } else{
                output.push_back(false);
                box.max_lat = box.lat_center();
            }
        }
        is_lng = !is_lng;
    }
    return output;
}

camber_box decode(const binary_hash &hash){
    camber_box output;
    output.init();
    bool is_lng = true;
    size_t size = hash.size();
    for(size_t i = 1; i <= size; i++){
        bool has_bit = hash.test(i);
        if(is_lng){
            if(has_bit){
                output.min_lng = output.lng_center();
            } else{
                output.max_lng = output.lng_center();
            }
        } else{
            if(has_bit){
                output.min_lat = output.lat_center();
            } else{
                output.max_lat = output.lat_center();
            }
        }
        is_lng = !is_lng;
    }
    return output;
}

std::string encode(location loc, size_t precision){
    camber_box box;
    box.init();
    bool is_lng = true;
    int num_bits = 0;
    int hash_index = 0;
    std::string output(precision, ' ');
    size_t output_length = 0;

    while(output_length < precision){
        if(is_lng){
            if(loc.lng > box.lng_center()){
                hash_index = (hash_index << 1) + 1;
                box.min_lng = box.lng_center();
            } else{
                hash_index = (hash_index << 1) + 0;
                box.max_lng = box.lng_center();
            }
        } else{
            if(loc.lat > box.lat_center()){
                hash_index = (hash_index << 1) + 1;
                box.min_lat = box.lat_center();
            } else{
                hash_index = (hash_index << 1) + 0;
                box.max_lat = box.lat_center();
            }
        }
        is_lng = !is_lng;
        num_bits++;
        if(MAX_BITS == num_bits){
            output[output_length++] = base32_codes[hash_index];
            num_bits = 0;
            hash_index = 0;
        }
    }
    return output;
}

camber_box decode(const std::string &hash) {
    camber_box output;
    output.init();
    bool is_lng = true;

    for(auto &c : hash) {
        if (c<'0' || c>'z') {
            throw std::invalid_argument("Invalid geohash");
        }
        int char_index = base32_indexes[c-48];
        if (char_index < 0) {
            throw std::invalid_argument("Invalid geohash");
        }

        for (int bits = 4; bits >= 0; bits--) {
            int bit = (char_index >> bits) & 1;
            if (is_lng) {
                if(bit == 1) {
                    output.min_lng = output.lng_center();
                } else {
                    output.max_lng = output.lng_center();
                }
            } else {
                if(bit == 1) {
                    output.min_lat = output.lat_center();
                } else {
                    output.max_lat = output.lat_center();
                }
            }
            is_lng = !is_lng;
        }
    }
    return output;
}

std::string base_hash(location loc, double dist){
    for(size_t i = MAX_GEOHASH_LENGTH; i >= 1; i--){
        std::string hash = encode(loc,i);
        if(decode(hash).min_span() >= dist*2) {
            return hash;
        }
    }
    return "";
}
#include <iostream>
#include <vector>
#include <map>
#include "location/geohash.h"

#define BASE_MIN 0.02
#define NEARBY_MIN 0.005

using namespace std;

int main() {
    cout << "Hello, World!" << endl;
    location latlng{39.9793796465,116.3397989834};

    string loc_me = base_hash(latlng, BASE_MIN);

    std::vector<std::string> geo_list;
    map<string, vector<string>> geo_map;

    geo_list.push_back(loc_me);
    vector<string> neighbor_geo_list;
    if(geo_map.find(loc_me) == geo_map.end()){
        hash_codes(latlng, NEARBY_MIN, neighbor_geo_list);
        geo_map.insert(map<string, vector<string>>::value_type(loc_me, neighbor_geo_list));
    }
    double min_lat = 90.0;
    double max_lat = -90.0;
    double min_lng = 180.0;
    double max_lng = -180.0;
    for(auto c : neighbor_geo_list){
        cout<<"box_codes: "<<c<<endl;
        camber_box box;
        box = decode(c);
        min_lat = min(min_lat, box.min_lat);
        max_lat = max(max_lat, box.max_lat);
        min_lng = min(min_lng, box.min_lng);
        max_lng = max(max_lng, box.max_lng);
        cout<<box.min_lat<<","<<box.min_lng<<endl;
        cout<<box.min_lat<<","<<box.max_lng<<endl;
        cout<<box.max_lat<<","<<box.min_lng<<endl;
        cout<<box.max_lat<<","<<box.max_lng<<endl;
        cout<<"************************************************************\n";
    }
    cout<< "box_bounder: "<<endl;
    cout<<min_lat<<","<<min_lng<<endl;
    cout<<min_lat<<","<<max_lng<<endl;
    cout<<max_lat<<","<<min_lng<<endl;
    cout<<max_lat<<","<<max_lng<<endl;

    return 0;
}
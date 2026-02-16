#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <tuple>
#include <flash_search.h>

struct SearchData {
    std::vector<std::string> v;
    std::set<std::string> s;
    std::map<std::string, int> m;
    std::unordered_map<std::string, int> um;
    std::unordered_set<std::string> us;
};

SearchData get_data(std::string_view file, FlashSearch& search);
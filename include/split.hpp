#pragma once

#include<vector>
#include<string>

std::vector<std::string> split(const std::string& in, char delim) {
    std::vector<std::string> V;
    std::size_t start = 0, end;

    while ((end = in.find(delim, start)) != std::string::npos) {
        V.push_back(in.substr(start, end - start));
        start = end + 1;
    }
    V.push_back(in.substr(start));

    return V;
}
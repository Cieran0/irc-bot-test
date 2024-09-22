#include <util.hpp>
#include <algorithm>

std::vector<std::string> split_string(const std::string& input, const std::string& split_by, bool include_last) {
    std::string buffer;
    std::vector<std::string> split_strings;

    size_t start = 0;
    size_t end = input.find(split_by);

    while (end != std::string::npos) {
        buffer = input.substr(start, end - start);
        split_strings.push_back(buffer);
        start = end + split_by.length();
        end = input.find(split_by, start);
    }
    if(include_last) {
        buffer = input.substr(start, end);
        split_strings.push_back(buffer);
    }
    return split_strings;
}

bool contains(const std::vector<std::string>& vector, const std::string& element) {
    return std::find(vector.begin(), vector.end(), element) != vector.end();
}
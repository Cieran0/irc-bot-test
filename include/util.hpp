#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iostream>


std::vector<std::string> split_string(const std::string& input, const std::string& split_by, bool include_last);
bool contains(const std::vector<std::string>& vector, const std::string& element);
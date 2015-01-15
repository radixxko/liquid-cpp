#ifndef __liquid_cpp__string__string__
#define __liquid_cpp__string__string__

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

namespace liquid
{
	namespace string
    {
		std::string replace(std::string subject, const std::string& search, const std::string& replace);
		std::vector<std::string> split(const std::string& s, const char c);
		std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty = false);
		std::pair<std::string, std::string> split2pair(const std::string& s, const char c);
		std::string remove(std::string s, const std::string chars);
	}
}

#endif
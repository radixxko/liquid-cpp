#include <liquid-cpp/string/string.hpp>

namespace liquid
{
	namespace string
    {
		std::string replace(std::string subject, const std::string& search, const std::string& replace)
		{
			size_t pos = 0;
			while ((pos = subject.find(search, pos)) != std::string::npos)
			{
				subject.replace(pos, search.length(), replace);
				pos += replace.length();
			}
			return subject;
		}

		std::vector<std::string> split(const std::string& s, const char c){
			std::string line;
			std::vector<std::string> lines;
			std::stringstream ss(s);

			while (std::getline(ss, line, c)){
				lines.push_back(line);
			}
			return lines;
		}

		std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty/* = false*/) {
			std::vector<std::string> result;
			if (delim.empty()) {
				result.push_back(s);
				return result;
			}

			std::string::const_iterator substart = s.begin(), subend;
			for (;;) {
				subend = search(substart, s.end(), delim.begin(), delim.end());
				std::string temp(substart, subend);
				if (keep_empty || !temp.empty()) {
					result.push_back(temp);
				}
				if (subend == s.end()) {
					break;
				}
				substart = subend + delim.size();
			}
			return result;
		}

		std::pair<std::string, std::string> split2pair(const std::string& s, const char c){
			std::stringstream ss = std::stringstream(s);
			std::string key, value;

			if (std::getline(ss, key, c)){
				if (std::getline(ss, value)){
					return std::pair<std::string, std::string>(key, value);
				}
			}

			return std::pair<std::string, std::string>("", "");
		}

		std::string remove(std::string s, const std::string chars){
			unsigned int i = 0;

			for (; i < chars.length(); i++){
				s.erase(std::remove(s.begin(), s.end(), chars[i]), s.end());
			}
			return s;
		}
	}
}
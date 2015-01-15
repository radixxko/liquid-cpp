#include "./http_header.hpp"

#include <liquid-cpp/string/string.hpp>
#include <liquid-cpp/http/http_header.hpp>

namespace liquid
{
	namespace http
    {
		http_header::http_header(){}

		http_header::http_header(const std::string& sreq)
		{
			clear();
			parse(sreq);
		}

		http_header::~http_header(){}

		void http_header::clear()
		{
			request_line.clear();
			fields.clear();
		}

		void http_header::parse(const std::string& sreq)
		{
			std::vector<std::string> lines = string::split(sreq, '\n');

			if (lines.size() == 0){ return; }

			request_line = lines[0];

			std::pair<std::string, std::string> pair;
			std::vector<std::string>::iterator it = lines.begin();
			++it;
			for (; it != lines.end(); ++it)
            {
				*it = string::remove(*it, " \r\n");
				pair = string::split2pair(*it, ':');
				fields.insert(pair);
			}
		}

		std::string http_header::str()
		{
			std::ostringstream oss;
			std::unordered_map<std::string, std::string>::iterator it = fields.begin();

			oss << request_line << "\r\n";
			for (; it != fields.end(); ++it){
				oss << it->first << ": " << it->second << "\r\n";
			}
			oss << "\r\n";

			return oss.str();
		}

		std::string& http_header::operator[](std::string s)
		{
			return fields[s];
		}
	}
}

#ifndef __liquid_cpp__http__http_header__
#define __liquid_cpp__http__http_header__

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace liquid
{
	namespace http
	{
		const size_t max_header_size = 16000;

		class http_header
		{
		private:
			std::string m_uri;

		public:
			http_header();
			http_header(const std::string& sreq);
			virtual ~http_header();

			void add(std::string name, std::string val);
			void clear();
			void parse(const std::string& sreq);

			inline std::string get_uri(){ return m_uri; }

			std::string str();
			std::string& operator[](std::string s);

			// public members
			std::unordered_map<std::string, std::string> fields;
			std::string request_line;
		};
	}
}

#endif
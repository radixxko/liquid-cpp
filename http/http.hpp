#ifndef __liquid_cpp_http_http__
#define __liquid_cpp_http_http__

#include <liquid-cpp/core/defines.hpp>

#include <liquid-cpp/string/const_string.hpp>
#include <liquid-cpp/json/json.hpp>

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

// rev.2 
#include <regex>

namespace liquid
{
    enum http_request_type { GET, POST };
    
    class http;
    class http_session_data;
    
    class http_session
    {
      public:
        
        json get_cookies();
        
        http_session( http_session const & session );
        http_session & operator=( http_session const & session );
        
        ~http_session();
        
      private:
        
        http_session( http_session_data * data ):m_data(data){};
        http_session_data * operator->(){ return m_data; };
        
      private:
        
        http_session_data * m_data;
        // TODO dorobit copy konstruktory a detruktor, reference counting
        
      friend class http;
    };
    
    class http_request_parameter
    {
      public:
        
        http_request_parameter();
        http_request_parameter( const_string name, const_string value );
        http_request_parameter & operator()( const_string name, const_string value );
        
      private:
        
        std::string m_querystring;
        
      friend class http_session_data;
    };

    class http
    {
      public:

		static void get( const_string url, std::function<void(http_session, std::string)> callback);
		static void get( const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback);
		static void get( http_session session, const_string url, std::function<void(http_session, std::string)> callback);
		static void get( http_session session, const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback);
        
        static void post( const_string, std::function<void(http_session, std::string)> callback);
		static void post( const_string, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback);
		static void post( http_session session, const_string url, std::function<void(http_session, std::string)> callback);
		static void post( http_session session, const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback);
        
        static void request( http_request_type type, const_string url, std::function<void(http_session, std::string)> callback);
		static void request( http_request_type type, const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback);
		static void request( http_request_type type, http_session session, const_string url, std::function<void(http_session, std::string)> callback);
		static void request( http_request_type type, http_session session, const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback);
        
        static std::string get_url( const_string url, const_string base_url );
        static std::string get_domain( const_string url, bool protocol = false );
        static std::string get_path( const_string url );
        static std::string url_encode( const_string text );

        static std::regex REGEX_DOMAIN_AND_PATH;
    
      private:

    };
}

#endif

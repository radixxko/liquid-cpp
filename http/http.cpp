#include <liquid-cpp/http/http.hpp>

#include <liquid-cpp/core/includes/net.hpp>
#include <liquid-cpp/json/json.hpp>

#include <iostream>
#include <unordered_map>



#include <sys/types.h>

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>

// rev.2
#include <regex>
#include <iomanip>
#include <ctime>
#include <map>
#include <queue>
#include <algorithm>
#include <atomic>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace liquid
{
    class http_cookie
    {
      private:
        
        http_cookie( std::string url, std::string cookie );
        
        std::string str();
        json get();
        
        static std::regex REGEX_COOKIE_NAME_VALUE;
        static std::regex REGEX_COOKIE_DOMAIN;
        static std::regex REGEX_COOKIE_PATH;
        static std::regex REGEX_COOKIE_HTTPONLY;
        static std::regex REGEX_COOKIE_SECURE;
        static std::regex REGEX_COOKIE_EXPIRES;
        
        std::string url;
        std::string name;
        std::string value;
        std::string domain;
        std::string path;
        bool secure;
        bool http_only;
        bool host_only;
        size_t expires;
    
      friend class http_cookie_jar;
      friend class http_cookie_comparator;
    };
    
    class http_cookie_comparator
    {
      public:
        
        bool operator()( const http_cookie & a, const http_cookie & b ) const
        {
            return ( a.name != b.name ) ? ( a.name < b.name ) : ( ( a.path.length() != b.path.length() ) ? ( a.path.length() > b.path.length() ) : ( a.domain.length() > b.domain.length() ) );
        }
    }
    http_cookie_comparator;
    
    class http_cookie_jar
    {
      public:
        
        void add( const char * url, const char * cookie_str );
        std::string cookie_header( const char * url );
        json get_cookies();
        
      private:
        
        std::map< std::string, std::vector<http_cookie>> m_cookies;
        
      friend class http_cookie;
    };
    
    std::regex http_cookie::REGEX_COOKIE_NAME_VALUE = std::regex("^([^=]*)=([^;^\r]*)", std::regex::icase);
    std::regex http_cookie::REGEX_COOKIE_DOMAIN = std::regex("domain=[.]?([^;^\r]*)", std::regex::icase);
    std::regex http_cookie::REGEX_COOKIE_PATH = std::regex("path=([^;^\r]*)?", std::regex::icase);
    std::regex http_cookie::REGEX_COOKIE_HTTPONLY = std::regex("httponly", std::regex::icase);
    std::regex http_cookie::REGEX_COOKIE_SECURE = std::regex("secure", std::regex::icase);
    std::regex http_cookie::REGEX_COOKIE_EXPIRES = std::regex("expires=([^;^\r]*)", std::regex::icase);

    std::string http_cookie::str()
    {
        std::stringstream ss;
        ss << name << "=" << value;
        
        return ss.str();
    }
    
    json http_cookie::get()
    {
        json cookie;
        std::string cookiestring;
        
        cookiestring += name;
        cookiestring += "=";
        cookiestring += value;
        
        if( domain != "" )
        {
            cookiestring += "; Domain=" + domain;
        }
        
        if( path != "" )
        {
            cookiestring += "; Path=" + path;
        }
        
        if( secure )
        {
            cookiestring += "; Secure";
        }
        
        if( http_only )
        {
            cookiestring += "; HttpOnly";
        }
        
        cookie["url"] = url;
        cookie["cookiestring"] = cookiestring;
        
        return cookie;
    }
       
    http_cookie::http_cookie( std::string url, std::string cookie ): url(url)
    {
        std::smatch matches;

        // name / value
        if( std::regex_search(cookie, matches, REGEX_COOKIE_NAME_VALUE) )
        {
                name = matches[1];
                value = matches[2];
        }

        // domain
        if( std::regex_search(cookie, matches, REGEX_COOKIE_DOMAIN) )
            domain = matches[1];
        else
            domain = http::get_domain(url.c_str());

        // path
        if( std::regex_search(cookie, matches, REGEX_COOKIE_PATH) )
            path = matches[1];
        else
            path = http::get_path(url.c_str());

        // http_only
        if( std::regex_search(cookie, matches, REGEX_COOKIE_HTTPONLY) )
            http_only = true;
        else
            http_only = false;

        // secure
        if( std::regex_search(cookie, matches, REGEX_COOKIE_SECURE) )
            secure = true;
        else
            secure = false;
        
        // expires
        if( std::regex_search(cookie, matches, REGEX_COOKIE_EXPIRES) )
        {
            // musia pankovia do gcc dokodit (gcc 4.9 to este nema)
            //
            //std::tm t;
            //std::istringstream ss( matches[1] );
            //ss >> std::get_time(&t, "%a, %d %b %Y %H:%M:%S GMT");
            //std::cout << "DATUM = " << matches[1] << " sparsovane a spat = " << std::put_time(&t, "%c") << std::endl;
        }
        
    }

    void http_cookie_jar::add( const char * url, const char *cookie_str )
    {
        size_t position = 0;
        while( cookie_str[position] != 0 && cookie_str[position] != '\r' && cookie_str[position] != '\n' ){ ++position; }

        std::string cookie_string(cookie_str, position);
        
        //std::cout << "KUKIE " << cookie_string << std::endl << std::endl;
        
        // parse cookie parameters
        http_cookie cookie( url, cookie_string);

        // cookie not stored yet
        if( m_cookies.find(cookie.name) == m_cookies.end() )
        {
            // store new cookie into jar
            m_cookies[cookie.name] = std::vector<http_cookie>();
            m_cookies[cookie.name].push_back( cookie );
            
            //std::cout << "NEW COOKIE STORED: " << cookie.name << "=" << cookie.value << std::endl;
            //std::cout << cookie_string << std::endl;
        }
        // cookie with same name already exists
        else
        {
            bool is_new = true;

            // try to find exact same cookie which already have been stored
            for( auto c = m_cookies[cookie.name].begin(); c != m_cookies[cookie.name].end(); ++c )
            {
                if( c->domain == cookie.domain && c->path == cookie.path )
                {
                    c->value = cookie.value;
                    c->http_only = cookie.http_only;
                    c->secure = cookie.secure;

                    is_new = false;
                    
                    //std::cout << "COOKIE UPDATED: " << cookie.name << "=" << cookie.value << " " << c->path << " = " << cookie.path << std::endl;
                }
            }

            if( is_new )
            {
                m_cookies[cookie.name].push_back( cookie );
                std::sort( m_cookies[cookie.name].begin(), m_cookies[cookie.name].end(), http_cookie_comparator) ;
                //std::cout << "NEW COOKIE STORED: " << cookie.name << "=" << cookie.value << std::endl;
            }
        }
        
        //std::cout << std::endl;
    }

    std::string http_cookie_jar::cookie_header( const char * url )
    {
        std::string domain = http::get_domain(url);
        std::string path = http::get_path(url);
        
        if( !m_cookies.empty() )
        {
            std::map< std::string, std::vector<http_cookie> > cookies;

            for( auto i = m_cookies.begin(); i != m_cookies.end(); ++i )
            {
                auto cookies_vector = i->second;
                
                //std::cout << "FILTERING" << std::endl;
                
                // filtering
                for( auto cookie = cookies_vector.begin(); cookie != cookies_vector.end(); ++cookie )
                {
                    //std::cout << cookie->domain << " " << cookie->name << std::endl;
                    
                    //std::cout << domain << std::endl;
                    
                    // - ak je cookie domena dlhsia ako domena jar tak ju vyluc
                    if( cookie->domain.length() > domain.length() ) { /*std::cout << "= dluhsa" << std::endl;*/ }
                    // - ak je cookie path dlhsi ako jar path tak ho vyluc
                    else if( cookie->path.length() > path.length() ) { /*std::cout << "= path dluhsi" << std::endl;*/ }
                    
                    // - z cookie domeny vytvor regular a ak nematchne voci jar url domene tak vyluc
                    else if( ! std::regex_match(domain, std::regex("(^|.*[.])"+ cookie->domain +"$", std::regex::icase)) ) {}
                    // - z cookie pathu vytvor regular a ak nematchne voci jar url pathu tak vyluc
                    else if( ! std::regex_match(path, std::regex("^"+ cookie->path +".*$", std::regex::icase)) ) {}
                    
                    // - inac cookie odpamataj
                    else
                        cookies[ i->first ].push_back( *cookie );
                }


                // preparing
                // - vsetky ktore zostali zorad podla najdlhsej domeny a pathe
                if( cookies[ i->first ].size() >= 2 )
                {
                   std::sort( cookies[i->first].begin(), cookies[i->first].end(), 
                      [](const http_cookie & a, const http_cookie & b) -> bool
                      {
                          return 
                            ( a.domain.length() > b.domain.length() ||
                            ( a.domain.length() == a.domain.length() && a.path.length() >= b.path.length() ) );
                      }
                   );
                }
            }
            
            if( cookies.size() > 0 )
            {
                std::string cookie_header = "Cookie: ";
                size_t i = 0;

                // generating
                for( auto cookie = cookies.begin(); cookie != cookies.end(); ++cookie )
                {
                    i++;
                    
                    if( cookie->second.size() > 0 )
                    {
                        cookie_header += cookie->second[0].str();
                        
                        if( i != cookies.size() ){ cookie_header += "; "; }
                    }
                }
                
                cookie_header += "\r\n";

                return cookie_header;
            }
        }

        return "";
    }
    
    json http_cookie_jar::get_cookies()
    {
        json cookies;
        
        for( auto i = m_cookies.begin(); i != m_cookies.end(); ++i )
        {
            auto cookies_vector = i->second;
            
            for( auto cookie = cookies_vector.begin(); cookie != cookies_vector.end(); ++cookie )
            {
                cookies.push(cookie->get());
            }
        }
        
        return cookies;
    }
    
    struct http_addr
    {
        ///http_addr():family{};
        
        int family;
        int socktype;
        int protocol;
        socklen_t addrlen;
        sockaddr addr;
    };

    class http_session_data
    {
      public:

        http_session_data(): m_socket(INVALID_SOCKET), m_response_data(NULL), m_ssl(false), m_ssl_handle(NULL), m_ssl_context(NULL), m_references_cnt(1){};
        ~http_session_data()
        {
            if( m_socket )
			{
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
			}

            if( m_response_data != NULL ){ free( m_response_data ); }
            
            // TODO: crashuje na eset.com
            //if( m_ssl_handle != NULL ){ SSL_free( m_ssl_handle ); } //TODO poladit
            //if( m_ssl_context != NULL ){ SSL_CTX_free( m_ssl_context ); } //TODO poladit
        };

		std::string request(http_request_type type, const_string url, const http_request_parameter & parameters);
        json get_cookies();
        
        std::atomic_size_t m_references_cnt;

      private:

        bool connect();
        std::string parseHeader(); // redirectURL

        ssize_t read( void *buf, size_t count );
        ssize_t write( const void *buf, size_t count );
        
        static hostent * gethostbyname( const char * name );

      private:

        struct sockaddr_in m_address;
        hostent * m_hostent;

        std::string m_domain;
        std::string m_url;
        SOCKET m_socket;
        http_cookie_jar m_cookie_jar;
        
        std::string m_last_url;

        bool m_ssl;
        SSL * m_ssl_handle;
        SSL_CTX * m_ssl_context;

        char * m_response_header;
        char * m_response_body;
        char * m_response_data;
        size_t m_response_data_length;
        size_t m_response_data_size;
        
        static std::map<std::string, http_addr> s_hostnames;
        static std::mutex s_hostnames_mutex;
    };
    
    std::map<std::string, http_addr> http_session_data::s_hostnames;
    std::mutex http_session_data::s_hostnames_mutex;

    inline ssize_t http_session_data::read( void *buf, size_t count )
    {
        return ( m_ssl ) ? SSL_read( m_ssl_handle, buf, (int)count ) : ::recv( m_socket, (char*)buf, (int)count, 0 );
    }

    inline ssize_t http_session_data::write( const void *buf, size_t count )
    {
        return ( m_ssl ) ? SSL_write( m_ssl_handle, buf, (int)count ) : ::send( m_socket, (char*)buf, (int)count, 0 );
    }

    bool http_session_data::connect()
    {
        if( m_socket != INVALID_SOCKET )
        {
            // TODO: socket reuse
            
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
            
            if( m_ssl )
            {
                if( m_ssl_handle )
                {
                    SSL_shutdown( m_ssl_handle );
                    SSL_free( m_ssl_handle );
                }
                if( m_ssl_context )
                {
                    SSL_CTX_free( m_ssl_context );
                }
            }
        }
        
        if( m_socket == INVALID_SOCKET )
        {
            std::unique_lock<std::mutex> lock(s_hostnames_mutex);
            
            std::map<std::string, http_addr>::const_iterator host = http_session_data::s_hostnames.find((( m_ssl ? "https" : "http" ) + m_domain).c_str());
            
            if( host != http_session_data::s_hostnames.end() )
            {
                http_addr address = host->second;
                
                lock.unlock();
                
                m_socket = socket( address.family, address.socktype, address.protocol );
                
                if( m_socket != INVALID_SOCKET )
                {
                    if( ::connect(m_socket, &address.addr, address.addrlen) == -1 )
                    {
						closesocket(m_socket);
						m_socket = INVALID_SOCKET;
                    }
                }
            }
        }
        
        if( m_socket == INVALID_SOCKET )
        {
            struct addrinfo hints;
            struct addrinfo *addresses, *address;
            
            memset(&hints, 0, sizeof(struct addrinfo));
            hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
            hints.ai_socktype = SOCK_STREAM; /* TCP socket */
            hints.ai_flags = 0;
            hints.ai_protocol = 0;          /* Any protocol */
            
            if( getaddrinfo(m_domain.c_str(), ( m_ssl ? "https" : "http" ), &hints, &addresses) == 0 )
            {
                for( address = addresses; address != NULL; address = address->ai_next )
                {
                    m_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
                    
                    if( m_socket != INVALID_SOCKET )
                    {
                        if( ::connect(m_socket, address->ai_addr, address->ai_addrlen) == 0 )
                        {
                            http_addr addr;
                            addr.family = address->ai_family;
                            addr.socktype = address->ai_socktype;
                            addr.protocol = address->ai_protocol;
                            addr.addrlen = address->ai_addrlen;
                            addr.addr = *address->ai_addr;
                            
                            std::unique_lock<std::mutex> lock(s_hostnames_mutex);
                            
							http_session_data::s_hostnames[(( m_ssl ? "https" : "http" ) + m_domain).c_str()] = addr;
                            
                            break;
                        }
                        else
                        {
							closesocket(m_socket);
							m_socket = INVALID_SOCKET;
                        }
                    }
                }
            }
        }

        if( m_socket != INVALID_SOCKET )
        {
            if( m_ssl )
            {
                SSL_load_error_strings();
                SSL_library_init();

                m_ssl_context = SSL_CTX_new(SSLv23_client_method());
                if( m_ssl_context == NULL ){ ERR_print_errors_fp(stderr); return false; }

                SSL_CTX_set_verify(m_ssl_context, SSL_VERIFY_NONE, NULL);
                
                m_ssl_handle = SSL_new( m_ssl_context );
                if( m_ssl_handle == NULL ){ ERR_print_errors_fp(stderr); return false; }

                if( !SSL_set_fd(m_ssl_handle, (int)m_socket) ){ ERR_print_errors_fp(stderr); return false; }

                if( SSL_connect(m_ssl_handle) != 1 ){ ERR_print_errors_fp(stderr); return false; }
            }

            return true;
        }

        return false;
    }

    // TODO premenit na const_string
    std::string http_session_data::request(http_request_type type, const_string url, const http_request_parameter & parameters)
    {
        //std::cout << "Data " << parameters.m_querystring << std::endl;
        
        if( type == POST )
        {
            //std::cout << "URL " << *url << std::endl;
        }
        
        m_domain = http::get_domain(url);
        m_url = *url;
        m_ssl = ( strncmp( m_url.c_str(), "https://", strlen("https://")) == 0 );
        
        std::string resource(m_url.c_str() + m_domain.length() + strlen("http://") + ( m_ssl ? 1 : 0 ) );
        if( resource == "" ){ resource = "/"; }
        
        std::string querystring = ( parameters.m_querystring != "" ) ? ( "?" + parameters.m_querystring ) : "";

        if( connect() )
        {
            std::string request;
            std::string referer;
            if( m_last_url != "" )
            {
                referer.append("Referer: ").append(m_last_url).append("\r\n");
            }
            
            
            // TOTO poprososim nechat tak
            if( type == GET || querystring.length() == 0 )
            {
                request .append("GET ").append(resource).append(querystring).append(" HTTP/1.1\r\n")
                        //.append("User-Agent: Mozilla/5.0 (Mozilla/5.0 (Macintosh; Intel Mac OS X 10.10; rv:34.0) Gecko/20100101 Firefox/34.0\r\n")
                        .append("User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.95 Safari/537.36\r\n")
                        .append("Host: ").append(m_domain).append("\r\n")
                        .append(referer)
                        .append(m_cookie_jar.cookie_header(*url))
                        //.append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n")
                        .append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n")
                        //.append("Accept-Language: sk-SK,sk;q=0.9,en;q=0.8\r\n")
                        .append("Accept-Language: sk,cs;q=0.8,en-US;q=0.5,en;q=0.3\r\n")
                        .append("Accept-Charset: UTF-8\r\n")
                        //.append("Accept-Encoding: gzip, deflate, sdch\r\n")
                        .append("Connection: close\r\n")
                        .append("\r\n");
            }
            else // POST
            {
                char querystring_length[20];
                
                if( querystring.length() > 0 )
                {
                    querystring.erase(0,1); // remove '?' character
                }

#ifdef _WIN32
				sprintf_s(querystring_length,"%ld",querystring.length());
#else
				sprintf(querystring_length, "%ld", querystring.length());
#endif // _WIN32
                
                //std::cout << m_cookie_jar.cookie_header(*url).c_str() << std::endl;
                
                
                request .append("POST ").append(resource.c_str()).append(" HTTP/1.1\r\n")
                        .append("User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:30.0) Gecko/20100101 Firefox/30.0\r\n")
                        .append("Host: ").append(m_domain.c_str()).append("\r\n")
                        .append(referer)
                        .append(m_cookie_jar.cookie_header(*url).c_str())
                        .append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n")
                        //.append("Accept-Language: sk-SK,sk;q=0.9,en;q=0.8\r\n")
                        .append("Accept-Language: sk,cs;q=0.8,en-US;q=0.5,en;q=0.3\r\n")
                        .append("Accept-Charset: UTF-8\r\n")
                        //.append("Accept-Encoding: gzip, deflate\r\n")
                        .append("Content-Type: application/x-www-form-urlencoded\r\n")
                        .append("Content-Length: ").append(querystring_length).append("\r\n")
                        .append("Connection: close\r\n")
                        .append("\r\n")
                        .append(querystring);
            }
            
            if( strstr(m_domain.c_str(), "t-mobile") )
            {
                std::cout << "picacinka" << std::endl;
            }
            
            /*if( m_domain == "www.tumblr.com" )
            {
                std::ofstream path_log;
                path_log.open("/Users/alex/Desktop/log/request.txt");
                path_log << request;
                path_log.close();
            }*/
            
            m_last_url = m_url; // Save for refering
            
            //std::cout << std::endl << request << std::endl << std::endl;

            write(request.c_str(),request.length());

            size_t response_part_length = 0;
            m_response_data_length = 0;

            if( m_response_data == NULL )
            {
                m_response_data_size = 262144;
                m_response_data = (char *) malloc(m_response_data_size * sizeof(char));
            }

            while( ( response_part_length = read(m_response_data + m_response_data_length, m_response_data_size - m_response_data_length) ) > 0 )
            {
                if( m_response_data_size == ( m_response_data_length += response_part_length ) )
                {
					m_response_data_size = (size_t)(m_response_data_size * 1.3);
                    m_response_data = (char *) realloc (m_response_data, m_response_data_size * sizeof(char));
                }
            }

            m_response_data[m_response_data_length] = 0;

            std::string redirect = parseHeader();
            
            if( strstr(m_domain.c_str(), "t-mobile") )
            {
                std::ofstream path_log;
                path_log.open("/Users/alex/Desktop/log/final_response.html");
                path_log << m_response_body;
                path_log.close();
            }

            if( redirect != "" )
            {
                return http_session_data::request(type, redirect.c_str(), (type == http_request_type::GET ? parameters : http_request_parameter()));// redirect nezachovava querystring
            }
        }
		else
		{ 
			//std::cout << "Cant connect " << m_url << std::endl; 
			return ""; 
		}

        return ( m_response_body ) ? m_response_body: "" ;
    }

    std::string http_session_data::parseHeader()
    {
        size_t position = 0;
        size_t header_start;
        std::string redirect = "";

		m_response_header = nullptr;
		m_response_body = nullptr;

        while( position < m_response_data_length - 4 )
        {
            header_start = position;

            while( position < m_response_data_length && m_response_data[position] != '\r' ){ ++position; }

            if( position < m_response_data_length - 1 && m_response_data[position++] == '\r' && m_response_data[position++] == '\n' )
            {
                if( strncasecmp( m_response_data + header_start, "set-cookie: ", strlen("set-cookie: ") ) == 0 )
                {
                    m_cookie_jar.add( m_url.c_str(), m_response_data + header_start + strlen("set-cookie: "));
                }
                else if( strncasecmp( m_response_data + header_start, "location: ", strlen("location: ") ) == 0 )
                {
                    redirect = std::string(m_response_data + header_start + strlen("location: "), position - header_start - strlen("location: ") - 2);
                }
            }
            else{ break;/* ERROR */ }

            // Header end
            if( position < m_response_data_length - 1 && m_response_data[position] == '\r' && m_response_data[position+1] == '\n' )
            {
                m_response_header = m_response_data;
                m_response_body = m_response_data + position + 2;

                break;
            }
        }

        //std::cout << std::string(m_response_header, m_response_body - m_response_header);
        
        if( redirect != "" && strncmp("http", redirect.c_str(), 4) != 0 )
        {
            redirect = "http://" + http::get_domain(m_url.c_str()) + redirect;
        }

        //std::cout << "Redirect: " << redirect << std::endl;
        
        return redirect;
    }
    
    json http_session_data::get_cookies()
    {
        return m_cookie_jar.get_cookies();
    }
    
    json http_session::get_cookies()
    {
        return m_data->get_cookies();
    }
    
    http_session::http_session( http_session const & session ): m_data(session.m_data)
    {
        ++m_data->m_references_cnt;
    }
    
    http_session & http_session::operator=( http_session const & session )
    {
        http_session_data * const old_data = m_data;
        
        m_data = session.m_data;
        ++m_data->m_references_cnt;
        
        if( --old_data->m_references_cnt == 0 ){ delete old_data; }
        
        return *this;
    }
    
    http_session::~http_session()
    {
        if( --m_data->m_references_cnt == 0 )
        {
            delete m_data;
        }
    }

    http_request_parameter::http_request_parameter():m_querystring(){}
    
    http_request_parameter::http_request_parameter( const_string name, const_string value )
    {
        m_querystring  = *name;
        m_querystring += "=";
        m_querystring += http::url_encode(*value);
    }
    
    http_request_parameter & http_request_parameter::operator()( const_string name, const_string value )
    {
        if( m_querystring.length() > 0 ){ m_querystring += "&"; }
        
        m_querystring += *name;
        m_querystring += "=";
        m_querystring += http::url_encode(*value);
        
        return *this;
    }
    
    http_request_parameter::http_request_parameter( std::unordered_map<std::string, std::string> values ):m_querystring()
    {
        for( auto value = values.begin(); value != values.end(); ++value )
        {
            this->operator()(value->first.c_str(), value->second.c_str());
        }
    }

/*
	std::regex http::REGEX_DOMAIN_AND_PATH = std::regex("http[s]*://([^/\?]*)(.*)", std::regex::icase);

	std::string http::get_path(const char * url)
	{
		std::smatch matches;
		std::string url_str(url);

		if (std::regex_match(url_str, matches, REGEX_DOMAIN_AND_PATH))
			return matches[2];
		else
			return "/"; // TODO: hmm 
	}


	std::string http::get_domain(const char * url)
	{
		std::smatch matches;
		std::string url_str(url);
		
		if (std::regex_match(url_str, matches, REGEX_DOMAIN_AND_PATH))
			return matches[1];
		else
			return url_str; // TODO: vracat povodny string ak nematchne ??
	}
*/
    
    std::string http::get_url( const_string url, const_string base_url )
    {
        const char * c_url = *url;
        const char * c_base_url = *base_url;
        
        if( c_url == NULL || c_url[0] == 0 )
        {
            return c_base_url;
        }
        else if (strncmp(c_url, "http://", strlen("http://")) == 0)
		{
			return c_url;
		}
		else if (strncmp(c_url, "https://", strlen("https://")) == 0)
		{
			return c_url;
		}
        else
        {
            if( c_url[0] == '/' )
            {
                return get_domain(base_url, true) + *url;
            }
            else
            {
                size_t position = 8;
                size_t last_slash_position = 0;
                
                while( c_base_url[position] != 0 && c_base_url[position] != '?' )
                {
                    if( c_base_url[position] == '/' )
                    {
                        last_slash_position = position;
                    }
                    
                    ++position;
                }
                
                if( last_slash_position == 0 )
                {
                    return std::string(c_base_url) + "/" + c_url;
                }
                else
                {
                    return std::string(c_base_url, last_slash_position+1) + c_url;
                }
            }
        }
    }

    std::string http::get_path( const_string url )
    {
        const char * c_url = *url;
		size_t position = 0;
		size_t domain_start;

		if (strncmp(c_url, "http://", strlen("http://")) == 0)
		{
			position = domain_start = strlen("http://");
		}
		else if (strncmp(c_url, "https://", strlen("https://")) == 0)
		{
			position = domain_start = strlen("https://");
		}

		while (c_url[position] != 0 && c_url[position] != '/' && c_url[position] != '?'){ ++position; }
        
        if( c_url[position] == 0 || c_url[position] == '?' )
        {
            return "";
        }
        else
        {
            size_t start_position = position;
            size_t last_slash_position = position;
            
            while( c_url[position] != 0 && c_url[position] != '?' )
            {
                if( c_url[position] == '/' )
                {
                    last_slash_position = position;
                }
                
                ++position;
            }
            
            return std::string(c_url + start_position, last_slash_position - start_position + 1);
        }
    }

	std::string http::get_domain( const_string url, bool protocol )
    {
        const char * c_url = *url;
		size_t position = 0;
		size_t domain_start = 0;

		if (strncmp(c_url, "http://", strlen("http://")) == 0)
		{
			position = domain_start = strlen("http://");
		}
		else if (strncmp(c_url, "https://", strlen("https://")) == 0)
		{
			position = domain_start = strlen("https://");
		}

		while (c_url[position] != 0 && c_url[position] != '/' && c_url[position] != '?'){ ++position; }

		return  ( protocol ) ? std::string( c_url, position ) : std::string( c_url + domain_start, position - domain_start );
	}

    std::string http::url_encode( const_string text )
    {
        const char * c_text = *text;
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;
        
        size_t i = 0;
        char c;
        while( ( c = c_text[i++] ) )
        {
            if( c > 0 && (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~'))
            {
                escaped << c;
            }
            else if (c == ' ')
            {
                escaped << '+';
            }
            else
            {
                escaped << '%' << std::setw(2) << ((int) c) << std::setw(0);
            }
        }
        
        return escaped.str();
    }

	void http::get( const_string url, std::function<void(http_session, std::string)> callback)
    {
        get( http_session(new http_session_data()), url, callback );
    }
    
	void http::get( const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback)
    {
        get( http_session(new http_session_data()), url, parameters, callback );
    }
    
	void http::get(http_session session, const_string url, std::function<void(http_session, std::string)> callback)
    {
        callback(session, session->request(GET, url, http_request_parameter()));
    }
    
	void http::get(http_session session, const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback)
    {
        callback(session, session->request(GET, url, parameters));
    }
    
	void http::post( const_string url, std::function<void(http_session, std::string)> callback)
    {
        post( http_session(new http_session_data()), url, callback );
    }
    
	void http::post( const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback)
    {
        post( http_session(new http_session_data()), url, parameters, callback );
    }
    
	void http::post(http_session session, const_string url, std::function<void(http_session, std::string)> callback)
    {
        callback(session, session->request(POST, url, http_request_parameter()));
    }
    
	void http::post( http_session session, const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback)
    {
        callback(session, session->request(POST, url, parameters));
    }
    
	void http::request( http_request_type type, const_string url, std::function<void(http_session, std::string)> callback)
    {
        request(type, http_session(new http_session_data()), url, callback );
    }
    
	void http::request( http_request_type type,  const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback)
    {
        request(type, http_session(new http_session_data()), url, parameters, callback );
    }
    
	void http::request( http_request_type type, http_session session, const_string url, std::function<void(http_session, std::string)> callback)
    {
        callback(session, session->request(type, url, http_request_parameter()));
    }
    
	void http::request( http_request_type type, http_session session, const_string url, const http_request_parameter & parameters, std::function<void(http_session, std::string)> callback)
    {
        callback(session, session->request(type, url, parameters));
    }
}
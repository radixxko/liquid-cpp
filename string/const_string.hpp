#ifndef __liquid_cpp__string__const_string__
#define __liquid_cpp__string__const_string__

#include <liquid-cpp/core/defines.hpp>

namespace liquid
{
    #define NULL_const_string (char*)NULL
    
    class const_string_data;

    class const_string
    {
      public:

        const_string( char * string ); // TODO pridat offset ale bacha ze sa nedojebem dakde, pridat aj daky extract co nefreeine data, len vrati char * ked mam len jednu referenciu
        const_string( const char * string = nullptr );
        const_string( const_string const & string );
        const_string & operator=( const char * string );
        const_string & operator=( const_string const & string );
        ~const_string();

        bool is_null() const;
        const char * operator*() const;
        size_t length() const;

      private:

        const_string_data * m_string_data;
    };
}

#endif
#ifndef __liquid_lib_string_string__
#define __liquid_lib_string_string__

#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdio.h>

namespace liquid
{
    #define NULL_const_string (char*)NULL
    
    class const_string_data;

    class const_string
    {
      public:

        const_string( char * string ); // TODO pridat offset ale bacha ze sa nedojebem dakde, pridat aj daky extract co nefreeine data, len vrati char * ked mam len jednu referenciu
        const_string( const char * string = NULL );
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
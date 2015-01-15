#include "const_string.hpp"
#include <atomic>
#include <stdlib.h>
#include <iostream>
#include <string.h>

namespace liquid
{
    size_t INVALID_LENGTH = (size_t)-1;

    class const_string;

    class const_string_data
    {
      private:

        const_string_data( char * data ): m_data(data), m_length(INVALID_LENGTH), m_references_cnt(1){};
        ~const_string_data(){ if( m_data != NULL ){ free( m_data ); } };

      private:

        char * m_data;
        size_t m_length;
        std::atomic_size_t m_references_cnt;

      friend class const_string;
    };
    
    const_string::const_string( char * string )
    {
        if( string == NULL )
        {
            m_string_data = new const_string_data(NULL);
        }
        else
        {
           m_string_data = new const_string_data(string);
        }
    }
    
    const_string::const_string( const char * string )
    {
        if( string == NULL )
        {
            m_string_data = new const_string_data(NULL);
        }
        else
        {
            char * string_copy = (char *) malloc (strlen(string) + 1);
#ifdef _WIN32
			strcpy_s(string_copy, (strlen(string) + 1), string);
#else
			strcpy(string_copy, string);
#endif

            m_string_data = new const_string_data(string_copy);
        }
    }

    const_string::const_string( const_string const & string ): m_string_data(string.m_string_data)
    {
        ++m_string_data->m_references_cnt;
    }

    const_string & const_string::operator=( const char * string )
    {
        if( string == NULL )
        {
            m_string_data = new const_string_data(NULL);
        }
        else
        {
            char * string_copy = (char *) malloc (strlen(string) + 1);
#ifdef _WIN32
			strcpy_s(string_copy, (strlen(string) + 1), string);
#else
			strcpy(string_copy, string);
#endif
       
            m_string_data = new const_string_data(string_copy);
        }
        
        return *this;
    }
    
    const_string & const_string::operator=( const_string const & string )
    {
        const_string_data * const old_data = m_string_data;

        m_string_data = string.m_string_data;
        ++m_string_data->m_references_cnt;

        if( --old_data->m_references_cnt == 0 ){ delete old_data; }

        return *this;
    }

    const_string::~const_string()
    {
        if( --m_string_data->m_references_cnt == 0 )
        {
            delete m_string_data;
        }
    }
    
    bool const_string::is_null() const
    {
        return ( m_string_data->m_data == NULL );
    }

    const char * const_string::operator*() const
    {
        return m_string_data->m_data;
    }

    size_t const_string::length() const
    {
        if( m_string_data->m_length == INVALID_LENGTH )
        {
            m_string_data->m_length = ( m_string_data->m_data != NULL ) ? strlen(m_string_data->m_data) : 0;
        }

        return m_string_data->m_length;
    }
}
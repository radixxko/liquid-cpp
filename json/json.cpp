#include <liquid-cpp/string/string.hpp>

#include "json.hpp"
#include <string.h>
#include <sstream>


namespace liquid
{
    std::string to_string( double number )
    {
        std::stringstream ss;
        ss << number;
        
        return ss.str();
    }

    json_object_iterator::json_object_iterator()
    {

    }

    json_object_iterator::json_object_iterator( std::map<std::string, json>::const_iterator iterator )
    :
        m_iterator(iterator)
    {
    }

    json_object_iterator & json_object_iterator::operator ++()
    {
        ++m_iterator;
        return *this;
    }

    bool json_object_iterator::operator !=(json_object_iterator object_iterator)
    {
        return m_iterator != object_iterator.m_iterator;
    }

    std::string json_object_iterator::key()
    {
        return m_iterator->first;
    }

    json json_object_iterator::value()
    {
        return m_iterator->second;
    }

    json json::parse( std::string json_str )
    {
        const char * text = json_str.c_str();
        size_t length = json_str.size();
        size_t position = 0;

        while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;

        if( text[position] == '{' )
        {
            ++position;
            return json::decode_object( text, length, position );
        }
        else if( text[position] == '[' )
        {
            ++position;
            return json::decode_array( text, length, position );
        }

        return json();
    }

    json::json()
        : m_type(json_error)
    {

    }

    json::json( json_type type ) 
        : m_type(type)
    {

    }

    json::json( std::string string )
    : m_type(json_string),
    m_string(string)
    {
        
    }

    json::json( double number )
    : m_type(json_number),
    m_number(number)
    {
        
    }

    json::json( int number )
    : m_type(json_number),
    m_number(number)
    {
        
    }

    json::json( bool boolean )
    : m_type(json_bool),
    m_bool(boolean)
    {
        
    }

    json::json( std::vector<json> array ) 
        : m_type(json_array), 
        m_array(array)
    {

    }

    json::json( std::map<std::string, json> object )
        : m_type(json_object), 
        m_object(object)
    {

    }

    json & json::operator= (const json & json)
    {
        if (this != &json)
        {
            this->m_type    = json.m_type;
            this->m_object  = json.m_object;
            this->m_array   = json.m_array;
            this->m_string  = json.m_string;
            this->m_number  = json.m_number;
            this->m_bool    = json.m_bool;
        }

        return *this;
    }

    json & json::operator= (const char * string)
    {
        this->m_type    = json_string;
        this->m_string  = string;
        
        return *this;
    }

    json & json::operator= (const std::string & string)
    {
        this->m_type    = json_string;
        this->m_string  = string;

        return *this;
    }

    json & json::operator= (double number)
    {
        this->m_type    = json_number;
        this->m_number  = number;
        
        return *this;
    }

    json & json::operator= (int number)
    {
        this->m_type    = json_number;
        this->m_number  = number;
        
        return *this;
    }

    json & json::operator= (bool boolean)
    {
        this->m_type    = json_bool;
        this->m_bool    = boolean;
        
        return *this;
    }

    const json json::operator [](std::string key) const
    {
        if( m_type == json_object )
        {
            std::map<std::string, json>::const_iterator value = m_object.find(key);

            if( value != m_object.end() )
            {
                return value->second;
            }
        }

        return json();
    }

    const json json::operator [](size_t index) const
    {
        if( m_type == json_array )
        {
            if( index < m_array.size() )
            {
                return m_array[index];
            }
        }

        return json();
    }

    json & json::operator [](std::string key)
    {
        /*if( m_type == json_object && m_object.find(key) != m_object.end() )
        {
            return m_object[key];
        }

        return json();*/

        if( m_type == json_error )
        {
            m_type = json_object;
            m_object[key] = json();
        }

        if( m_type == json_object )
        {
            return m_object[key];
        }

        return m_object[key]; // but its inaccesible
    }

    json & json::operator [](size_t index)
    {
        if( m_type == json_array )
        {
            if( index < m_array.size() )
            {
                return m_array[index];
            }
        }

        return *this; // ERROR
    }

    json & json::push(json json)
    {
        if( m_type == json_error )
        {
            m_type = json_array;
            m_array.clear();
        }
        
        if( m_type == json_array )
        {
            m_array.push_back(json);
        }

        return m_array.back();
    }

    json & json::push(std::string string)
    {
        if( m_type == json_error )
        {
            m_type = json_array;
            m_array.clear();
        }
        
        if( m_type == json_array )
        {
            m_array.push_back(json(string));
        }

        return m_array.back();
    }

    json_object_iterator json::object_begin() const
    {
        return json_object_iterator(m_object.begin());
    }

    json_object_iterator json::object_end() const
    {
        return json_object_iterator(m_object.end());
    }

    std::string json::type() const
    {
        if( m_type == json_error ){
            return "JSON_ERROR";
        }
        else if( m_type == json_string ){
            return "JSON_STRING";
        }
        else if( m_type == json_array ){
            return "JSON_ARRAY";
        }
        else if( m_type == json_object ){
            return "JSON_OBJECT";
        }
        else if( m_type == json_number ){
            return "JSON_NUMBER";
        }
        else if( m_type == json_bool ){
            return "JSON_BOOL";
        }

        return "JSON_UNKNOWN";
    }

    std::string json::string() const
    {
        if( m_type == json_string )
        {
            return m_string;
        }
        else if( m_type == json_number )
        {
            return to_string(m_number);
        }
        else if( m_type == json_bool )
        {
            return ( m_bool ? "true" : "false" );
        }
        else if( m_type == json_array )
        {
            std::ostringstream stringified;
            stringified << "[";

            for( std::vector<json>::const_iterator element = m_array.begin(); element != m_array.end(); ++element )
            {
                if( element != m_array.begin() ){ stringified << ","; }

                if( element->m_type == json_string )
                {
                    stringified << "\"" << escape_string(element->string()) << "\"";
                }
                else
                {
                    stringified << element->string();
                }
            }

            stringified << "]";

            return stringified.str();
        }
        else if( m_type == json_object )
        {
            std::ostringstream stringified;
            stringified << "{";

            for( std::map<std::string, json>::const_iterator property = m_object.begin(); property != m_object.end(); ++property )
            {
                if( property != m_object.begin() ){ stringified << ","; }

                stringified << "\"" << property->first << "\":";

                if( property->second.m_type == json_string )
                {
                    stringified << "\"" << escape_string(property->second.string()) << "\"";
                }
                else if( property->second.m_type == json_error )
                {
                    stringified << "\"\"";
                }
                else
                {
                    stringified << property->second.string();
                }
            }

            stringified << "}";

            return stringified.str();
        }
        else if( m_type == json_error )
        {
            return "null";
        }

        return "";
    }

    double json::number() const
    {
        if( m_type == json_string )
        {
            return atof(m_string.c_str());
        }
        else if( m_type == json_number )
        {
            return m_number;
        }

        return 0;
    }

    int json::integer() const
    {
        if( m_type == json_string )
        {
            return atoi(m_string.c_str());
        }
        else if( m_type == json_number )
        {
            return (int)m_number;
        }

        return 0;
    }

    bool json::boolean() const
    {
        if (m_type == json_string)
        {
            return ( m_string == "true" ) ? true : false;
        }
        else if (m_type == json_number)
        {
            return ( m_number == 1 ) ? true : false;
        }
        else if (m_type == json_bool)
        {
            return m_bool;
        }

        return false;
    }

    std::map<std::string,std::string> json::key_string() const
    {
        std::map<std::string,std::string> values;

        for( std::map<std::string, json>::const_iterator iterator = m_object.begin(); iterator != m_object.end(); ++iterator )
        {
            values[iterator->first] = iterator->second.string();
        }

        return values;
    }

    bool json::empty() const
    {
        if( m_type == json_error ){
            return true;
        } else if( m_type == json_string ){
            return (m_string == "");
        } else if( m_type == json_array ){
            return m_array.empty();
        } else if( m_type == json_object ){
            return m_object.empty();
        }
        return true;
    }

    bool json::is_set( std::string key ) const
    {
        if( m_type != json_object ) return false;

        return ( m_object.find(key) != m_object.end() );
    }

    bool json::in_array( std::string value ) const
    {
        if( m_type != json_array ) return false;

        for( std::vector<json>::const_iterator i = m_array.begin(); i != m_array.end(); ++i )
        {
            if( (i->m_type == json_string) && (i->m_string == value) ) 
                return true;
        }

        return false;
    }

    bool json::in_array( std::string value, int & index ) const
    {
        index = -1;
        int j = 0;

        if( m_type != json_array ){
            return false;
        }

        for( std::vector<json>::const_iterator i = m_array.begin(); i != m_array.end(); ++i )
        {
            if( i->m_type == json_string && i->m_string == value ){
                index = j;
                return true;
            }

            j++;
        }

        return false;
    }

    size_t json::size() const
    {
        if( m_type == json_array ){
            return m_array.size();
        } else if ( m_type == json_object ){
            return m_object.size();
        }

        return 0;
    }

    bool json::is_char_escaped( const char * text, size_t position )
    {
        size_t escaped = 0;
        while( text[--position] == '\\' ) ++escaped;

        return (escaped % 2 == 0) ? false : true;
    }

    std::string json::escape_string( const std::string text )
    {
        std::string escaped = text;
        
        escaped = string::replace(escaped,"\\","\\\\");
        escaped = string::replace(escaped,"\"","\\\"");
        escaped = string::replace(escaped,"\r","\\r");
        escaped = string::replace(escaped,"\n","\\n");
        
        return escaped;
    }

    std::string json::unescape_string( const std::string text )
    {
        std::string unescaped = text;

        unescaped = string::replace(unescaped,"\\n","\n");
        unescaped = string::replace(unescaped,"\\r","\r");
        unescaped = string::replace(unescaped,"\\\"","\"");
        unescaped = string::replace(unescaped,"\\\\","\\");

        return unescaped;
    }

    json json::decode_value( const char * text, size_t length, size_t & position )
    {
        while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;

        if( text[position] == '{' )
        {
            ++position;
            return json::decode_object( text, length, position );
        }
        else if( text[position] == '[' )
        {
            ++position;
            return json::decode_array( text, length, position );
        }
        else if( text[position] == '"' )
        {
            size_t start = ++position;
            while( position < length )
            {
                if( ( text[position] == '"' ) && ( !json::is_char_escaped( text, position ) ) )
                {
                    ++position;
                    return unescape_string(std::string( text, start, position - start - 1 ));
                }
                ++position;
            }
        }
        else if( (text[position] >= '0' && text[position] <= '9') || (text[position] == '-' || text[position] == '.'))
        {
            size_t start = position++;
            while( position < length )
            {
                if( ( text[position] < '0' || text[position] > '9') && text[position] != '.' && text[position] != '-')
                {
                    return atof(std::string( text, start, position - start ).c_str());
                }
                ++position;
            }
        }
        else if( text[position] == 'n' && strncmp(text+position,"null",4) == 0 )
        {
            position += 4;
            return json();
        }
        else if( text[position] == 'f' && strncmp(text+position,"false",5) == 0 )
        {
            position += 5;
            return false;
        }
        else if( text[position] == 't' && strncmp(text+position,"true",4) == 0 )
        {
            position += 4;
            return true;
        }

        ++position;
        return json();
    }

    std::vector<json> json::decode_array( const char * text, size_t length, size_t & position )
    {
        std::vector<json> array;

        while( position < length )
        {
            while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;

            if( text[position] == ']' )
            {
                ++position;
                return array;
            }
            else
            {
                array.push_back( json::decode_value( text, length, position ) );

                while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;
                if( text[position] == ',' ) ++position;
            }
        }

        ++position;
        return array;
    }

    std::map<std::string, json> json::decode_object( const char * text, size_t length, size_t & position )
    {
        std::map<std::string, json> object;

        while( position < length )
        {
            while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;

            if( text[position] == '"' )
            {
                std::string key = json::decode_key( text, length, position );

                while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;

                if( text[position] == ':' )
                {
                    ++position;
                    object[key] = json::decode_value( text, length, position );
                }
                else
                {
                    ++position;
                    return object; // NULL
                }

                while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;
                if( text[position] == ',' ) ++position;
            }
            else if( text[position] == '}' )
            {
                ++position;
                return object;
            }
            else
            {
                ++position;
                return object; // NULL
            }
        }

        ++position;
        return object;
    }

    std::string json::decode_key( const char * text, size_t length, size_t & position )
    {
        while( ( text[position] == ' ' || text[position] == '\t' || text[position] == '\n' || text[position] == '\r' ) && ( position < length ) ) ++position;

        if( text[position] == '"' )
        {
            size_t start = ++position;
            while( position < length )
            {
                if( ( text[position] == '"' ) && ( !json::is_char_escaped( text, position ) ) )
                {
                    ++position;
                    return std::string( text, start, position - start - 1 );
                }
                ++position;
            }
        }

        ++position;
        return "";
    }

    json::~json()
    {

    }

    bool json::erase( std::string key )
    {
        if( m_type == json_object )
        {
            std::map<std::string,json>::iterator value = m_object.find(key);

            if( value != m_object.end() )
            {
                m_object.erase(value);

                return true;
            }
        }

        return false;
    }

    bool json::erase( size_t index )
    {
        if( m_type == json_array )
        {
            if( index < m_array.size() )
            {
                m_array.erase(m_array.begin() + index);

                return true;
            }
        }

        return false;
    }
}
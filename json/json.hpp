#ifndef __liquid_cpp__json__json__
#define __liquid_cpp__json__json__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <sstream>

namespace liquid
{
	enum json_type
    {
		json_error, 
		json_string, 
		json_array, 
		json_object,
        json_number,
        json_bool
	};

	class json_object_iterator;

	class json
	{
	  public:

		static json parse( std::string json );
		json();
		json( json_type type );
		~json();

		json & operator= (const json & json);
		json & operator= (const char * string);
		json & operator= (const std::string & string);
		json & operator= (double number);
		json & operator= (int number);
		json & operator= (bool boolean);

		const json operator [](std::string key) const;
		const json operator [](size_t index) const;
		json & operator [](std::string key);
		json & operator [](size_t index);
		json & push(json json);
		json & push(std::string string);

		json_object_iterator object_begin() const;
		json_object_iterator object_end() const;

		std::string type() const;
		std::string string() const;
		double number() const;
		int integer() const;
		bool boolean() const;
		std::map<std::string,std::string> key_string() const;

		bool empty() const;
		bool is_set( std::string key ) const;
		bool in_array( std::string value ) const;
		bool in_array( std::string value, int & index ) const;
		size_t size() const;

		bool erase( std::string key );
        bool erase( size_t index ); // WARNING iterate downwards --i for loop erase

	  private:

		json( std::string string );
        json( double number );
        json( int number );
        json( bool boolean );
		json( std::vector<json> array );
		json( std::map<std::string,json> object );

	  private:

		static bool is_char_escaped( const char * text, size_t position );
		static json decode_value( const char * text, size_t length, size_t & position );
		static std::vector<json> decode_array( const char * text, size_t length, size_t & position );
		static std::map<std::string,json> decode_object( const char * text, size_t length, size_t & position );
		static std::string decode_key( const char * text, size_t length, size_t & position );
		static std::string escape_string( const std::string text );
		static std::string unescape_string( const std::string text );

	  private:

		json_type                   m_type;
		std::map<std::string,json>  m_object;
		std::vector<json>           m_array;
		std::string                 m_string;
        double                      m_number;
        bool                        m_bool;
	};

	class json_object_iterator
	{
	  public:
        
		json_object_iterator();
		json_object_iterator( std::map<std::string,json>::const_iterator iterator );

		json_object_iterator & operator ++();
		bool operator !=(json_object_iterator object_iterator);

		std::string key();
		json value();

	  private:
        
		std::map<std::string, json>::const_iterator m_iterator;
	};
}

#endif
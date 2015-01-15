#ifndef __liquid_cpp__html__html__
#define __liquid_cpp__html__html__

#include <liquid-cpp/core/defines.hpp>

#include <liquid-cpp/string/const_string.hpp>


#include <string>
#include <stdlib.h>
#include <vector>
#include <atomic>
#include <unordered_map>

namespace liquid
{
    class html_document;
    class html_document_node;
    class html_document_node_list;
    class html_document_data;
    
    class html
    {
      public:
        
        static html_document parse( const_string source );
    };

    class html_document
    {
      public:

       //bool exists() const{};

        bool dump( const char * filename/*, html_document_node_list_item & node*/ ) const;
        bool save( const char * filename/*, html_document_node_list_item & node*/ ) const;
        
        const html_document_node    query_selector( std::string selector ) const;
        const html_document_node_list     query_selector_all(std::string selector) const;
        
        bool is_ok( ) const;

      private:

        html_document( const_string source );

      private:

        html_document_data * m_document_data;

      friend html_document html::parse( const_string source );
	  friend html_document_node;
	  friend html;
    };
    
    class html_document_node
    {
      public:
        
        const html_document_node    parent_node() const;
        const html_document_node    previous_sibling() const;
        const html_document_node    next_sibling() const;
        const html_document_node    first_child() const;
        const html_document_node    last_child() const;
        const html_document_node    nth_child( size_t n ) const;
        const html_document_node    query_selector( std::string selector ) const;
        const html_document_node    nth_child_selector( size_t n, std::string selector ) const;
        const html_document_node    parent_selector( std::string selector ) const;
        
        const html_document_node_list   child_nodes() const;
        const html_document_node_list   query_selector_all( std::string selector ) const;
        
        const bool  has_class( std::string class_name ) const;
        
        const std::string   tag() const;
        bool                has_attribute( std::string attribute ) const;
        const std::string   attribute( std::string attribute ) const;
        const std::string   outer_text() const;
        const std::string   inner_text() const;
        const std::string   plain_text() const;
        const std::string   outer_text_except( html_document_node_list & except_nodes ) const;
        const std::string   inner_text_except( html_document_node_list & except_nodes ) const;
        const std::string   plain_text_except( html_document_node_list & except_nodes ) const;
        
        const std::string   unique_hash() const;
        bool                exists() const;
        
      private:
        
        html_document_node( html_document document, size_t document_node ): m_document(document), m_document_node(document_node){};
        
      private:
        
        html_document m_document;
        size_t m_document_node;
        
      friend html_document;
    };
    
    class html_document_node_list
    {
      public:
        
        html_document_node_list(){};
        html_document_node_list(std::vector<html_document_node> node_list);
        
        html_document_node operator[]( int index ) const
        {
            return m_document_node_list[index];
        }
        
        size_t size() const
        {
            return m_document_node_list.size();
        }
        
      private:
        
        std::vector<html_document_node> m_document_node_list;
        
      friend html_document_node;
    };
    
    class html_form
    {
      public:
        
        html_form( html_document_node form );
        
        bool set_value( std::string name, std::string value );
        std::string get_action();
        std::string get_method();
        std::unordered_map<std::string, std::string> get_values();
        void dump();
        
      private:
        
        html_document_node m_form;
        const_string m_action;
        const_string m_method;
        std::unordered_map<std::string, std::string> m_elements;
    };
}


#endif

//#endif
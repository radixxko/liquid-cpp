#include "../http/http.hpp"
#include "html.hpp"
#include "entities.h"
#include <iostream>
#include <string.h>
#include <map>
#include <regex>

//str to lower
#include <algorithm>

namespace liquid
{

#define INVALID_NODE (size_t)-1
#define INVALID_OFFSET (size_t)-1

#ifdef _WIN32
#define strnicmp _strnicmp
#define strcasecmp _stricmp
#else
#define strnicmp strncasecmp
#endif // _WIN32


	char *strnstr(const char *s, const char *find, size_t slen) {
		char c, sc;
		size_t len;

		if ((c = *find++) != '\0') {
			len = strlen(find);
			do {
				do {
					if ((sc = *s++) == '\0' || slen-- < 1)
						return (NULL);
				} while (sc != c);
				if (len > slen)
					return (NULL);
			} while (strnicmp(s, find, len) != 0);
			s--;
		}
		return ((char *)s);
	}

	struct html_document_node_list_item
	{
		size_t node_start;
		size_t node_end;
		size_t tag_end;
		size_t node;
		size_t first_child;
		size_t last_child;
		size_t next_sibling;
		size_t previous_sibling;
		size_t parent_node;
	};

	struct html_selector_node
	{
		std::string tag;
		std::string id;
		std::vector<std::string> classes;
		std::map<std::string, std::string> attributes;
	};

	struct html_selector_match
	{
		bool matched;
		size_t matched_nodes;
	};

	class html_selector
	{
	private:

		std::vector<html_selector_node> m_nodes;

	public:

		html_selector(std::string selector);
		html_selector_match match(const char * source, html_document_node_list_item node, size_t matched_nodes = 0);
	};

	html_form::html_form(html_document_node form) :m_form(form)
	{
		m_action = form.attribute("action").c_str();
		m_method = form.attribute("method").c_str();

		html_document_node_list inputs = form.query_selector_all("input");

		for (int i = 0; i < (int)inputs.size(); ++i)
		{
			std::string type = inputs[i].attribute("type");

			if (type == "submit" || type == "button")
			{
				//nist
			}
			else if (type == "radio")
			{
				// nevim si rady
			}
			else if (type == "checkbox")
			{
				// nevim si rady
			}
			else
			{
				m_elements[inputs[i].attribute("name")] = inputs[i].attribute("value");
			}
		}

		html_document_node_list selects = form.query_selector_all("select");
		for (int i = 0; i < (int)selects.size(); ++i)
		{
			html_document_node_list options = selects[i].query_selector_all("options");

			if (options.size() > 0)
			{
				m_elements[selects[i].attribute("name")] = options[0].attribute("value");
			}

			for (int j = 0; j < (int)options.size(); ++j)
			{
				if (options[j].has_attribute("selected"))
				{
					m_elements[selects[i].attribute("name")] = options[j].attribute("value");

					break;
				}
			}
		}

		html_document_node_list textareas = form.query_selector_all("textarea");
		for (int i = 0; i < (int)textareas.size(); ++i)
		{
			m_elements[textareas[i].attribute("name")] = textareas[i].inner_text();
		}
	}

	void html_form::dump()
	{
		for (std::unordered_map<std::string, std::string>::const_iterator val = m_elements.begin(); val != m_elements.end(); ++val)
		{
			std::cout << val->first << " = " << val->second << std::endl;
		}
	}

	bool html_form::set_value(std::string name, std::string value)
	{
		m_elements[name] = value;

		return true;
	}

	std::string html_form::get_method()
	{
		return (!m_method.is_null() && strcasecmp(*m_method, "POST") == 0) ? "POST" : "GET";
	}

	std::string html_form::get_action()
	{
		return *m_action;
	}

	std::unordered_map<std::string, std::string> html_form::get_values()
	{
		return m_elements;
	}

	enum SELECTOR_TYPE { NONE, TAG, ID, CLASS, ATTRIBUTE };

	html_selector::html_selector(std::string selector)
	{
		SELECTOR_TYPE type = NONE;
		char sub_selector[64];
		size_t sub_selector_position = 0;
		size_t open_square_brackets = 0;

		html_selector_node node;

		for (size_t i = 0; i < selector.length(); i++)
		{
			if (type == ATTRIBUTE)
			{
				if (selector[i] == ']' && --open_square_brackets == 0)
				{
					sub_selector[sub_selector_position++] = 0;

					size_t value = 0;
					while (sub_selector[value++] != '=');
					sub_selector[value - 1] = 0;
					node.attributes[sub_selector] = sub_selector + value;

					type = NONE; sub_selector_position = 0;
				}
				else
				{
					if (selector[i] == '['){ ++open_square_brackets; }

					sub_selector[sub_selector_position++] = selector[i];
				}
			}
			else if (selector[i] == '#')
			{
				sub_selector[sub_selector_position++] = 0;
				if (sub_selector_position > 1)
				{
					if (type == ID){ node.id = sub_selector; }
					else if (type == TAG){ node.tag = sub_selector; }
					else if (type == CLASS){ node.classes.push_back(sub_selector); }
				}

				type = ID; sub_selector_position = 0;
			}
			else if (selector[i] == '.')
			{
				sub_selector[sub_selector_position++] = 0;
				if (sub_selector_position > 1)
				{
					if (type == ID){ node.id = sub_selector; }
					else if (type == TAG){ node.tag = sub_selector; }
					else if (type == CLASS){ node.classes.push_back(sub_selector); }
				}

				type = CLASS; sub_selector_position = 0;
			}
			else if (selector[i] == '[')
			{
				open_square_brackets = 1;
				sub_selector[sub_selector_position++] = 0;
				if (sub_selector_position > 1)
				{
					if (type == ID){ node.id = sub_selector; }
					else if (type == TAG){ node.tag = sub_selector; }
					else if (type == CLASS){ node.classes.push_back(sub_selector); }
				}

				type = ATTRIBUTE; sub_selector_position = 0;
			}
			else if (selector[i] == ' ')
			{
				sub_selector[sub_selector_position++] = 0;
				if (sub_selector_position > 1)
				{
					if (type == ID){ node.id = sub_selector; }
					else if (type == TAG){ node.tag = sub_selector; }
					else if (type == CLASS){ node.classes.push_back(sub_selector); }
				}

				m_nodes.push_back(node); node.id = ""; node.tag = ""; node.classes.clear(); node.attributes.clear();

				type = NONE; sub_selector_position = 0;
			}
			else if (type == NONE)
			{
				sub_selector[sub_selector_position++] = 0;
				if (sub_selector_position > 1)
				{
					if (type == ID){ node.id = sub_selector; }
					else if (type == TAG){ node.tag = sub_selector; }
					else if (type == CLASS){ node.classes.push_back(sub_selector); }
				}

				sub_selector_position = 0;
				sub_selector[sub_selector_position++] = selector[i];

				type = TAG;
			}
			else
			{
				sub_selector[sub_selector_position++] = selector[i];
			}
		}

		sub_selector[sub_selector_position++] = 0;

		if (sub_selector_position > 1)
		{
			if (type == ID){ node.id = sub_selector; }
			else if (type == TAG){ node.tag = sub_selector; }
			else if (type == CLASS){ node.classes.push_back(sub_selector); }
		}

		m_nodes.push_back(node);

		//std::cout << std::endl << "Selector" << std::endl;

		/*for (size_t i = 0; i < m_nodes.size(); i++)
		{
			if (m_nodes[i].tag != "") std::cout << "tag = " << m_nodes[i].tag << " , ";
			if (m_nodes[i].id != "") std::cout << "id = " << m_nodes[i].id << " , ";
			if (m_nodes[i].classes.size() > 0)
			{
				//std::cout << "classes : ";
				for (size_t j = 0; j < m_nodes[i].classes.size(); j++)
				{
					//std::cout << m_nodes[i].classes[j] << " , ";
				}
			}
			if (m_nodes[i].attributes.size() > 0)
			{
				//std::cout << "attributes : ";
				for (std::map<std::string, std::string>::const_iterator j = m_nodes[i].attributes.begin(); j != m_nodes[i].attributes.end(); ++j)
				{
					//std::cout << j->first << " = " << j->second << " , ";
				}
			}
			//std::cout << std::endl;
		}*/

		//std::cout << std::endl;
	}

	html_selector_match html_selector::match(const char * source, html_document_node_list_item node, size_t matched_nodes)
	{
		size_t position = node.node_start + 1;
		html_selector_match matched;
		matched.matched_nodes = (matched_nodes > m_nodes.size() - 1) ? m_nodes.size() - 1 : matched_nodes;
		matched.matched = false;

		//std::cout << "match " << std::string(source, position, 10) << std::endl;

		bool tag_matched;
		bool id_matched;
		bool classes_matched;
		size_t attributes_matched;

		tag_matched = (m_nodes[matched.matched_nodes].tag != "") ? false : true;
		id_matched = (m_nodes[matched.matched_nodes].id != "") ? false : true;
		classes_matched = (m_nodes[matched.matched_nodes].classes.size() > 0) ? false : true;
		attributes_matched = 0;

		if (!tag_matched)
		{
			tag_matched = ((m_nodes[matched.matched_nodes].tag.length() == node.tag_end - position) && (strnicmp(m_nodes[matched.matched_nodes].tag.c_str(), source + position, node.tag_end - position) == 0));
		}

		if (tag_matched)
		{
			while (source[position] != '>' && source[position] != 0)
			{
				while (source[position] == ' ' || source[position] == '\t' || source[position] == '\n' || source[position] == '\r') ++position;

				if (source[position] != '>' && source[position] != 0)
				{
					size_t attr_start = position, attr_end;
					while (source[position] != '>' && source[position] != ' ' && source[position] != '=' && source[position] != 0) ++position;
					attr_end = position;
					//printf( "%.*s ", attr_end - attr_start, element + attr_start );
					if (source[position] == '=')
					{
						char quote = '"';
						if (source[position + 1] == '\'' || source[position + 1] == '"'){
							quote = source[position + 1];
							position += 2;
						}
						else{
							quote = ' ';
							position += 1;
						}
						size_t value_start = position, value_end;

						while (source[position] != '>' && source[position] != quote && source[position] != 0){ ++position; }

						value_end = position;
						//printf( "= %.*s\n", value_end - value_start, element + value_start );

						if ((attr_end - attr_start == 5) && (strnicmp("class", source + attr_start, attr_end - attr_start) == 0))
						{
							//printf("%.*s\n", value_end - value_start, element + value_start);
							if (m_nodes[matched.matched_nodes].classes.size() > 0)
							{
								classes_matched = true;
								for (size_t i = 0; i < m_nodes[matched.matched_nodes].classes.size(); ++i)
								{
									if (strnstr(source + value_start, m_nodes[matched.matched_nodes].classes[i].c_str(), value_end - value_start))
									{
										//std::cout << "=== " << this->nodes[0].classes[i] << " Je" << std::endl;
									}
									else
									{
										classes_matched = false;
										//std::cout << "=== " << this->nodes[0].classes[i] << " Neni" << std::endl;
										break;
									}
								}
								//std::cout << "Treba tote classy robic" << std::endl;
							}
						}
						else if ((attr_end - attr_start == 2) && (strnicmp("id", source + attr_start, attr_end - attr_start) == 0))
						{
							if ((m_nodes[matched.matched_nodes].id == "") || ((value_end - value_start == m_nodes[matched.matched_nodes].id.length()) && (strnicmp(m_nodes[matched.matched_nodes].id.c_str(), source + value_start, value_end - value_start) == 0)))
							{
								id_matched = true;
							}
							else
							{
								id_matched = false;
							}
						}
						else
						{
							std::map<std::string, std::string>::const_iterator attr = m_nodes[matched.matched_nodes].attributes.find(std::string(source + attr_start, attr_end - attr_start));
							if (attr != m_nodes[matched.matched_nodes].attributes.end())
							{
								//printf("%.*s\n", value_end - value_start, element + value_start);

								std::string valueStr(source + value_start, value_end - value_start);

								//if( std::regex_match(source + value_start, source + value_end, std::regex(attr->second)) )
								if (valueStr == attr->second)
								{
									++attributes_matched;
								}

								/*if( ( value_end - value_start == attr->second.length() ) && ( strnicmp(attr->second.c_str(), element + value_start, value_end - value_start) == 0 ) )
								 {
								 ++attributes_matched;
								 }*/
							}
						}

						while (source[position] != '>' && source[position] != ' ' && source[position] != 0) ++position;
					}
				}
			}

			if (tag_matched && classes_matched && id_matched && (attributes_matched == m_nodes[matched.matched_nodes].attributes.size() - 1) && m_nodes[matched.matched_nodes].attributes.find("text") != m_nodes[matched.matched_nodes].attributes.end())
			{
				position = 1;
				while (source[position] != 0 && source[position++] != '>');

				std::map<std::string, std::string>::const_iterator attr = m_nodes[matched.matched_nodes].attributes.find("text");

				//if( std::regex_match( element + position, std::regex(attr->second)) )
				if (strnicmp(source + position, attr->second.c_str(), attr->second.length()) == 0)
				{
					++attributes_matched;
				}
			}

			if (tag_matched && classes_matched && id_matched && (attributes_matched >= m_nodes[matched.matched_nodes].attributes.size()))
			{
				++matched.matched_nodes;
				matched.matched = (matched.matched_nodes == m_nodes.size());

				return matched;
			}

			return matched;
		}

		return matched;
	}

	const char *leafTags[] = { "br", "meta", "link", "img", "input", "hr", "param", "col" };
	const int leafTagsLengths[] = { 2, 4, 4, 3, 5, 2, 5, 3 };
	const int leafTags_cnt = 8;

	double NODES_TO_SOURCE_RATIO = 1 / 100.0f;

	class html_document_data
	{
	private:

		html_document_data(const_string source);

		size_t add_node(size_t node_start, size_t node_end, size_t tag_end, size_t parent_node);
		size_t update_node(size_t node, size_t node_start, size_t node_end, size_t tag_end);
		size_t parse_node(size_t parent, size_t position);

		bool is_ok(){ return true; }

	private:

		const_string m_source;
		const char * m_source_data;
		size_t m_source_length;

		html_document_node_list_item * m_nodes;
		size_t	m_nodes_cnt;
		size_t	m_nodes_size;

		std::atomic_size_t m_references_cnt;

		friend class html_document;
		friend class html_document_node;
	};

	html_document_data::html_document_data(const_string source) : m_source(source), m_source_data(*source), m_source_length(source.length())
	{
		m_nodes_cnt = 0;
		m_nodes_size = (size_t)(m_source.length() * NODES_TO_SOURCE_RATIO) + 10;
		m_nodes = (html_document_node_list_item *)malloc(m_nodes_size * sizeof(html_document_node_list_item));

		//std::cout << "HTML " << m_source.length() << " " << m_nodes_size << std::endl;

		size_t root = add_node(0, 0, 0, INVALID_NODE);

		size_t offset = 0, children_offset = 0;
		while (offset != (children_offset = parse_node(root, children_offset))){ offset = children_offset; }
	}

	size_t html_document_data::add_node(size_t node_start, size_t node_end, size_t tag_end, size_t parent_node)
	{
		if (m_nodes_cnt >= m_nodes_size)
		{
			m_nodes_size = (size_t)(m_nodes_size * 1.2);
			m_nodes = (html_document_node_list_item *)realloc(m_nodes, m_nodes_size * sizeof(html_document_node_list_item));
		}

		m_nodes[m_nodes_cnt].node = m_nodes_cnt;
		m_nodes[m_nodes_cnt].node_start = node_start;
		m_nodes[m_nodes_cnt].node_end = node_end;
		m_nodes[m_nodes_cnt].tag_end = tag_end;
		m_nodes[m_nodes_cnt].parent_node = parent_node;
		m_nodes[m_nodes_cnt].first_child = m_nodes[m_nodes_cnt].last_child = m_nodes[m_nodes_cnt].next_sibling = m_nodes[m_nodes_cnt].previous_sibling = INVALID_NODE;

		if (parent_node != INVALID_NODE)
		{
			if (m_nodes[parent_node].last_child != INVALID_NODE){ m_nodes[m_nodes[parent_node].last_child].next_sibling = m_nodes_cnt; }
			m_nodes[m_nodes_cnt].previous_sibling = m_nodes[parent_node].last_child;
			if (m_nodes[parent_node].first_child == INVALID_NODE){ m_nodes[parent_node].first_child = m_nodes_cnt; }
			m_nodes[parent_node].last_child = m_nodes_cnt;
		}

		//std::string tag(m_source_data + node_start, tag_end - node_start);
		//html_document_node_list_item tmp = m_nodes[m_nodes_cnt];

		return m_nodes_cnt++;
	}

	size_t html_document_data::update_node(size_t node, size_t node_start, size_t node_end, size_t tag_end)
	{
		if (node >= m_nodes_cnt){ return 0; }

		m_nodes[node].node_start = node_start;
		m_nodes[node].node_end = node_end;
		m_nodes[node].tag_end = tag_end;

		return node;
	}

	size_t html_document_data::parse_node(size_t parent, size_t position)
	{
		if (position >= m_source_length){ return m_source_length; }

		size_t offset = position;

		html_document_node_list_item node;

		while (true)
		{
			while (m_source_data[offset] && m_source_data[offset] != '<'){ ++offset; } // Najdeme zaciatok tagu

			if (m_source_data[offset] == 0){ return offset; }

			if (m_source_data[offset + 1] && m_source_data[offset + 1] == '/'){ return offset - 1; } // UZAVRETY TAG, cakal som otvoreny

			if (m_source_data[offset + 1] && m_source_data[offset + 1] == '!' && m_source_data[offset + 2] == '-' && m_source_data[offset + 3] == '-') // Komentar, tak ho preskocime
			{
				const char *comment = "-->";
				size_t matches = 0;

				while (comment[matches] != 0 && m_source_data[offset] != 0){ if (comment[matches++] != m_source_data[offset++]) matches = 0; }
			}
			else if (m_source_data[offset + 1] && m_source_data[offset + 1] == '!') // Kratky komentar
			{
				while (m_source_data[offset] != 0 && m_source_data[offset] != '>') ++offset;
			}
			else if (m_source_data[offset + 1] && !((m_source_data[offset + 1] >= 'a' && m_source_data[offset + 1] <= 'z') || (m_source_data[offset + 1] >= 'A' && m_source_data[offset + 1] <= 'Z') || (m_source_data[offset + 1] == ':')))
			{
				++offset;
			}
			else break;
		}

		node.node_start = offset++; // Otvoreny tag, poznacim zaciatok
		node.node_end = 0;

		while (m_source_data[offset] && (((m_source_data[offset] >= 'a') && (m_source_data[offset] <= 'z')) || ((m_source_data[offset] >= 'A') && (m_source_data[offset] <= 'Z')) || (m_source_data[offset] == ':') || ((m_source_data[offset] >= '0') && (m_source_data[offset] <= '9')))){ ++offset; } // Nacitam meno tagu

		node.tag_end = offset; // Nasiel som koniec tagu

		if (strnicmp(m_source_data + node.node_start + 1, "script", 6) == 0 || strnicmp(m_source_data + node.node_start + 1, "style", 5) == 0) // Debilne tagy ktorych obsah uz nie je HTML Node
		{
			char tag[64];
#ifdef _WIN32
			strncpy_s(tag, 64, "</", 2);
			strncpy_s(tag + 2, 64, m_source_data + node.node_start + 1, 6);
			strncpy_s(tag + 2 + node.tag_end - node.node_start - 1, 64, ">", 2);
#else
			strncpy(tag, "</", 2);
			strncpy(tag + 2, m_source_data + node.node_start + 1, 6);
			strncpy(tag + 2 + node.tag_end - node.node_start - 1, ">", 2);
#endif

			size_t matches = 0;

			while (tag[matches] != 0 && m_source_data[offset] != 0){ if (tolower(tag[matches++]) != tolower(m_source_data[offset++])) matches = 0; }

			node.node_end = offset;

			add_node(node.node_start, node.node_end, node.tag_end, parent);

			return offset;
		}

		if (m_source_data[offset] == 0){ return offset; }

		char attribute_quote = 0;
		while (m_source_data[offset])  //while( m_source[offset] && m_source[offset] != '>' ) ++offset;
		{
			if (attribute_quote == 0)
			{
				if (m_source_data[offset] == '=')
				{
					if (m_source_data[offset + 1] == '"' || m_source_data[offset + 1] == '\''){ attribute_quote = m_source_data[++offset]; }
				}
				else if (m_source_data[offset] == '>') break;
			}
			else
			{
				if (m_source_data[offset] == attribute_quote && m_source_data[offset - 1] != '\\'){ attribute_quote = 0; }
			}
			++offset;
		}

		if (m_source_data[offset - 1] == '/')
		{
			node.node_end = ++offset;

			add_node(node.node_start, node.node_end, node.tag_end, parent);

			return offset;
		}
		else
		{
			bool leafTag = false;
			for (int i = 0; i < leafTags_cnt; ++i)
			{
				if ((leafTagsLengths[i] == node.tag_end - node.node_start - 1) && strnicmp(m_source_data + node.node_start + 1, leafTags[i], leafTagsLengths[i]) == 0)
				{
					leafTag = true;
					break;
				}
			}

			if (leafTag) // Listovy tag, pozrieme ako je uzatvoreny
			{
				node.node_end = ++offset;

				size_t tag_close = node.node_end;

				while (!(m_source_data[offset] == '<'))
				{
					++offset;
					if (m_source_data[offset] == 0) return tag_close;
				};

				if (m_source_data[++offset] != '/')
				{
					add_node(node.node_start, node.node_end, node.tag_end, parent);
					return tag_close;
				}

				size_t close_tag_start = ++offset;

				while (m_source_data[offset] && m_source_data[offset] != '>'){ ++offset; }

				if ((node.tag_end - node.node_start - 1 != offset - close_tag_start) || (strnicmp(m_source_data + node.node_start + 1, m_source_data + close_tag_start, offset - close_tag_start) != 0)) // Neuzatvoreny tag
				{
					add_node(node.node_start, node.node_end, node.tag_end, parent);

					return tag_close;
				}
				else // Uzatvoreny zatvaracim tagom
				{
					node.node_end = ++offset;

					add_node(node.node_start, node.node_end, node.tag_end, parent);

					return offset;
				}
			}
			else
			{
				size_t current_node = add_node(node.node_start, node.node_end, node.tag_end, parent);

				size_t subnode_position = ++offset;
				bool closedTag = false;

				while (offset != (subnode_position = parse_node(current_node, subnode_position))){ offset = subnode_position; }; // Prejdeme vsetky deti

				while (!closedTag)
				{
					while (!((m_source_data[offset] == '<') && (m_source_data[offset + 1] == '/')))
					{
						++offset;
						if (m_source_data[offset] == 0){ return offset; }
					}; // a sparsujeme zatvaraci tag

					offset += 2;

					size_t close_tag_start = offset;

					while (m_source_data[offset] && m_source_data[offset] != '>'){ ++offset; }

					if ((node.tag_end - node.node_start - 1 != offset - close_tag_start) || (strnicmp(m_source_data + node.node_start + 1, m_source_data + close_tag_start, offset - close_tag_start) != 0))
					{
						if ((m_nodes[parent].tag_end - m_nodes[parent].node_start - 1 == offset - close_tag_start) && strnicmp(m_source_data + m_nodes[parent].node_start + 1, m_source_data + close_tag_start, offset - close_tag_start) == 0)
						{
							/*printf( "%.*s", offset - close_tag_start, m_source + close_tag_start );
							std::cout << "Je to parentov Tag" << std::endl;*/

							offset = close_tag_start - 3;
							closedTag = true;
						}
						else
						{
							/*std::cout << "Nie je to parentov Tag Preskakujeme: ";
							printf("%.*s\n", offset - close_tag_start, m_source + close_tag_start );*/
						}

						/*std::cout << "ERROR closing tag\n";

						std::cout << "...";

						for( size_t i = close_tag_start - 30; i < offset + 10; ++i )
						{
						if( m_source[i] != '\r' && m_source[i] != '\n' )
						{
						std::cout << m_source[i];
						}
						}
						std::cout << "...\n";*/

					}
					else
					{
						closedTag = true;
					}
				}

				node.node_end = ++offset;

				update_node(current_node, node.node_start, node.node_end, node.tag_end);

				return offset;
			}
		}
	}

	html_document::html_document(const_string source)
	{
		m_document_data = new html_document_data(source);
	}

	bool html_document::is_ok() const
	{
		return true;
	}

	bool html_document::dump(const char *filename/*, html_document_node_list_item & node*/) const
	{
#ifndef _WIN32
		//if( &node != NULL && node.documentNode == NULL ) return false;
		std::cout << m_document_data->m_nodes_cnt << std::endl;


		FILE *dump = fopen(filename, "w");

		if (!dump) return false;

		// Alex cache
		html_document_node_list_item * nodes = m_document_data->m_nodes;
		const char * source_data = m_document_data->m_source_data;

		size_t root = 1;//( &node == NULL ) ? 0 : node.documentNode->node; //TODO: nie tak celkom root ale node od ktoreho sa odvija dump
		size_t currentNode = root;
		size_t level = 1;
		size_t position = 0;

		while (currentNode != nodes[root].parent_node && currentNode != INVALID_NODE)
		{
			int n_chars = fprintf(dump, "%lu", currentNode);

			for (int i = n_chars; i < 10; ++i) {fputc(' ', dump);}
			for (position = 1; position < level; ++position) {
				fputc(' ', dump);
				fputc(' ', dump);
			}

			position = nodes[currentNode].node_start;

			bool whitespace = false;
			char attribute_quote = 0;

			if( position > 10000000 ){ fclose(dump); exit(0); }

			while (source_data[position])  //while( m_source[position] != '>' && m_source[position] != 0 ) fputc(m_source[position++], dump);
			{
				if( position > 10000000 ){ fclose(dump); exit(0); }

				if (attribute_quote == 0) {
					if (source_data[position] == '=') {
						if (source_data[position + 1] == '"' || source_data[position + 1] == '\'') {
							if (source_data[position] == ' ' || source_data[position] == '\r' || source_data[position] == '\n' || source_data[position] == '\t') {
								if (!whitespace) fputc(' ', dump);
								whitespace = true;
							}
							else {
								whitespace = false;
								fputc(source_data[position], dump);
							}

							attribute_quote = source_data[++position];
						}
					}
					else if (source_data[position] == '>') break;
				}
				else {
					if (source_data[position] == attribute_quote && source_data[position - 1] != '\\') {attribute_quote = 0;}
				}

				if (source_data[position] == ' ' || source_data[position] == '\r' || source_data[position] == '\n' || source_data[position] == '\t') {
					if (!whitespace) fputc(' ', dump);
					whitespace = true;
				}
				else {
					whitespace = false;
					fputc(source_data[position], dump);
				}

				++position;
			}

			fputc('>', dump);
			fputc('\n', dump);

			if (nodes[currentNode].first_child != INVALID_NODE) {
				++level;
				currentNode = nodes[currentNode].first_child;
			}
			else if (nodes[currentNode].next_sibling != INVALID_NODE) {
				for (size_t i = 0; i < 10; ++i) {fputc(' ', dump);}
				for (position = 1; position < level; ++position) {
					fputc(' ', dump);
					fputc(' ', dump);
				}

				fputc('<', dump);
				fputc('/', dump);
				position = nodes[currentNode].node_start + 1;
				while (position < nodes[currentNode].tag_end) fputc(source_data[position++], dump);
				fputc('>', dump);
				fputc('\n', dump);

				currentNode = nodes[currentNode].next_sibling;
			}
			else {
				while (currentNode != nodes[root].parent_node && currentNode != INVALID_NODE) {
					for (size_t i = 0; i < 10; ++i) {fputc(' ', dump);}
					for (position = 1; position < level; ++position) {
						fputc(' ', dump);
						fputc(' ', dump);
					}

					fputc('<', dump);
					fputc('/', dump);
					position = nodes[currentNode].node_start + 1;
					while (position < nodes[currentNode].tag_end) fputc(source_data[position++], dump);
					fputc('>', dump);
					fputc('\n', dump);

					currentNode = nodes[currentNode].parent_node;

					if (--level == 0) {
						currentNode = 0;
						break;
					}

					if (nodes[currentNode].next_sibling != INVALID_NODE) {
						currentNode = nodes[currentNode].next_sibling;
						break;
					}
				}
			}
		}

		fclose(dump);
#endif // !_WIN32

		return true;
	}

	bool html_document::save(const char *filename/*, html_document_node_list_item & node*/) const
	{
#ifndef _WIN32



		if( FILE * file = fopen (filename,"w") )
		{
			fwrite(m_document_data->m_source_data, 1, m_document_data->m_source_length, file);
			fclose(file);

			return true;
		}
#endif // !_WIN32
		return false;
	}

	html_document html::parse(const_string source)
	{
		return html_document(source);
	}

	const html_document_node html_document::query_selector(std::string selector) const
	{
		return html_document_node(*this, 1).query_selector(selector);
	}
    
    const html_document_node_list html_document::query_selector_all(std::string selector) const
    {
        return html_document_node(*this, 1).query_selector_all(selector);
    }

	const html_document_node html_document_node::parent_node() const
	{
		return html_document_node(m_document, (m_document_node != INVALID_NODE) ? m_document.m_document_data->m_nodes[m_document_node].parent_node : INVALID_NODE);
	}

	const html_document_node html_document_node::previous_sibling() const
	{
		return html_document_node(m_document, (m_document_node != INVALID_NODE) ? m_document.m_document_data->m_nodes[m_document_node].previous_sibling : INVALID_NODE);
	}

	const html_document_node html_document_node::next_sibling() const
	{
		return html_document_node(m_document, (m_document_node != INVALID_NODE) ? m_document.m_document_data->m_nodes[m_document_node].next_sibling : INVALID_NODE);
	}

	const html_document_node html_document_node::first_child() const
	{
		return html_document_node(m_document, (m_document_node != INVALID_NODE) ? m_document.m_document_data->m_nodes[m_document_node].first_child : INVALID_NODE);
	}

	const html_document_node html_document_node::last_child() const
	{
		return html_document_node(m_document, (m_document_node != INVALID_NODE) ? m_document.m_document_data->m_nodes[m_document_node].last_child : INVALID_NODE);
	}

	const html_document_node html_document_node::nth_child(size_t n) const
	{
		if (m_document.m_document_data->m_nodes[m_document_node].first_child == INVALID_NODE) return html_document_node(m_document, INVALID_NODE);

		size_t actual_node = m_document.m_document_data->m_nodes[m_document_node].first_child;

		for (size_t i = 1; i < n; i++)
		{
			if (actual_node >= m_document.m_document_data->m_nodes_cnt || actual_node == INVALID_NODE) return html_document_node(m_document, INVALID_NODE);
			if (m_document.m_document_data->m_nodes[actual_node].next_sibling == INVALID_NODE) return html_document_node(m_document, INVALID_NODE);

			actual_node = m_document.m_document_data->m_nodes[actual_node].next_sibling;
		}

		return html_document_node(m_document, actual_node);
	}

	const html_document_node html_document_node::query_selector(std::string selector) const
	{
		if (!m_document.m_document_data->is_ok()) return html_document_node(m_document, INVALID_NODE);

		html_selector q_selector(selector);

		size_t current_node = m_document_node;
		size_t level = 1;

		html_selector_match selector_match;
		selector_match.matched = false;
		selector_match.matched_nodes = 0;

		size_t matched_nodes_size = 256;
		size_t * matched_nodes = (size_t *)malloc(matched_nodes_size * sizeof(size_t));

		matched_nodes[0] = 0;

		while (current_node != INVALID_NODE)
		{
			selector_match = q_selector.match(m_document.m_document_data->m_source_data, m_document.m_document_data->m_nodes[current_node], matched_nodes[level - 1]);

			if (selector_match.matched)
			{
				free(matched_nodes);

				return html_document_node(m_document, current_node);
			}

			if (m_document.m_document_data->m_nodes[current_node].first_child != INVALID_NODE)
			{
				matched_nodes[level++] = selector_match.matched_nodes;
				current_node = m_document.m_document_data->m_nodes[current_node].first_child;

				//std::cout << " -> first child" << std::endl;
			}
			else if (m_document.m_document_data->m_nodes[current_node].next_sibling != INVALID_NODE)
			{
				current_node = m_document.m_document_data->m_nodes[current_node].next_sibling;

				//std::cout << " -> next sibling" << std::endl;
			}
			else
			{
				while (current_node != INVALID_NODE)
				{
					current_node = m_document.m_document_data->m_nodes[current_node].parent_node;

					//std::cout << " <- parent" << std::endl;

					if (--level == 0){ current_node = INVALID_NODE; break; }

					if (m_document.m_document_data->m_nodes[current_node].next_sibling != INVALID_NODE)
					{
						current_node = m_document.m_document_data->m_nodes[current_node].next_sibling;
						break;
					}
				}
			}
		}

		free(matched_nodes);

		return html_document_node(m_document, INVALID_NODE);
	}

	const html_document_node html_document_node::nth_child_selector(size_t n, std::string selector) const
	{
		html_selector q_selector(selector);
		size_t actual_node = m_document_node;
		size_t matched = 0;

		while (true)
		{
			if (q_selector.match(m_document.m_document_data->m_source_data, m_document.m_document_data->m_nodes[actual_node]).matched)
			{
				++matched;
				if (matched == n) return html_document_node(m_document, actual_node);
			}

			if (m_document.m_document_data->m_nodes[actual_node].next_sibling == INVALID_NODE) break;

			actual_node = m_document.m_document_data->m_nodes[actual_node].next_sibling;
		}

		return html_document_node(m_document, INVALID_NODE);
	}

	const html_document_node_list html_document_node::child_nodes() const
	{
		std::vector<html_document_node> m_children;

		if (m_document_node == INVALID_NODE){ return html_document_node_list(m_children); }

		size_t actual_node = m_document.m_document_data->m_nodes[m_document_node].first_child;

		while (actual_node != INVALID_NODE)
		{
			m_children.push_back(html_document_node(m_document, actual_node));

			actual_node = m_document.m_document_data->m_nodes[actual_node].next_sibling;
		}

		return html_document_node_list(m_children);
	}

	const html_document_node_list html_document_node::query_selector_all(std::string selector) const
	{
		html_document_node_list result;

		//TODO if( !m_document.m_document_data->is_ok() ){ return result; }

		html_selector q_selector(selector);

		size_t current_node = m_document_node;
		size_t level = 1;

		html_selector_match selector_match;
		selector_match.matched = false;
		selector_match.matched_nodes = 0;

		size_t matched_nodes_size = 2560;
		size_t * matched_nodes = (size_t *)malloc(matched_nodes_size * sizeof(size_t));

		matched_nodes[0] = 0;

		while (current_node != INVALID_NODE)
		{
			selector_match = q_selector.match(m_document.m_document_data->m_source_data, m_document.m_document_data->m_nodes[current_node], matched_nodes[level - 1]);

			if (selector_match.matched)
			{
				result.m_document_node_list.push_back(html_document_node(m_document, current_node));
			}

			if (m_document.m_document_data->m_nodes[current_node].first_child != INVALID_NODE)
			{
				matched_nodes[level++] = selector_match.matched_nodes;
				current_node = m_document.m_document_data->m_nodes[current_node].first_child;
			}
			else if (m_document.m_document_data->m_nodes[current_node].next_sibling != INVALID_NODE)
			{
				current_node = m_document.m_document_data->m_nodes[current_node].next_sibling;
			}
			else
			{
				while (current_node != INVALID_NODE)
				{
					current_node = m_document.m_document_data->m_nodes[current_node].parent_node;

					if (--level == 0){ current_node = INVALID_NODE; break; }

					if (m_document.m_document_data->m_nodes[current_node].next_sibling != INVALID_NODE)
					{
						current_node = m_document.m_document_data->m_nodes[current_node].next_sibling;
						break;
					}
				}
			}
		}

		free(matched_nodes);

		return result;
	}

	const std::string html_document_node::tag() const
	{
		return (m_document_node != INVALID_NODE) ? std::string(m_document.m_document_data->m_source_data + m_document.m_document_data->m_nodes[m_document_node].node_start + 1, m_document.m_document_data->m_nodes[m_document_node].tag_end - m_document.m_document_data->m_nodes[m_document_node].node_start - 1) : "";
	}

	bool html_document_node::has_attribute(std::string attribute) const
	{
		if (m_document_node == INVALID_NODE) return false;

		size_t position = m_document.m_document_data->m_nodes[m_document_node].node_start;
		char quote = 0;

		//ALEX cache
		const char * source = m_document.m_document_data->m_source_data;

		while (source[position] != '>')
		{
			while ((source[position] != '=') && (source[position] != '>') && (source[position] != ' ') && (source[position] != '\r') && (source[position] != '\n') && (source[position] != '\t') && (source[position] != '/')) ++position;

			if (source[position] == '>' || source[position] == '/') return false;

			size_t attr_pos = position;
			while (source[attr_pos] != ' ' && source[attr_pos] != '\r' && source[attr_pos] != '\n' && source[attr_pos] != '\t') --attr_pos;
			++attr_pos;

			if (strnicmp(source + position - attribute.length(), attribute.c_str(), attribute.length()) == 0)
			{
				return true;
			}
			else
			{
				if (source[position + 1] == '\'' || source[position + 1] == '"')
				{
					quote = source[position + 1];
					position += 2;
				}
				else
				{
					quote = ' ';
					position += 1;
				}

				while ((source[position] != 0) && ((source[position] != quote) || (source[position] == quote && source[position] == '\\')) && (quote != ' ' || source[position] != '>')) ++position;

			}
			++position;
		}

		return false;
	}

	const std::string html_document_node::attribute(std::string attribute) const
	{
		if (m_document_node == INVALID_NODE) return "";

		size_t position = m_document.m_document_data->m_nodes[m_document_node].node_start;
		char quote = 0;

		//ALEX cache
		const char * source = m_document.m_document_data->m_source_data;

		while (source[position] != '>')
		{
			while ((source[position] != '=') && (source[position] != '>')) ++position;

			if (source[position] == '>' || source[position] == '/') return "";

			size_t attr_pos = position;
			while (source[attr_pos] != ' ' && source[attr_pos] != '\r' && source[attr_pos] != '\n' && source[attr_pos] != '\t') --attr_pos;
			++attr_pos;

			if (strnicmp(source + attr_pos, attribute.c_str(), attribute.length()) == 0)
			{
				if (source[position + 1] == '\'' || source[position + 1] == '"')
				{
					quote = source[position + 1];
					position += 2;
				}
				else
				{
					quote = ' ';
					position += 1;
				}
				size_t start = position;

				while ((source[position] != 0) && ((source[position] != quote) || (source[position] == quote && source[position] == '\\')) && (quote != ' ' || source[position] != '>')) ++position;

				char * htmlentities = (char *)malloc((position - start + 1) * sizeof(char));
				memcpy(htmlentities, source + start, position - start);
				htmlentities[position - start] = 0;

				decode_html_entities_utf8(htmlentities, 0);

				attribute = htmlentities;
				free(htmlentities);

				return attribute;
			}
			else
			{
				if (source[position + 1] == '\'' || source[position + 1] == '"')
				{
					quote = source[position + 1];
					position += 2;
				}
				else
				{
					quote = ' ';
					position += 1;
				}

				while ((source[position] != 0) && ((source[position] != quote) || (source[position] == quote && source[position] == '\\')) && (quote != ' ' || source[position] != '>')) ++position;

			}
			++position;
		}

		return "";
	}

	const bool html_document_node::has_class(std::string class_name) const
	{
		if (m_document_node == INVALID_NODE || m_document.m_document_data->m_nodes[m_document_node].node_end == INVALID_OFFSET){ return false; }

		if (attribute("class").find(class_name) != std::string::npos)
		{
			return true;
		}

		return false;
	}

	const std::string html_document_node::outer_text() const
	{
		return (m_document_node != INVALID_NODE) ? std::string(m_document.m_document_data->m_source_data + m_document.m_document_data->m_nodes[m_document_node].node_start, m_document.m_document_data->m_nodes[m_document_node].node_end - m_document.m_document_data->m_nodes[m_document_node].node_start) : "";
	}

	const std::string html_document_node::inner_text() const
	{
		if (m_document_node == INVALID_NODE){ return ""; }

		size_t end_position = m_document.m_document_data->m_nodes[m_document_node].node_end;

		if (end_position == INVALID_NODE){ return ""; }

		//ALEX cache
		const char * source = m_document.m_document_data->m_source_data;

		if (source[end_position - 2] == '/'){ return ""; }

		size_t start_position = m_document.m_document_data->m_nodes[m_document_node].node_start;
		while (source[start_position++] != '>');

		while (source[--end_position] != '<');

		if (end_position < start_position) return "";

		return std::string(source + start_position, end_position - start_position);
	}

	const std::string html_document_node::plain_text() const
	{
		if (m_document_node == INVALID_NODE){ return ""; }

		std::string value;
		bool in_tag = false;
		size_t position = m_document.m_document_data->m_nodes[m_document_node].node_start;

		const char * newline_tags[] = { "br", "tr", "p" };
		const int newline_tags_lengths[] = { 2, 2, 1 };
		const int newline_tags_cnt = 3;

		//ALEX cache
		const char * source = m_document.m_document_data->m_source_data;

		while (position < m_document.m_document_data->m_nodes[m_document_node].node_end)
		{
			if (source[position] == '<')
			{
				in_tag = true;

				for (int i = 0; i < newline_tags_cnt; ++i)
				{
					size_t start_position = ++position;

					while (source[position] && (((source[position] >= 'a') && (source[position] <= 'z')) || ((source[position] >= 'A') && (source[position] <= 'Z')) || (source[position] == ':') || ((source[position] >= '0') && (source[position] <= '9')))) ++position;
					--position;

					if ((newline_tags_lengths[i] == position + 1 - start_position) && strnicmp(source + start_position, newline_tags[i], newline_tags_lengths[i]) == 0)
					{
						value += "\n";
						break;
					}
				}
			}
			else if (source[position] == '>'){ in_tag = false; value += " "; }
			else if (!in_tag) value += source[position];

			++position;
		}

		return value;
	}

	const html_document_node html_document_node::parent_selector(std::string selector) const
	{
		if (m_document_node == INVALID_NODE){ return html_document_node(m_document, INVALID_NODE); }

		std::vector<size_t> parents;

		size_t parent_node = m_document.m_document_data->m_nodes[m_document_node].parent_node;

		while (parent_node != INVALID_NODE && parent_node < 100000) // TODO hotfix
		{
			parents.push_back(parent_node);

			parent_node = m_document.m_document_data->m_nodes[parent_node].parent_node;
		}

		html_selector q_selector(selector);

		size_t level = 1;

		html_selector_match selector_match;
		selector_match.matched = false;
		selector_match.matched_nodes = 0;

		for (std::vector<size_t>::iterator current_node = parents.begin(); current_node != parents.end(); ++current_node)
		{
			selector_match = q_selector.match(m_document.m_document_data->m_source_data, m_document.m_document_data->m_nodes[*current_node], selector_match.matched_nodes);

			if (selector_match.matched)
			{
				return html_document_node(m_document, *current_node);
			}

			++level;
		}

		return html_document_node(m_document, INVALID_NODE);
	}

	const std::string html_document_node::outer_text_except(html_document_node_list & except_nodes) const
	{
		/*if( m_document_node == INVALID_NODE ){ return ""; }

		std::string result;

		if( except_nodes.m_document_node_list.size() == 0 ){ return outer_text(); }

		std::vector<std::vector<html_document_node>> invalid_nodes;

		for( std::vector<html_document_node>::iterator i = except_nodes.m_document_node_list.begin(); i != except_nodes.m_document_node_list.end(); ++i )
		{
		if( ( m_document.m_document_data->m_nodes[i->m_document_node].node_start < m_document.m_document_data->m_nodes[m_document_node].node_start ) || ( m_document.m_document_data->m_nodes[i->m_document_node].node_end > m_document.m_document_data->m_nodes[m_document_node].node_end ) )
		{
		invalid_nodes.push_back(i);
		}
		}

		for( std::vector<std::vector<html_document_node>>::reverse_iterator i = invalid_nodes.rbegin(); i != invalid_nodes.rend(); ++i )
		{
		except_nodes.m_document_node_list.erase(*i);
		}

		//ALEX cache
		const char * source = m_document.m_document_data->m_source_data;

		result = std::string(source + m_document.m_document_data->m_nodes[m_document_node].node_start, except_nodes.m_document_node_list[0].node_start - m_document.m_document_data->m_nodes[m_document_node].node_start);

		for( size_t i = 0; i < except_nodes.m_document_node_list.size() - 1; ++i )
		{
		result += std::string(source + m_document.m_document_data->m_nodes[except_nodes.m_document_node_list[i]].node_end, m_document.m_document_data->m_nodes[except_nodes.m_document_node_list[i+1]].node_start - m_document.m_document_data->m_nodes[except_nodes[i]].node_end);
		}

		result += std::string(source + m_document.m_document_data->m_nodes[except_nodes.m_document_node_list[except_nodes.m_document_node_list.size()-1]].node_end, m_document.m_document_data->m_nodes[m_document_node].node_end - m_document.m_document_data->m_nodes[except_nodes.m_document_node_list[exceptNodes.m_document_node_list.size()-1]].node_end);

		return result;*/
		return "";
	}

	const std::string html_document_node::inner_text_except(html_document_node_list & except_nodes) const
	{
		/*if( m_document_node == INVALID_NODE ){ return ""; }

		std::string result;

		if( except_nodes.size() == 0 ){ return m_document.m_document_data->m_nodes[m_document_node].inner_text(); }

		if(m_document.m_document_data->m_nodes[m_document_node].node_end == INVALID_NODE){ return ""; }

		std::vector<std::vector<html_document_node>::iterator> invalid_nodes;

		for( std::vector<html_document_node>::iterator i = except_nodes.m_document_node_list.begin(); i != except_nodes.m_document_node_list.end(); ++i )
		{
		if( ( m_document.m_document_data->m_nodes[i->m_document_node].node_start < m_document.m_document_data->m_nodes[m_document_node].node_start ) || ( m_document.m_document_data->m_nodes[i->m_document_node].node_end > m_document.m_document_data->m_nodes[m_document_node].node_end ) )
		{
		invalid_nodes.push_back(i);
		}
		}

		for( std::vector<std::vector<html_document_node>::iterator>::reverse_iterator i = invalid_nodes.rbegin(); i != invalid_nodes.rend(); ++i )
		{
		except_nodes.m_document_node_list.erase(*i);
		}

		//ALEX cache
		const char * source = m_document.m_document_data->m_source_data;

		size_t start_position = m_document.m_document_data->m_nodes[m_document_node].node_start;
		while(source[start_position++] != '>');

		size_t end_position = node.documentNode->node_end;
		while(source[--end_position] != '<');

		result = std::string(source + start_position, except_nodes[0].node_start - start_position);

		for( size_t i = 0; i < except_nodes.size() - 1; ++i )
		{
		if( except_nodes[i+1].node_start < except_nodes[i].node_end ){ continue; }

		result += std::string(source + except_nodes[i].node_end, except_nodes[i+1].node_start - except_nodes[i].node_end);
		}

		if( end_position < except_nodes[exceptNodes.size()-1].node_end ) return "";

		result += std::string(source + except_nodes[except_nodes.size()-1].node_end, end_position - except_nodes[except_nodes.size()-1].node_end);

		return result;*/
		return "";
	}

	const std::string html_document_node::plain_text_except(html_document_node_list & except_nodes) const
	{
		/*if( m_document_node == INVALID_NODE ){ return ""; }

		std::string value;

		if( except_nodes.size() == 0 ){ return m_document.m_document_data->m_nodes[m_document_node].plain_text(); }

		std::vector<html_document_node_list::iterator> invalid_nodes;

		for( html_document_node_list::iterator i = except_nodes.begin(); i != except_nodes.end(); ++i )
		{
		if( ( (*i).node_start < m_document.m_document_data->m_nodes[m_document_node].node_start ) || ( (*i).node_end > m_document.m_document_data->m_nodes[m_document_node].node_end ) )
		{
		invalid_nodes.push_back(i);
		}
		}

		for( std::vector<html_document_node_list::iterator>::reverse_iterator i = invalid_nodes.rbegin(); i != invalid_nodes.rend(); ++i )
		{
		except_nodes.erase(*i);
		}

		bool in_tag = false;
		size_t position = m_document.m_document_data->m_nodes[m_document_node].node_start;

		size_t actual_node = 0;

		//ALEX cache
		const char * source = m_document.m_document_data->m_source_data;

		while( position < m_document.m_document_data->m_nodes[m_document_node].node_end )
		{
		if( ( except_nodes[actual_node].node_start <= position ) && ( position < except_nodes[actual_node].node_end ) );
		else if( source[position] == '<' ){ in_tag = true; }
		else if( source[position] == '>' ){ in_tag = false; value += " "; }
		else if(!in_tag){ value += source[position]; }

		if( position > except_nodes[actual_node].node_end ){ actual_node = ( actual_node + 1 < except_nodes.size() ) ? actual_node + 1 : except_nodes.size() - 1; }

		++position;
		}

		return value;*/
		return "";
	}

	const std::string html_document_node::unique_hash() const
	{
		char hash[64];
#ifdef _WIN32
		sprintf_s(hash, "%p:%ld", m_document.m_document_data, m_document_node);
#else
		sprintf(hash, "%p:%ld", m_document.m_document_data, m_document_node);
#endif // _WIN32


		return hash;
	}

	bool html_document_node::exists() const
	{
		return (m_document_node != INVALID_NODE);
	}

	html_document_node_list::html_document_node_list(std::vector<html_document_node> node_list) : m_document_node_list(node_list)
	{
	}

}
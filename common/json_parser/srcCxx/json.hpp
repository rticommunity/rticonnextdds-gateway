/*
 * (c) Copyright, Real-Time Innovations, 2015.
 * All rights reserved.
 *
 * No duplications, whole or partial, manual or electronic, may be made
 * without express written permission.  Any such copies, or
 * revisions thereof, must display this notice unaltered.
 * This code contains trade secrets of Real-Time Innovations, Inc.
 */

#ifndef JSON_HPP
#define JSON_HPP

#include <cstring>

#include "json.h"

#define STRING_BUFFER_LENGTH 1000
#define ERR_MSG_PARSE_JSON "Error parsing JSON request"

namespace rti { namespace json {
/**
 * This class wraps an _json_value object providing to handle its memory
 * using a constructor and a destructor. It also provides some extra
 * functionality such as find_node, which returns the first node with a
 * certain name.
 */
class json_document {
public:
    /**
     * Constructor of the json_ducument class, initializes the wrapped
     * json_value to NULL.
     */
    json_document() : _json_value(NULL) {}
    
    /**
     * Destructor of the json_ducument class. It releases memory allocated for
     * the json_value.
     */
    ~json_document()
    {
        if (_json_value != NULL) {
            json_value_free(_json_value);
            _json_value = NULL;
        }
    }
    
    /**
     * This method parses a JSON string and returns a pointer to the root
     * node of the JSON tree.
     * @param json JSON string to be parsed.
     * @return Pointer to the root of parsed JSON tree. If there are errors
     * parsing the JSON string, the method returns NULL.
     */
    void parse(const char *json)
    {
        parse(json, strlen(json));
    }
    
    /**
     * This method parses a JSON string and returns a pointer to the root
     * node of the JSON tree.
     * @param json JSON string to be parsed.
     * @param length Length of the JSON string to be parsed.
     * @return If the method fails to parse it throws an exception.
     */
    void parse(const char *json, size_t length)
    {
        const char *method_name = "rti::webdds::json_document::parse";
        char error[STRING_BUFFER_LENGTH] = "There was something here";
        json_settings settings = { 0 };
        _json_value = json_parse_ex(&settings, json, length, error);
        
        //_json_value = json_parse(json, length);
        
        if (_json_value == NULL) {
            throw std::runtime_error(ERR_MSG_PARSE_JSON);
        }
    }
    
    /**
     * This method looks for a node within the JSON node tree with the given
     * name.
     * @param node_name Name of the node to be located.
     * @return Pointer to the JSON node with the given name.
     */
    json_value * find_node(const char * node_name)
    {
        return find_node(_json_value, node_name);
    }
    
    /**
     * This method returns the root of the json document.
     * @return Pointer to the root of the JSON document. If the JSON document
     * has not been initialized (i.e., we have not parsed anything yet) the
     * method returns NULL.
     */
    json_value * first_node()
    {
        return _json_value;
    }
    
private:
    /**
     * Root of the JSON document.
     */
    json_value * _json_value;

    
    /**
     * This method looks for a node within the JSON node tree with the given
     * name.
     * @param parent_node Pointer to the parent node.
     * @param node_name Name of the node to be located.
     * @return Pointer to the JSON node with the given name.
     */
    json_value * find_node(json_value * parent_node, const char * node_name)
    {
        json_value * node = NULL;
        
        if (parent_node == NULL) {
            return node;
        }

        switch (parent_node->type) {
            case json_object:
            {
                size_t length = parent_node->u.object.length;
                for (unsigned int i = 0; i < length; i++) {
                    if (strcmp(parent_node->u.object.values[i].name,
                               node_name) == 0) {
                        node = parent_node->u.object.values[i].value;
                        return node;
                    }
                    node = find_node(parent_node->u.object.values[i].value,
                                     node_name);
                    if (node != NULL) {
                        return node;
                    };
                }
            } break;
            case json_array:
            {
                size_t length = parent_node->u.array.length;
                for (unsigned int i = 0; i < length; i++) {
                    node = find_node(parent_node->u.array.values[i],
                                     node_name);
                    if (node != NULL) {
                        return node;
                    };
                }
            }
            default:
            {
                // NOOP
            }
        }
        
        return node;
        
    }
};
    
}}

#endif //JSON_HPP

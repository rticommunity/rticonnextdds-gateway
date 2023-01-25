/******************************************************************************/
/* (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
/*                                                                            */
/* RTI grants Licensee a license to use, modify, compile, and create          */
/* derivative works of the software solely for use with RTI Connext DDS.      */
/* Licensee may redistribute copies of the software provided that all such    */
/* copies are subject to this license.                                        */
/* The software is provided "as is", with no warranty of any type, including  */
/* any warranty for fitness for any purpose. RTI is under no obligation to    */
/* maintain or support the software.  RTI shall not be liable for any         */
/* incidental or consequential damages arising out of the use or inability to */
/* use the software.                                                          */
/*                                                                            */
/******************************************************************************/

#ifndef UTILSSTRINGS_HPP
#define UTILSSTRINGS_HPP

#include <string>
#include <vector>

namespace rti { namespace utils { namespace strings {

/**
 * @brief Splits a string separated by 'delim' characters.
 * @param input the input string that contains several words separated by
 * 'delim' characters.
 * @param delim the character that delimits the words inside 'input'
 * @return A vector of strings that contain all the words delimited by 'delim'
 * as elements. For example: 'hello.world' will return ["hello", "world"].
 */

inline std::vector<std::string> split(const std::string &input, char delim)
{
    std::vector<std::string> output;
    size_t prev_position = 0;

    while (prev_position != std::string::npos) {
        size_t length = input.find(delim, prev_position);

        if (length != std::string::npos) {
            // If the find hasn't reached the end of the string, calculate
            // the length of the next element, and copy it to the output vector
            size_t word_length = length - prev_position;
            std::string temp_string(input, prev_position, word_length);
            output.push_back(temp_string);
            // update the previous position by adding with word_length + 1,
            // the + 1 will 'delete' the following 'delim' from the input
            // "type1.type2" --> word_length will be 6, but in the second
            // iteration we want to start searching in the
            // 7th position: "type2" instead of the 6th position: ".type2"
            prev_position += word_length + 1;
        } else {
            // If the current element that is processed is the last element
            // in name, get it and
            std::string temp_string(input, prev_position);
            output.push_back(temp_string);
            prev_position = std::string::npos;
        }
    }
    return output;
}
}}}  // namespace rti::utils::strings

#endif

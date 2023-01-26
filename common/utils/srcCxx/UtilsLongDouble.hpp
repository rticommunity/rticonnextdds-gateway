/******************************************************************************/
/* (c) 2023 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#ifndef UTILSLONGDOUBLE_HPP
#define UTILSLONGDOUBLE_HPP

#include <cstdint>
#include <iostream>

namespace rti { namespace utils { namespace long_double {

/**
 * @brief These safe_cast functions ensure that the long double is correctly
 * casted to an unsigned value. In some architectures, the casting from a
 * floating number value to a unsigned value is not defined if the value doesn't
 * fit in the unsigned value. For example:
 *     long double x = -1.0;
 *     uint8_t y = static_cast<uint8_t>(x); --> this behavior is not defined,
 *                                  some architectures return 0, instead of 255.
 * The solution is to cast 'x' to an signed int8_t and then to a uint8_y.
 *     uint8_t y = static_cast<uint8_t>(static_cast<int8_t>(x); --> returns 255
 * However, in this case, values that are between INT8_MAX and UINT8_MAX cannot
 * be represented. Therefore, we need a function that check if the number
 * is negative or do a different thing otherwise.
 *
 * These functions also throw a warning in case there is overflow. The overflow
 * happens in these situations:
 *   - Signed integer: value < INTx_MIN || value > INTx_MAX
 *   - Unsigned integer: value > UINTx_MAX || value < INTx_MIN
 * The last condition overflows when doing the first casting to signed integer
 * from a negative value. The value cannot be casted to a number that is less
 * than the minimum integer number of the same size.
 * @param value The long double value that will be casted.
 * @return The value casted safely to the specified size.
 */

template<typename T>
inline T safe_cast(long double value) {
    return static_cast<T>(value);
}

template<>
inline uint8_t safe_cast(long double value)
{
    if (value > UINT8_MAX || value < INT8_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to uint8_t. Potential loss of information.";
    }
    if (value < 0) {
        return static_cast<uint8_t>(static_cast<int8_t>(value));
    } else {
        return static_cast<uint8_t>(value);
    }
}

template<>
inline uint16_t safe_cast(long double value)
{
    if (value > UINT16_MAX || value < INT16_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to uint16_t. Potential loss of information.";
    }
    if (value < 0) {
        return static_cast<uint16_t>(static_cast<int16_t>(value));
    } else {
        return static_cast<uint16_t>(value);
    }
}

template<>
inline uint32_t safe_cast(long double value)
{
    if (value > UINT32_MAX || value < INT32_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to uint32_t. Potential loss of information.";
    }
    if (value < 0) {
        return static_cast<uint32_t>(static_cast<int32_t>(value));
    } else {
        return static_cast<uint32_t>(value);
    }
}

template<>
inline uint64_t safe_cast(long double value)
{
    if (value > UINT64_MAX || value < INT64_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to uint64_t. Potential loss of information.";
    }
    if (value < 0) {
        return static_cast<uint64_t>(static_cast<int64_t>(value));
    } else {
        return static_cast<uint64_t>(value);
    }
}

template<>
inline char safe_cast(long double value)
{
    if (value > CHAR_MAX || value < CHAR_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to char. Potential loss of information.";
    }
    return static_cast<char>(value);
}

template<>
inline int8_t safe_cast(long double value)
{
    if (value > INT8_MAX || value < INT8_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to int8_t. Potential loss of information.";
    }
    return static_cast<int8_t>(value);
}

template<>
inline int16_t safe_cast(long double value)
{
    if (value > INT16_MAX || value < INT16_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to int16_t. Potential loss of information.";
    }
    return static_cast<int16_t>(value);
}

template<>
inline int32_t safe_cast(long double value)
{
    if (value > INT32_MAX || value < INT32_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to int32_t. Potential loss of information.";
    }
    return static_cast<int32_t>(value);
}

template<>
inline int64_t safe_cast(long double value)
{
    if (value > INT64_MAX || value < INT64_MIN) {
        std::cerr << "Warning: overflow casting value <"
                  << std::to_string(value)
                  << "> to int64_t. Potential loss of information.";
    }
    return static_cast<int64_t>(value);
}

template<>
inline bool safe_cast(long double value)
{
    return static_cast<bool>(value != 0 ? 1 : 0);
}


}}}  // namespace rti::utils::long_double

#endif

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

#ifndef UniversalPrimitiveType_h
#define UniversalPrimitiveType_h

#include <cstdint>
#include <dds/core/xtypes/TypeKind.hpp>

using namespace dds::core::xtypes;

namespace rti { namespace utils { namespace types {

class UniversalPrimitiveType {

    public:
        UniversalPrimitiveType() {
            u = {0};
        }

        ~UniversalPrimitiveType(){}

        /* Getters of the value of the Union u. Depending on the type of K,
         * the getter returns a double, a int64_t or a uint64_t.
         * Example of use:
         *   UniversalPrimitiveType u;
         *   u.value<FLOAT_64_TYPE>(); // this returns u.double_value
         */
        template <
                TypeKind::type K,
                typename std::enable_if<
                        K == TypeKind::FLOAT_64_TYPE
                        || K == TypeKind::FLOAT_32_TYPE>::type* = nullptr>
        double value()
        {
            return u.double_value;
        }

        template <
                TypeKind::type K,
                typename std::enable_if<
                        K == TypeKind::CHAR_8_TYPE
                        || K == TypeKind::INT_16_TYPE
                        || K == TypeKind::INT_32_TYPE
                        || K == TypeKind::ENUMERATION_TYPE
                        || K == TypeKind::INT_64_TYPE>::type* = nullptr>
        int64_t value()
        {
            return u.int64_value;
        }

        template <
                TypeKind::type K,
                typename std::enable_if<
                        K == TypeKind::UINT_8_TYPE
                        || K == TypeKind::BOOLEAN_TYPE
                        || K == TypeKind::UINT_16_TYPE
                        || K == TypeKind::UINT_32_TYPE
                        || K == TypeKind::UINT_64_TYPE>::type* = nullptr>
        uint64_t value()
        {
            return u.uint64_value;
        }

        /* Setters of the value of the Union u. The type T defines the input
         * value and K to which element of the union it is saved.
         * Example of use:
         *   UniversalPrimitiveType u;
         *   double a = 3.5;
         *   u.value<double,FLOAT_64_TYPE>(a);
         * This says that the value 'a', which is a double, is going to be set
         * to the corresponding value of the union identified by FLOAT_64_TYPE
         * which is u.double_value.
         */
        template <
                typename T,
                TypeKind::type K,
                typename std::enable_if<
                        K == TypeKind::CHAR_8_TYPE
                        || K == TypeKind::INT_16_TYPE
                        || K == TypeKind::INT_32_TYPE
                        || K == TypeKind::INT_64_TYPE>::type* = nullptr>
        void value(T v) {
            u.int64_value = static_cast<int64_t>(v);
        }

        template <
                typename T,
                TypeKind::type K,
                typename std::enable_if<
                        K == TypeKind::UINT_8_TYPE
                        || K == TypeKind::UINT_16_TYPE
                        || K == TypeKind::UINT_32_TYPE
                        || K == TypeKind::UINT_64_TYPE>::type* = nullptr>
        void value(T v)
        {
            u.uint64_value = static_cast<uint64_t>(v);
        }

        template <
                typename T,
                TypeKind::type K,
                typename std::enable_if<
                        K == TypeKind::FLOAT_64_TYPE
                        || K == TypeKind::FLOAT_32_TYPE>::type* = nullptr>
        void value(T v)
        {
            u.double_value = static_cast<double>(v);
        }

    private:
        typedef union {
            double double_value;
            int64_t int64_value;
            uint64_t uint64_value;
        } UniversalPrimitiveTypeUnion;

        UniversalPrimitiveTypeUnion u;
};

}}} // rti::utils::strings

#endif /* UniversalPrimitiveType_h */

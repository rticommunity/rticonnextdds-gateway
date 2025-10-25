/******************************************************************************/
/* (c) 2025 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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
#ifndef DynamicDataConverter_hpp
#define DynamicDataConverter_hpp

#include <string>

#include "ndds/ndds_c.h"

namespace rti {
namespace dyndata {

class DynamicDataConverter {
public:
    virtual
    bool
    deserialize(
        DDS_DynamicData & data,
        const char * const serialized_data,
        const size_t serialized_data_len) = 0;

    virtual
    bool
    serialize(
        DDS_DynamicData & data,
        std::string & serialized_data) = 0;
};


} // namespace dyndata
} // namespace rti

#endif // DynamicDataConverter_hpp

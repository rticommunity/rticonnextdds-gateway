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

#ifndef rtiprocess_fwd_properties_hpp
#define rtiprocess_fwd_properties_hpp

#include <rtiprocess_fwd_platform.hpp>

namespace rti { namespace prcs { namespace fwd { namespace property {

extern const std::string PREFIX;
extern const std::string FORWARDING_TABLE;
extern const std::string INPUT_MEMBERS_TABLE;

extern const std::string FORWARDING_TABLE_KEY_IN_KEY;
extern const std::string FORWARDING_TABLE_KEY_OUT_NAME;

extern const std::string INPUT_MEMBERS_TABLE_KEY_IN_KEY;
extern const std::string INPUT_MEMBERS_TABLE_KEY_OUT_NAME;

void parse_config(
        const rti::routing::PropertySet &properties,
        fwd::ByInputNameForwardingEngineConfiguration &config);

void parse_config(
        const rti::routing::PropertySet &properties,
        fwd::ByInputValueForwardingEngineConfiguration &config);

}}}}  // namespace rti::prcs::fwd::property

#endif /* rtiprocess_fwd_properties_hpp */
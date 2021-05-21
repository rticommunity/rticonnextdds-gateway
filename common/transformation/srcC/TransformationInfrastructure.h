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

#ifndef TransformationInfrastructure_h
#define TransformationInfrastructure_h

#include "ndds/ndds_c.h"

RTIBool RTI_TSFM_DDS_DynamicDataPtr_initialize_w_params(
        DDS_DynamicData **self,
        const struct DDS_TypeAllocationParams_t *allocParams);

RTIBool RTI_TSFM_DDS_DynamicDataPtr_finalize_w_params(
        DDS_DynamicData **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams);

RTIBool RTI_TSFM_DDS_DynamicDataPtr_copy(
        DDS_DynamicData **dst,
        const DDS_DynamicData **src);

DDS_SEQUENCE(RTI_TSFM_DDS_DynamicDataPtrSeq, DDS_DynamicData *);

#endif /* TransformationInfrastructure_h */

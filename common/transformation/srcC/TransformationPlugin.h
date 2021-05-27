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

#ifndef TransformationPlugin_h
#define TransformationPlugin_h

#include "TransformationTypes.h"

#include "ndds/ndds_c.h"

DDS_ReturnCode_t RTI_TSFM_TransformationPluginConfig_default(
        RTI_TSFM_TransformationPluginConfig **config_out);

DDS_ReturnCode_t RTI_TSFM_TransformationPluginConfig_new(
        DDS_Boolean allocate_optional,
        RTI_TSFM_TransformationPluginConfig **config_out);

void RTI_TSFM_TransformationPluginConfig_delete(
        RTI_TSFM_TransformationPluginConfig *self);


#endif /* TransformationPlugin_h */

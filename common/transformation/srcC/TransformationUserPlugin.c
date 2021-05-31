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

#include "TransformationPlatform.h"
#include "TransformationSimple.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::simple::user"

RTI_TSFM_UserTypePlugin *RTI_TSFM_UserTypePlugin_create_static(
        RTI_TSFM_TransformationPlugin *transform_plugin,
        RTI_TSFM_UserTypePlugin_SerializeSampleFn serialize_sample,
        RTI_TSFM_UserTypePlugin_DeserializeSampleFn deserialize_sample)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    RTI_TSFM_UserTypePlugin *self = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_UserTypePlugin_create_static)

    self = (RTI_TSFM_UserTypePlugin *) RTI_TSFM_Heap_allocate(
            sizeof(RTI_TSFM_UserTypePlugin));
    if (self == NULL) {
        goto done;
    }

    RTI_TSFM_UserTypePlugin_initialize(self);

    self->serialize_sample = serialize_sample;
    self->deserialize_sample = deserialize_sample;
    self->delete_plugin = RTI_TSFM_UserTypePlugin_delete_default;

    retval = DDS_BOOLEAN_TRUE;

done:
    if (!retval) {
        if (self != NULL) {
            RTI_TSFM_UserTypePlugin_delete_default(self, transform_plugin);
            self = NULL;
        }
    }

    return self;
}

RTI_TSFM_UserTypePlugin *RTI_TSFM_UserTypePlugin_create_dynamic(
        RTI_TSFM_TransformationPlugin *transform_plugin,
        const char *plugin_lib,
        const char *plugin_create_fn)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_UserTypePlugin_create_dynamic)
    /* TODO implement me */
    return NULL;
}

void RTI_TSFM_UserTypePlugin_delete_default(
        RTI_TSFM_UserTypePlugin *self,
        RTI_TSFM_TransformationPlugin *transform_plugin)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_UserTypePlugin_delete_default)
    RTI_TSFM_Heap_free(self);
}

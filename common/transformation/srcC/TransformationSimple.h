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

#ifndef Transformation_Simple_h
#define Transformation_Simple_h

#include "ndds/ndds_c.h"
#include "routingservice/routingservice_transformation.h"

#include "TransformationInfrastructure.h"
#include "TransformationLog.h"
#include "TransformationTypes.h"

/*****************************************************************************
 *                         Library Version
 *****************************************************************************/

#define RTI_TSFM_TRANSFORMATION_VERSION_MAJOR 1
#define RTI_TSFM_TRANSFORMATION_VERSION_MINOR 0
#define RTI_TSFM_TRANSFORMATION_VERSION_RELEASE 0
#define RTI_TSFM_TRANSFORMATION_VERSION_REVISION 0

/*****************************************************************************
 *                         Configuration Properties
 *****************************************************************************/
#define RTI_TSFM_PLUGIN_PROPERTY_PREFIX ""

#define RTI_TSFM_TRANSFORMATION_PROPERTY_PREFIX ""

#define RTI_TSFM_PROPERTY_PLUGIN_DLL RTI_TSFM_PLUGIN_PROPERTY_PREFIX "dll"

#define RTI_TSFM_PROPERTY_CREATE_FN RTI_TSFM_PLUGIN_PROPERTY_PREFIX "create_fn"

#define RTI_TSFM_PROPERTY_TRANSFORMATION_TYPE \
    RTI_TSFM_TRANSFORMATION_PROPERTY_PREFIX "transform_type"

#define RTI_TSFM_PROPERTY_TRANSFORMATION_INPUT_TYPE \
    RTI_TSFM_TRANSFORMATION_PROPERTY_PREFIX "input_type"

#define RTI_TSFM_PROPERTY_TRANSFORMATION_OUTPUT_TYPE \
    RTI_TSFM_TRANSFORMATION_PROPERTY_PREFIX "output_type"

/*****************************************************************************
 *                        User Type Plugin Class
 *****************************************************************************/

struct RTI_TSFM_TransformationImpl;
struct RTI_TSFM_TransformationPluginImpl;
struct RTI_TSFM_UserTypePluginImpl;

typedef struct RTI_TSFM_UserTypePluginImpl *(*RTI_TSFM_UserTypePlugin_CreateFn)(
        struct RTI_TSFM_UserTypePluginImpl *plugin,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

typedef DDS_ReturnCode_t (*RTI_TSFM_UserTypePlugin_SerializeSampleFn)(
        struct RTI_TSFM_UserTypePluginImpl *plugin,
        struct RTI_TSFM_TransformationImpl *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out);

typedef DDS_ReturnCode_t (*RTI_TSFM_UserTypePlugin_DeserializeSampleFn)(
        struct RTI_TSFM_UserTypePluginImpl *plugin,
        struct RTI_TSFM_TransformationImpl *transform,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out);

typedef void (*RTI_TSFM_UserTypePlugin_DeletePluginFn)(
        struct RTI_TSFM_UserTypePluginImpl *self,
        struct RTI_TSFM_TransformationPluginImpl *plugin);

typedef struct RTI_TSFM_UserTypePluginImpl {
    RTI_TSFM_UserTypePlugin_DeletePluginFn delete_plugin;
    RTI_TSFM_UserTypePlugin_SerializeSampleFn serialize_sample;
    RTI_TSFM_UserTypePlugin_DeserializeSampleFn deserialize_sample;
    void *user_object;
} RTI_TSFM_UserTypePlugin;

#define RTI_TSFM_UserTypePlugin_initialize(p_) \
    {                                          \
        (p_)->serialize_sample = NULL;         \
        (p_)->deserialize_sample = NULL;       \
        (p_)->user_object = NULL;              \
    }

RTI_TSFM_UserTypePlugin *RTI_TSFM_UserTypePlugin_create_static(
        struct RTI_TSFM_TransformationPluginImpl *transform_plugin,
        RTI_TSFM_UserTypePlugin_SerializeSampleFn serialize_sample,
        RTI_TSFM_UserTypePlugin_DeserializeSampleFn deserialize_sample);

void RTI_TSFM_UserTypePlugin_delete_default(
        RTI_TSFM_UserTypePlugin *self,
        struct RTI_TSFM_TransformationPluginImpl *transform_plugin);

/*****************************************************************************
 *                       Base Transformation Class
 *****************************************************************************/
#define RTI_TSFM_TransformationConfig_INITIALIZER                 \
    {                                                             \
        RTI_TSFM_TransformationKind_SERIALIZER, /* type */        \
                "",                             /* input_type */  \
                ""                              /* output_type */ \
    }

DDS_ReturnCode_t RTI_TSFM_TransformationConfig_parse_from_properties(
        RTI_TSFM_TransformationConfig *config,
        const struct RTI_RoutingServiceProperties *properties);

typedef struct RTI_TSFM_TransformationImpl {
    RTI_TSFM_TransformationConfig *config;
    struct RTI_TSFM_TransformationPluginImpl *plugin;
    struct DDS_DynamicDataTypeSupport *tsupport;
    struct RTI_TSFM_DDS_DynamicDataPtrSeq read_buffer;
    DDS_Boolean read_buffer_loaned;
#if RTI_TSFM_USE_MUTEX
    RTI_TSFM_Mutex lock;
#endif /* RTI_TSFM_USE_MUTEX */
} RTI_TSFM_Transformation;


DDS_ReturnCode_t RTI_TSFM_Transformation_initialize(
        RTI_TSFM_Transformation *self,
        struct RTI_TSFM_TransformationPluginImpl *plugin,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

DDS_ReturnCode_t
        RTI_TSFM_Transformation_finalize(RTI_TSFM_Transformation *self);

DDS_ReturnCode_t RTI_TSFM_Transformation_transform(
        RTI_TSFM_Transformation *self,
        RTI_RoutingServiceSample **out_sample_lst,
        RTI_RoutingServiceSampleInfo **out_info_lst,
        int *out_count,
        RTI_RoutingServiceSample *in_sample_lst,
        RTI_RoutingServiceSampleInfo *in_info_lst,
        int in_count,
        RTI_RoutingServiceEnvironment *env);

void RTI_TSFM_Transformation_return_loan(
        RTI_TSFM_Transformation *self,
        RTI_RoutingServiceSample *sample_lst,
        RTI_RoutingServiceSampleInfo *info_lst,
        int count,
        RTI_RoutingServiceEnvironment *env);

DDS_ReturnCode_t RTI_TSFM_Transformation_update(
        RTI_RoutingServiceTransformation transformation,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);


DDS_SEQUENCE(RTI_TSFM_TransformationPtrSeq, RTI_TSFM_Transformation *);

/*****************************************************************************
 *                      Transformation Plugin Class
 *****************************************************************************/
#define RTI_TSFM_TransformationPluginConfig_INITIALIZER \
    {                                                   \
        "", /* dll */                                   \
        "" /* create_fn */                              \
    }

DDS_ReturnCode_t RTI_TSFM_TransformationPluginConfig_parse_from_properties(
        RTI_TSFM_TransformationPluginConfig *config,
        const struct RTI_RoutingServiceProperties *properties);


typedef struct RTI_TSFM_TransformationPluginImpl {
    struct RTI_RoutingServiceTransformationPlugin parent;
    struct RTI_TSFM_TransformationPtrSeq transforms;
    RTI_TSFM_TransformationPluginConfig *config;
    RTI_TSFM_UserTypePlugin *user_plugin;
#if RTI_TSFM_USE_MUTEX
    RTI_TSFM_Mutex lock;
#endif /* RTI_TSFM_USE_MUTEX */
} RTI_TSFM_TransformationPlugin;

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_initialize(
        RTI_TSFM_TransformationPlugin *self,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_initialize_static(
        RTI_TSFM_TransformationPlugin *self,
        RTI_TSFM_UserTypePlugin *user_plugin,
        RTI_TSFM_TransformationPluginConfig *config,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_finalize(
        RTI_TSFM_TransformationPlugin *self,
        RTI_RoutingServiceEnvironment *env);

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_initialize_transformation(
        RTI_TSFM_TransformationPlugin *self,
        RTI_TSFM_Transformation *transform,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_finalize_transformation(
        RTI_TSFM_TransformationPlugin *self,
        RTI_TSFM_Transformation *transform,
        RTI_RoutingServiceEnvironment *env);

#include "TransformationLog.h"

#endif /* Transformation_Simple_h */
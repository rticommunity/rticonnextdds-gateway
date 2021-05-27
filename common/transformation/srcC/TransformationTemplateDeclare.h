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

#ifndef concat
    #ifndef xconcat
        #define xconcat(A, B) A##B
    #endif /* xconcat */
    #define concat(A, B) xconcat(A, B)
#endif /* concat */

#ifndef T
    #error "a type T must be defined"
#endif /* T */

#ifndef TImpl
    #define TImpl concat(T, Impl)
#endif /* TPluginImpl */

#ifndef TPlugin
    #define TPlugin concat(T, Plugin)
#endif /* TPlugin */

#ifndef TPluginImpl
    #define TPluginImpl concat(TPlugin, Impl)
#endif /* TPluginImpl */

#define TPlugin_create concat(TPlugin, _create)
#define TPlugin_new concat(TPlugin, _new)
#define TPlugin_delete concat(TPlugin, _delete)
#define TPlugin_create_transformation concat(TPlugin, _create_transformation)
#define TPlugin_delete_transformation concat(TPlugin, _delete_transformation)

#ifdef TPluginConfig
    #ifndef TPluginConfig_parse
        #define TPluginConfig_parse \
            concat(TPluginConfig, _parse_from_properties)
    #endif /* TPluginConfig_parse */
#endif     /* TPluginConfig */

#define T_new concat(T, _new)
#define T_delete concat(T, _delete)
#define T_transform concat(T, _transform)
#define T_return_loan concat(T, _return_loan)
#define T_update concat(T, _update)

#ifdef TConfig
    #ifndef TConfig_parse
        #define TConfig_parse concat(TConfig, _parse_from_properties)
    #endif /* TConfig_parse */
#endif     /* TConfig */

#ifdef T_static
    #ifndef T_serialize
        #define T_serialize concat(T, _serialize)
    #endif /* T_serialize */
    #ifndef T_deserialize
        #define T_deserialize concat(T, _deserialize)
    #endif /* T_deserialize */
#endif     /* T_static */

/*****************************************************************************
 *                           Generated Header File
 *****************************************************************************/

#include "TransformationSimple.h"

typedef struct TPluginImpl {
    RTI_TSFM_TransformationPlugin parent;
#ifdef TPluginState
    TPluginState *state;
#endif
#ifdef TPluginConfig
    TPluginConfig *config;
#endif
} TPlugin;

RTI_USER_DLL_EXPORT
extern struct RTI_RoutingServiceTransformationPlugin *TPlugin_create(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

DDS_ReturnCode_t TPlugin_new(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        TPlugin **plugin_out);

void TPlugin_delete(
        struct RTI_RoutingServiceTransformationPlugin *plugin,
        RTI_RoutingServiceEnvironment *env);

RTI_RoutingServiceTransformation TPlugin_create_transformation(
        struct RTI_RoutingServiceTransformationPlugin *plugin,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

void TPlugin_delete_transformation(
        struct RTI_RoutingServiceTransformationPlugin *plugin,
        RTI_RoutingServiceTransformation transformation,
        RTI_RoutingServiceEnvironment *env);


typedef struct TImpl {
    RTI_TSFM_Transformation parent;
#ifdef TState
    TState *state;
#endif
#ifdef TConfig
    TConfig *config;
#endif
} T;

DDS_ReturnCode_t
        T_new(TPlugin *plugin,
              const struct RTI_RoutingServiceTypeInfo *input_type_info,
              const struct RTI_RoutingServiceTypeInfo *output_type_info,
              const struct RTI_RoutingServiceProperties *properties,
              RTI_RoutingServiceEnvironment *env,
              T **transform_out);

void T_delete(T *self);

void T_transform(
        RTI_RoutingServiceTransformation transformation,
        RTI_RoutingServiceSample **out_sample_lst,
        RTI_RoutingServiceSampleInfo **out_info_lst,
        int *out_count,
        RTI_RoutingServiceSample *in_sample_lst,
        RTI_RoutingServiceSampleInfo *in_info_lst,
        int in_count,
        RTI_RoutingServiceEnvironment *env);

void T_return_loan(
        RTI_RoutingServiceTransformation transformation,
        RTI_RoutingServiceSample *sample_lst,
        RTI_RoutingServiceSampleInfo *info_lst,
        int count,
        RTI_RoutingServiceEnvironment *env);

void T_update(
        RTI_RoutingServiceTransformation transformation,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env);

#ifdef T_static
DDS_ReturnCode_t T_serialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *self,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out);

DDS_ReturnCode_t T_deserialize(
        RTI_TSFM_UserTypePlugin *plugin,
        RTI_TSFM_Transformation *self,
        DDS_DynamicData *sample_in,
        DDS_DynamicData *sample_out);
#endif /* T_static */

#ifdef TPluginConfig_parse
DDS_ReturnCode_t TPluginConfig_parse(
        TPluginConfig *self,
        const struct RTI_RoutingServiceProperties *properties);
#endif /* TPluginConfig_parse */

#ifdef TConfig_parse
DDS_ReturnCode_t TConfig_parse(
        TConfig *self,
        const struct RTI_RoutingServiceProperties *properties);
#endif /* TConfig_parse */


#undef T
#undef TImpl
#undef TPlugin
#undef TPluginImpl
#undef TPlugin_create
#undef TPlugin_new
#undef TPlugin_delete
#undef TPlugin_create_transformation
#undef TPlugin_delete_transformation
#undef T_new
#undef T_delete
#undef T_transform
#undef T_return_loan
#undef T_update
#undef T_static
#undef T_serialize
#undef T_deserialize
#undef TConfig
#undef TState
#undef TPluginConfig
#undef TPluginState

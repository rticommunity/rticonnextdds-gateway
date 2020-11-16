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

#ifndef TPlugin
    #define TPlugin concat(T, Plugin)
#endif /* TPlugin */

#define TPlugin_create concat(TPlugin, _create)
#define TPlugin_new concat(TPlugin, _new)
#define TPlugin_delete concat(TPlugin, _delete)
#define TPlugin_create_transformation concat(TPlugin, _create_transformation)
#define TPlugin_delete_transformation concat(TPlugin, _delete_transformation)

#ifdef TPluginState
    #ifndef TPluginState_new
        #define TPluginState_new concat(TPluginState, TypeSupport_create_data)
    #endif /* TPluginState_new */
    #ifndef TPluginState_delete
        #define TPluginState_delete \
            concat(TPluginState, TypeSupport_delete_data)
    #endif /* TPluginState_delete */
#endif     /* TPluginState */

#ifdef TPluginConfig
    #ifndef TPluginConfig_new
        #define TPluginConfig_new concat(TPluginConfig, TypeSupport_create_data)
    #endif /* TPluginConfig_new */
    #ifndef TPluginConfig_delete
        #define TPluginConfig_delete \
            concat(TPluginConfig, TypeSupport_delete_data)
    #endif /* TPluginConfig_delete */
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

#ifdef TState
    #ifndef TState_new
        #define TState_new concat(TState, TypeSupport_create_data)
    #endif /* TState_new */
    #ifndef TState_delete
        #define TState_delete concat(TState, TypeSupport_delete_data)
    #endif /* TState_delete */
#endif     /* TState */

#ifdef TConfig
    #ifndef TConfig_new
        #define TConfig_new concat(TConfig, TypeSupport_create_data)
    #endif /* TnConfig_new */
    #ifndef TConfig_delete
        #define TConfig_delete concat(TConfig, TypeSupport_delete_data)
    #endif /* TConfig_delete */
    #ifndef TConfig_parse
        #define TConfig_parse concat(TConfig, _parse_from_properties)
    #endif /* TConfig_parse */
#endif     /* TConfig */

#ifndef T_VERSION_MAJOR
    #define T_VERSION_MAJOR 1
#endif /* T_VERSION_MAJOR */

#ifndef T_VERSION_MINOR
    #define T_VERSION_MINOR 0
#endif /* T_VERSION_MINOR */

#ifndef T_VERSION_RELEASE
    #define T_VERSION_RELEASE 0
#endif /* T_VERSION_RELEASE */

#ifndef T_VERSION_REVISION
    #define T_VERSION_REVISION 0
#endif /* T_VERSION_REVISION */

#ifndef TPlugin_VERSION
    #define TPlugin_VERSION concat(TPlugin, Plugin_VERSION)
#endif /* TPlugin_VERSION */

#ifdef T_static
    #ifndef T_serialize
        #define T_serialize concat(T, _serialize)
    #endif /* T_serialize */
    #ifndef T_deserialize
        #define T_deserialize concat(T, _deserialize)
    #endif /* T_deserialize */
#endif     /* T_static */

/*****************************************************************************
 *                            Transformation Plugin
 *****************************************************************************/

#ifdef TPlugin_VERSION
const struct RTI_RoutingServiceVersion TPlugin_VERSION = { T_VERSION_MAJOR,
                                                           T_VERSION_MINOR,
                                                           T_VERSION_RELEASE,
                                                           T_VERSION_REVISION };
#endif /* TPlugin_VERSION */

struct RTI_RoutingServiceTransformationPlugin *TPlugin_create(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    TPlugin *self = NULL;

    RTI_TSFM_LOG_FN(TPlugin_create)

    if (DDS_RETCODE_OK != TPlugin_new(properties, env, &self)) {
        goto done;
    }

    retval = DDS_BOOLEAN_TRUE;
done:
    if (retval) {
        return &self->parent.parent;
    } else {
        return NULL;
    }
}


DDS_ReturnCode_t TPlugin_new(
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env,
        TPlugin **plugin_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Boolean parent_initd = DDS_BOOLEAN_FALSE;
#ifdef T_static
    RTI_TSFM_UserTypePlugin *user_plugin = NULL;
#endif /* T_static */
    TPlugin *self = NULL;

    RTI_TSFM_LOG_FN(TPlugin_new)

    *plugin_out = NULL;

    self = (TPlugin *) RTI_TSFM_Heap_allocate(sizeof(TPlugin));
    if (self == NULL) {
        goto done;
    }
#ifdef TPluginConfig
    self->config = NULL;
#endif /* TPluginConfig */

#ifdef TPluginState
    self->state = TPluginState_new();
    if (self->state == NULL) {
        goto done;
    }
#endif /* TPluginState */

#ifdef TPluginConfig
    self->config = TPluginConfig_new();
    if (self->config == NULL) {
        goto done;
    }
    if (DDS_RETCODE_OK != TPluginConfig_parse(&self->config, properties)) {
        goto done;
    }
    self->parent.config = &self->config->parent;
#endif /* TPluginConfig */

#ifdef TPlugin_initialize
    if (DDS_RETCODE_OK != TPlugin_initialize(self, properties, env)) {
        goto done;
    }
#else
    #ifdef T_static
    user_plugin = RTI_TSFM_UserTypePlugin_create_static(
            &self->parent,
            T_serialize,
            T_deserialize);
    if (user_plugin == NULL) {
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationPlugin_initialize_static(
                &self->parent,
                user_plugin,
                NULL,
                properties,
                env)) {
        goto done;
    }
    #else
    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationPlugin_initialize(
                &self->parent,
                properties,
                env)) {
        goto done;
    }
    #endif /* T_static */
#endif     /* TPlugin_initialize */
    parent_initd = DDS_BOOLEAN_TRUE;

    self->parent.parent.plugin_version = TPlugin_VERSION;
    self->parent.parent.transformation_plugin_delete = TPlugin_delete;
    self->parent.parent.transformation_plugin_create_transformation =
            TPlugin_create_transformation;
    self->parent.parent.transformation_plugin_delete_transformation =
            TPlugin_delete_transformation;
    self->parent.parent.transformation_transform = T_transform;
    self->parent.parent.transformation_return_loan = T_return_loan;
    self->parent.parent.transformation_update = T_update;
    self->parent.parent.user_object = self;

    *plugin_out = self;

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (self != NULL) {
            if (parent_initd) {
                TPlugin_delete(&self->parent.parent, env);
            } else {
#ifdef TPluginState
                if (self->state != NULL) {
                    TPluginState_delete(self->state);
                }
#endif /* TPluginState */
#ifdef TPluginConfig
                if (self->config != NULL) {
                    TPluginConfig_delete(self->config);
                }
#endif /* TPluginConfig */
#ifdef T_static
                if (user_plugin != NULL) {
                    RTI_TSFM_UserTypePlugin_delete_default(
                            user_plugin,
                            &self->parent);
                }
#endif /* T_static */
                RTI_TSFM_Heap_free(self);
            }
        }
    }
    return retcode;
}

void TPlugin_delete(
        struct RTI_RoutingServiceTransformationPlugin *plugin,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    TPlugin *self = (TPlugin *) plugin;
    DDS_UnsignedLong seq_len = 0;

    RTI_TSFM_LOG_FN(TPlugin_delete)

    seq_len =
            RTI_TSFM_TransformationPtrSeq_get_length(&self->parent.transforms);
    if (seq_len > 0) {
        /* TODO Log error */
        RTI_TSFM_ERROR_1(
                "DELETING with existing transformation on plugin:",
                "%d",
                seq_len)
    }

#ifdef TPluginState
    if (self->state != NULL) {
        TPluginState_delete(self->state);
        self->state = NULL;
    }
#endif /* TPluginState */

#ifdef TPluginConfig
    if (self->config != NULL) {
        TPluginConfig_delete(self->config);
        self->config = NULL;
        self->parent.config = NULL;
    }
#endif /* TPluginConfig */

#ifdef TPlugin_finalize
    if (DDS_RETCODE_OK != TPlugin_finalize(self, env)) {
        /* TODO Log error */
    }
#else
    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationPlugin_finalize(&self->parent, env)) {
        /* TODO Log error */
    }
#endif /* TPlugin_finalize */
    RTI_TSFM_Heap_free(self);

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval) {
        /* TODO Log error */
    }
}

RTI_RoutingServiceTransformation TPlugin_create_transformation(
        struct RTI_RoutingServiceTransformationPlugin *plugin,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    TPlugin *self = (TPlugin *) plugin;
    T *transform = NULL, *transform_out = NULL;

    RTI_TSFM_LOG_FN(TPlugin_create_transformation)

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_take(&self->parent.lock)) {
        /* TODO Log error */
        return NULL;
    }
#endif /* RTI_TSFM_USE_MUTEX */

    if (DDS_RETCODE_OK
        != T_new(
                self,
                input_type_info,
                output_type_info,
                properties,
                env,
                &transform)) {
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationPlugin_initialize_transformation(
                &self->parent,
                &transform->parent,
                input_type_info,
                output_type_info,
                properties,
                env)) {
        goto done;
    }

    transform_out = transform;

    retval = DDS_BOOLEAN_TRUE;

done:

    if (!retval) {
        if (transform != NULL) {
            T_delete(transform);
        }
        transform_out = NULL;
    }

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_give(&self->parent.lock)) {
        /* TODO Log error */
    }
#endif /* RTI_TSFM_USE_MUTEX */

    return transform_out;
}

void TPlugin_delete_transformation(
        struct RTI_RoutingServiceTransformationPlugin *plugin,
        RTI_RoutingServiceTransformation transformation,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retcode = DDS_BOOLEAN_FALSE;
    TPlugin *self = (TPlugin *) plugin;
    T *transform = (T *) transformation;

    RTI_TSFM_LOG_FN(TPlugin_delete_transformation)

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_take(&self->parent.lock)) {
        /* TODO Log error */
        return;
    }
#endif /* RTI_TSFM_USE_MUTEX */

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationPlugin_finalize_transformation(
                &self->parent,
                &transform->parent,
                env)) {
        goto done;
    }

    T_delete(transform);

    retcode = DDS_BOOLEAN_TRUE;

done:
    if (!retcode) {
        /* TODO Log error */
    }

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_give(&self->parent.lock)) {
        /* TODO Log error */
    }
#endif /* RTI_TSFM_USE_MUTEX */
}

/*****************************************************************************
 *                            Transformation
 *****************************************************************************/


DDS_ReturnCode_t
        T_new(TPlugin *plugin,
              const struct RTI_RoutingServiceTypeInfo *input_type_info,
              const struct RTI_RoutingServiceTypeInfo *output_type_info,
              const struct RTI_RoutingServiceProperties *properties,
              RTI_RoutingServiceEnvironment *env,
              T **transform_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Boolean parent_initd = DDS_BOOLEAN_FALSE;
    T *self = NULL;

    RTI_TSFM_LOG_FN(T_new)

    *transform_out = NULL;

    self = (T *) RTI_TSFM_Heap_allocate(sizeof(T));
    if (self == NULL) {
        goto done;
    }
#ifdef TConfig
    self->config = NULL;
#endif /* TConfig */

#ifdef TState
    self->state = TState_new();
    if (self->state == NULL) {
        goto done;
    }
#endif /* TState */
#ifdef TConfig
    self->config = TConfig_new();
    if (self->config == NULL) {
        goto done;
    }
    if (DDS_RETCODE_OK != TConfig_parse(self->config, properties)) {
        goto done;
    }
    self->parent.config = &self->config->parent;
#endif /* TConfig */

#ifdef T_initialize
    if (DDS_RETCODE_OK
        != T_initialize(
                self,
                plugin,
                input_type_info,
                output_type_info,
                properties,
                env)) {
        goto done;
    }
#else
    if (DDS_RETCODE_OK
        != RTI_TSFM_Transformation_initialize(
                &self->parent,
                &plugin->parent,
                input_type_info,
                output_type_info,
                properties,
                env)) {
        goto done;
    }
#endif /* T_initialize */
    parent_initd = DDS_BOOLEAN_TRUE;

    *transform_out = self;

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (self != NULL) {
            if (parent_initd) {
                T_delete(self);
            } else {
#ifdef TState
                if (self->state != NULL) {
                    TState_delete(self->state);
                }
#endif /* TState */
#ifdef TConfig
                if (self->config != NULL) {
                    TConfig_delete(self->config);
                }
#endif /* TConfig */
                RTI_TSFM_Heap_free(self);
            }
        }
    }
    return retcode;
}

void T_delete(T *self)
{
    RTI_TSFM_LOG_FN(T_delete)

#ifdef TConfig
    if (self->config != NULL) {
        TConfig_delete(self->config);
        self->config = NULL;
        self->parent.config = NULL;
    }
#endif /* TConfig */

#ifdef TState
    if (self->state != NULL) {
        TState_delete(self->state);
        self->state = NULL;
    }
#endif /* TState */

    if (DDS_RETCODE_OK != RTI_TSFM_Transformation_finalize(&self->parent)) {
        /* TODO Log error */
    }

    RTI_TSFM_Heap_free(self);
}


void T_transform(
        RTI_RoutingServiceTransformation transformation,
        RTI_RoutingServiceSample **out_sample_lst,
        RTI_RoutingServiceSampleInfo **out_info_lst,
        int *out_count,
        RTI_RoutingServiceSample *in_sample_lst,
        RTI_RoutingServiceSampleInfo *in_info_lst,
        int in_count,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_Boolean retval = DDS_BOOLEAN_FALSE;
    T *self = (T *) transformation;

    RTI_TSFM_LOG_FN(T_transform)

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_take(&self->parent.lock)) {
        /* TODO Log error */
        return;
    }
#endif /* RTI_TSFM_USE_MUTEX */

    if (DDS_RETCODE_OK
        != RTI_TSFM_Transformation_transform(
                &self->parent,
                out_sample_lst,
                out_info_lst,
                out_count,
                in_sample_lst,
                in_info_lst,
                in_count,
                env)) {
        goto done;
    }
    retval = DDS_BOOLEAN_TRUE;
done:

    if (!retval) {
        /* TODO Log error */
    }
#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_give(&self->parent.lock)) {
        /* TODO Log error */
    }
#endif /* RTI_TSFM_USE_MUTEX */
}

void T_return_loan(
        RTI_RoutingServiceTransformation transformation,
        RTI_RoutingServiceSample *sample_lst,
        RTI_RoutingServiceSampleInfo *info_lst,
        int count,
        RTI_RoutingServiceEnvironment *env)
{
    T *self = (T *) transformation;

    RTI_TSFM_LOG_FN(T_return_loan)

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_take(&self->parent.lock)) {
        /* TODO Log error */
        return;
    }
#endif /* RTI_TSFM_USE_MUTEX */

    if (DDS_RETCODE_OK
        != RTI_TSFM_Transformation_return_loan(
                &self->parent,
                sample_lst,
                info_lst,
                count,
                env)) {
        /* TODO Log error */
    }
#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_give(&self->parent.lock)) {
        /* TODO Log error */
    }
#endif /* RTI_TSFM_USE_MUTEX */
}

void T_update(
        RTI_RoutingServiceTransformation transformation,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    T *self = (T *) transformation;

    RTI_TSFM_LOG_FN(T_update)

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_take(&self->parent.lock)) {
        /* TODO Log error */
        return;
    }
#endif /* RTI_TSFM_USE_MUTEX */

    if (DDS_RETCODE_OK
        != RTI_TSFM_Transformation_update(&self->parent, properties, env)) {
        /* TODO Log error */
    }
#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_take(&self->parent.lock)) {
        /* TODO Log error */
    }
#endif /* RTI_TSFM_USE_MUTEX */
}

#undef T
#undef TImpl
#undef TPlugin
#undef TPluginImpl
#undef TPlugin_create
#undef TPlugin_new
#undef TPlugin_initialize
#undef TPlugin_finalize
#undef TPlugin_delete
#undef TPlugin_create_transformation
#undef TPlugin_delete_transformation
#undef TPluginState
#undef TPluginConfig
#undef T_new
#undef T_initialize
#undef T_finalize
#undef T_delete
#undef T_transform
#undef T_return_loan
#undef T_update
#undef T_static
#undef T_serialize
#undef T_deserialize
#undef TConfig
#undef TState
#undef T_VERSION_MAJOR
#undef T_VERSION_MINOR
#undef T_VERSION_REVISION
#undef T_VERSION_RELEASE
#undef TPlugin_VERSION
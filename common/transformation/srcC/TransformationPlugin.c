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

#include "TransformationPlugin.h"
#include "Transformation.h"
#include "TransformationInfrastructure.h"
#include "TransformationPlatform.h"
#include "TransformationSimple.h"
#include "TransformationUserPlugin.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::simple::plugin"

const RTI_TSFM_TransformationPluginConfig
        RTI_TSFM_TransformationPluginConfig_DEFAULT =
                RTI_TSFM_TransformationPluginConfig_INITIALIZER;


static DDS_ReturnCode_t
        RTI_TSFM_TransformationPluginConfig_parse_from_properties_alloc(
                const struct RTI_RoutingServiceProperties *properties,
                RTI_TSFM_TransformationPluginConfig **config_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_TransformationPluginConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(
            RTI_TSFM_TransformationPluginConfig_parse_from_properties_alloc)

    loaned_config = (config != NULL);

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationPluginConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }

    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_PROPERTY_PLUGIN_DLL,
            DDS_String_replace(&config->dll, pval);
            if (config->dll == NULL) {
                /* TODO Log error */
                goto done;
            })

            RTI_TSFM_lookup_property(
                    properties,
                    RTI_TSFM_PROPERTY_CREATE_FN,
                    DDS_String_replace(&config->create_fn, pval);
                    if (config->create_fn == NULL) {
                        /* TODO Log error */
                        goto done;
                    })

                    *config_out = config;

    retcode = DDS_RETCODE_OK;
done:

    if (retcode != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_TSFM_TransformationPluginConfig_delete(config);
        }
    }
    return retcode;
}


DDS_ReturnCode_t RTI_TSFM_TransformationPluginConfig_parse_from_properties(
        RTI_TSFM_TransformationPluginConfig *config,
        const struct RTI_RoutingServiceProperties *properties)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPluginConfig_parse_from_properties)
    return RTI_TSFM_TransformationPluginConfig_parse_from_properties_alloc(
            properties,
            &config);
}


DDS_ReturnCode_t RTI_TSFM_TransformationPluginConfig_default(
        RTI_TSFM_TransformationPluginConfig **config_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_TSFM_TransformationPluginConfig *config = *config_out;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPluginConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_TSFM_TransformationPluginConfig_new(
                    DDS_BOOLEAN_FALSE,
                    &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    if (!RTI_TSFM_TransformationPluginConfig_copy(
                config,
                &RTI_TSFM_TransformationPluginConfig_DEFAULT)) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retcode = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retcode) {
        if (config != NULL && config_allocd) {
            RTI_TSFM_TransformationPluginConfig_delete(config);
        }
    }
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_TransformationPluginConfig_new(
        DDS_Boolean allocate_optional,
        RTI_TSFM_TransformationPluginConfig **config_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_TransformationPluginConfig *config = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPluginConfig_new)

    /* Allocate memory for config */
    RTIOsapiHeap_allocateStructure(
            &(config),
            RTI_TSFM_TransformationPluginConfig);
    if (config == NULL) {
        /* TODO Log error */
        goto done;
    }

    config->dll = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&config->dll, "", (0), RTI_FALSE);
    if (config->dll == NULL) {
        goto done;
    }

    config->create_fn = DDS_String_alloc((0));
    RTICdrType_copyStringEx(&config->create_fn, "", (0), RTI_FALSE);
    if (config->create_fn == NULL) {
        goto done;
    }

    *config_out = config;

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        RTI_TSFM_TransformationPluginConfig_delete(config);
    }
    return retcode;
}

void RTI_TSFM_TransformationPluginConfig_delete(
        RTI_TSFM_TransformationPluginConfig *self)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPluginConfig_delete)

    if (self == NULL) {
        return;
    }

    if (self->dll != NULL) {
        DDS_String_free(self->dll);
        self->dll = NULL;
    }
    if (self->create_fn != NULL) {
        DDS_String_free(self->create_fn);
        self->create_fn = NULL;
    }

    RTIOsapiHeap_freeStructure(self);
}


DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_initialize(
        RTI_TSFM_TransformationPlugin *self,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_UserTypePlugin *user_plugin = NULL;
    DDS_Boolean parsed_config = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPlugin_initialize)

    if (self->config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_TSFM_TransformationPluginConfig_parse_from_properties_alloc(
                    properties,
                    &self->config)) {
            /* TODO Log error */
            goto done;
        }
        parsed_config = DDS_BOOLEAN_TRUE;
    }


    user_plugin = RTI_TSFM_UserTypePlugin_create_dynamic(
            self,
            self->config->dll,
            self->config->create_fn);

    if (user_plugin == NULL) {
        /* TODO Log error */
        goto done;
    }

    if (DDS_RETCODE_OK
        != RTI_TSFM_TransformationPlugin_initialize_static(
                self,
                user_plugin,
                self->config,
                properties,
                env)) {
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:

    if (retcode != DDS_RETCODE_OK) {
        if (self->config != NULL && parsed_config) {
            RTI_TSFM_TransformationPluginConfig_delete(self->config);
            self->config = NULL;
        }
    }

    return retcode;
}


DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_initialize_static(
        RTI_TSFM_TransformationPlugin *self,
        RTI_TSFM_UserTypePlugin *user_plugin,
        RTI_TSFM_TransformationPluginConfig *config,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    struct RTI_TSFM_TransformationPtrSeq def_transforms =
            DDS_SEQUENCE_INITIALIZER;
    DDS_Boolean parsed_config = DDS_BOOLEAN_FALSE;
#if RTI_TSFM_USE_MUTEX
    DDS_Boolean lock_initd = DDS_BOOLEAN_FALSE;
#endif /* RTI_TSFM_USE_MUTEX */

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPlugin_initialize_static)

    RTI_RoutingServiceTransformationPlugin_initialize(&self->parent);

    self->transforms = def_transforms;
    self->user_plugin = user_plugin;
    self->config = config;

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_initialize(&self->lock)) {
        /* TODO Log error */
        goto done;
    }
    lock_initd = DDS_BOOLEAN_TRUE;
#endif /* RTI_TSFM_USE_MUTEX */

    if (!RTI_TSFM_TransformationPtrSeq_initialize(&self->transforms)) {
        /* TODO Log error */
        goto done;
    }

    if (self->config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_TSFM_TransformationPluginConfig_parse_from_properties_alloc(
                    properties,
                    &self->config)) {
            /* TODO Log error */
            goto done;
        }
        parsed_config = DDS_BOOLEAN_TRUE;
    }

    retcode = DDS_RETCODE_OK;

done:
#if RTI_TSFM_USE_MUTEX
    /* If we didn't initialize the lock, then there's nothing to finalize */
    if (retcode != DDS_RETCODE_OK && lock_initd)
#else
    if (retcode != DDS_RETCODE_OK)
#endif /* RTI_TSFM_USE_MUTEX */
    {
        RTI_TSFM_TransformationPluginConfig *config = NULL;
        /* Make sure we don't delete self->config if it was passed by caller */
        if (self->config != NULL && !parsed_config) {
            config = self->config;
            self->config = NULL;
        }
        RTI_TSFM_TransformationPlugin_finalize(self, env);
        self->config = config;
    }

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_finalize(
        RTI_TSFM_TransformationPlugin *self,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPlugin_finalize)

    seq_len = RTI_TSFM_TransformationPtrSeq_get_length(&self->transforms);

    if (seq_len > 0) {
        /* TODO Log error */
        goto done;
    }

    if (!RTI_TSFM_TransformationPtrSeq_finalize(&self->transforms)) {
        /* TODO Log error */
    }
    if (self->user_plugin != NULL && self->user_plugin->delete_plugin != NULL) {
        self->user_plugin->delete_plugin(self->user_plugin, self);
        self->user_plugin = NULL;
    }
    if (self->config != NULL) {
        RTI_TSFM_TransformationPluginConfig_delete(self->config);
        self->config = NULL;
    }
#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_finalize(&self->lock)) {
        /* TODO Log error */
    }
#endif /* RTI_TSFM_USE_MUTEX */

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_initialize_transformation(
        RTI_TSFM_TransformationPlugin *self,
        RTI_TSFM_Transformation *transform,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong cur_transform_len = 0;
    RTI_TSFM_Transformation **transform_ref = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPlugin_initialize_transformation)

    cur_transform_len =
            RTI_TSFM_TransformationPtrSeq_get_length(&self->transforms);

    if (!RTI_TSFM_TransformationPtrSeq_ensure_length(
                &self->transforms,
                cur_transform_len + 1,
                cur_transform_len + 1)) {
        /* TODO Log error */
        goto done;
    }

    transform_ref = RTI_TSFM_TransformationPtrSeq_get_reference(
            &self->transforms,
            cur_transform_len);

    *transform_ref = transform;

    RTI_TSFM_LOG_2(
            "CREATED transformation:",
            "plugin=%p, tot_transforms=%d",
            self,
            RTI_TSFM_TransformationPtrSeq_get_length(&self->transforms))

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (transform_ref != NULL && *transform_ref != NULL) {
            *transform_ref = NULL;
            if (!RTI_TSFM_TransformationPtrSeq_set_length(
                        &self->transforms,
                        cur_transform_len)) {
                /* TODO Log error */
                goto done;
            }
        }
    }
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_TransformationPlugin_finalize_transformation(
        RTI_TSFM_TransformationPlugin *self,
        RTI_TSFM_Transformation *transform,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong seq_len = 0, i = 0, rm_i = 0;
    DDS_Boolean do_rm = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationPlugin_finalize_transformation)

    seq_len = RTI_TSFM_TransformationPtrSeq_get_length(&self->transforms);

    for (i = 0; i < seq_len && !do_rm; i++) {
        if (*RTI_TSFM_TransformationPtrSeq_get_reference(&self->transforms, i)
            == transform) {
            rm_i = i;
            do_rm = DDS_BOOLEAN_TRUE;
        }
    }

    if (!do_rm) {
        RTI_TSFM_ERROR_3(
                "tranformation NOT FOUND:",
                "plugin=%p, transform=%p, tot_transforms=%d",
                self,
                transform,
                seq_len)
        goto done;
    }

    for (i = rm_i; i < seq_len - 1; i++) {
        *RTI_TSFM_TransformationPtrSeq_get_reference(&self->transforms, i) =
                *RTI_TSFM_TransformationPtrSeq_get_reference(
                        &self->transforms,
                        i + 1);
    }

    if (!RTI_TSFM_TransformationPtrSeq_set_length(
                &self->transforms,
                seq_len - 1)) {
        /* TODO Log error */

        goto done;
    }

    RTI_TSFM_LOG_2(
            "DELETED transformation:",
            "plugin=%p, tot_transforms=%d",
            self,
            RTI_TSFM_TransformationPtrSeq_get_length(&self->transforms))

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

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

#include "Transformation.h"
#include "TransformationInfrastructure.h"
#include "TransformationPlatform.h"
#include "TransformationSimple.h"
#include "TransformationTypes.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::simple"

const RTI_TSFM_TransformationConfig RTI_TSFM_TransformationConfig_DEFAULT =
        RTI_TSFM_TransformationConfig_INITIALIZER;


static DDS_ReturnCode_t RTI_TSFM_TransformationKind_from_string(
        const char *str,
        RTI_TSFM_TransformationKind *kind_out)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationKind_from_string)

    if (RTI_TSFM_String_compare(str, "serialize") == 0
        || RTI_TSFM_String_compare(str, "serializer") == 0
        || RTI_TSFM_String_compare(str, "s") == 0
        || RTI_TSFM_String_compare(str, "S") == 0
        || RTI_TSFM_String_compare(
                   str,
                   "RTI_TSFM_TransformationKind_SERIALIZER")
                == 0) {
        *kind_out = RTI_TSFM_TransformationKind_SERIALIZER;
        return DDS_RETCODE_OK;
    } else if (
            RTI_TSFM_String_compare(str, "deserialize") == 0
            || RTI_TSFM_String_compare(str, "deserializer") == 0
            || RTI_TSFM_String_compare(str, "d") == 0
            || RTI_TSFM_String_compare(str, "D") == 0
            || RTI_TSFM_String_compare(
                       str,
                       "RTI_TSFM_TransformationKind_DESERIALIZER")
                    == 0) {
        *kind_out = RTI_TSFM_TransformationKind_DESERIALIZER;
        return DDS_RETCODE_OK;
    }

    return DDS_RETCODE_ERROR;
}

static DDS_ReturnCode_t
        RTI_TSFM_TransformationConfig_parse_from_properties_alloc(
                const struct RTI_RoutingServiceProperties *properties,
                RTI_TSFM_TransformationConfig **config_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_TransformationConfig *config = *config_out;
    DDS_Boolean loaned_config = DDS_BOOLEAN_FALSE,
                allocd_config = DDS_BOOLEAN_FALSE;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationConfig_parse_from_properties_alloc)

    loaned_config = (config != NULL);

    if (DDS_RETCODE_OK != RTI_TSFM_TransformationConfig_default(&config)) {
        /* TODO Log error */
        goto done;
    }

    if (!loaned_config) {
        allocd_config = DDS_BOOLEAN_TRUE;
    }

    RTI_TSFM_lookup_property(
            properties,
            RTI_TSFM_PROPERTY_TRANSFORMATION_TYPE,
            if (DDS_RETCODE_OK
                != RTI_TSFM_TransformationKind_from_string(
                        pval,
                        &config->type)) {
                /* TODO Log error */
                goto done;
            })

            RTI_TSFM_lookup_property(
                    properties,
                    RTI_TSFM_PROPERTY_TRANSFORMATION_INPUT_TYPE,
                    DDS_String_replace(&config->input_type, pval);
                    if (config->input_type == NULL) {
                        /* TODO Log error */
                        goto done;
                    })

                    RTI_TSFM_lookup_property(
                            properties,
                            RTI_TSFM_PROPERTY_TRANSFORMATION_OUTPUT_TYPE,
                            DDS_String_replace(&config->output_type, pval);
                            if (config->output_type == NULL) {
                                /* TODO Log error */
                                goto done;
                            })

                            *config_out = config;

    retcode = DDS_RETCODE_OK;
done:

    if (retcode != DDS_RETCODE_OK) {
        if (config != NULL && allocd_config) {
            RTI_TSFM_TransformationConfig_delete(config);
        }
    }
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_TransformationConfig_parse_from_properties(
        RTI_TSFM_TransformationConfig *config,
        const struct RTI_RoutingServiceProperties *properties)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationConfig_parse_from_properties)
    return RTI_TSFM_TransformationConfig_parse_from_properties_alloc(
            properties,
            &config);
}

DDS_ReturnCode_t RTI_TSFM_TransformationConfig_default(
        RTI_TSFM_TransformationConfig **config_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_TSFM_TransformationConfig *config = *config_out;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_TSFM_TransformationConfig_new(DDS_BOOLEAN_FALSE, &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    config->type = RTI_TSFM_TransformationConfig_DEFAULT.type;
    if (!RTICdrType_copyStringEx(
                &config->input_type,
                RTI_TSFM_TransformationConfig_DEFAULT.input_type,
                (RTIXCdrLong_MAX - 1) + 1,
                RTI_TRUE)) {
        goto done;
    }

    if (!RTICdrType_copyStringEx(
                &config->output_type,
                RTI_TSFM_TransformationConfig_DEFAULT.output_type,
                (RTIXCdrLong_MAX - 1) + 1,
                RTI_TRUE)) {
        goto done;
    }

    *config_out = config;

    retcode = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retcode) {
        if (config != NULL && config_allocd) {
            RTI_TSFM_TransformationConfig_delete(config);
        }
    }
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_TransformationConfig_new(
        DDS_Boolean allocate_optional,
        RTI_TSFM_TransformationConfig **config_out)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_TransformationConfig *config = NULL;

    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationConfig_new)

    config = (RTI_TSFM_TransformationConfig *) malloc(
            sizeof(RTI_TSFM_TransformationConfig));
    if (config != NULL) {
        config->type = RTI_TSFM_TransformationKind_SERIALIZER;
        config->input_type = DDS_String_alloc((0));
        RTICdrType_copyStringEx(&config->input_type, "", (0), RTI_FALSE);
        if (config->input_type == NULL) {
            goto done;
        }
        config->output_type = DDS_String_alloc((0));
        RTICdrType_copyStringEx(&config->output_type, "", (0), RTI_FALSE);
        if (config->output_type == NULL) {
            goto done;
        }
    } else {
        /* Log error */
        goto done;
    }

    *config_out = config;

    retcode = DDS_RETCODE_OK;

done:
    return retcode;
}

void RTI_TSFM_TransformationConfig_delete(RTI_TSFM_TransformationConfig *self)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_TransformationConfig_delete)
    if (self == NULL) {
        return;
    }
    if (self->input_type != NULL) {
        DDS_String_free(self->input_type);
    }
    if (self->output_type != NULL) {
        DDS_String_free(self->output_type);
    }
    free(self);
}

static DDS_ReturnCode_t RTI_TSFM_Transformation_create_type_support(
        RTI_TSFM_Transformation *self,
        const struct RTI_RoutingServiceTypeInfo *type_info,
        char **type_name,
        struct DDS_DynamicDataTypeSupport **tsupport)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;

    RTI_TSFM_LOG_FN(RTI_TSFM_Transformation_create_type_support)

    *tsupport = NULL;

    if (RTI_TSFM_String_length(*type_name) == 0) {
        if (NULL == DDS_String_replace(type_name, type_info->type_name)) {
            /* TODO Log error */
            goto done;
        }
    } else {
        if (RTI_TSFM_String_compare(*type_name, type_info->type_name) != 0) {
            /* TODO Log error */
            goto done;
        }
    }

    *tsupport = DDS_DynamicDataTypeSupport_new(
            (struct DDS_TypeCode *) type_info->type_representation,
            &DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
    if (*tsupport == NULL) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (*tsupport != NULL) {
            DDS_DynamicDataTypeSupport_delete(*tsupport);
        }
    }

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_Transformation_initialize(
        RTI_TSFM_Transformation *self,
        RTI_TSFM_TransformationPlugin *plugin,
        const struct RTI_RoutingServiceTypeInfo *input_type_info,
        const struct RTI_RoutingServiceTypeInfo *output_type_info,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    RTI_TSFM_TransformationConfig *config = NULL;
    struct RTI_TSFM_DDS_DynamicDataPtrSeq def_read_buffer =
            DDS_SEQUENCE_INITIALIZER;
    DDS_Boolean config_initd = DDS_BOOLEAN_FALSE;
#if RTI_TSFM_USE_MUTEX
    DDS_Boolean lock_initd = DDS_BOOLEAN_FALSE;
#endif /* RTI_TSFM_USE_MUTEX */

    RTI_TSFM_LOG_FN(RTI_TSFM_Transformation_initialize)

    self->tsupport = NULL;
    self->read_buffer = def_read_buffer;
    self->read_buffer_loaned = DDS_BOOLEAN_FALSE;
    self->plugin = plugin;

#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_initialize(&self->lock)) {
        /* TODO Log error */
        goto done;
    }
    lock_initd = DDS_BOOLEAN_TRUE;
#endif /* RTI_TSFM_USE_MUTEX */

    if (!RTI_TSFM_DDS_DynamicDataPtrSeq_initialize(&self->read_buffer)) {
        /* TODO Log error */
        goto done;
    }

    if (self->config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_TSFM_TransformationConfig_parse_from_properties_alloc(
                    properties,
                    &self->config)) {
            /* TODO Log error */
            goto done;
        }
        config_initd = DDS_BOOLEAN_TRUE;
    }

    if (DDS_RETCODE_OK
        != RTI_TSFM_Transformation_create_type_support(
                self,
                output_type_info,
                &self->config->output_type,
                &self->tsupport)) {
        /* TODO Log error */
        goto done;
    }

    retcode = DDS_RETCODE_OK;

done:
#if RTI_TSFM_USE_MUTEX
    if (retcode != DDS_RETCODE_OK && lock_initd)
#else
    if (retcode != DDS_RETCODE_OK)
#endif /* RTI_TSFM_USE_MUTEX */
    {
        RTI_TSFM_TransformationConfig *config = NULL;
        /* Make sure we don't delete self->config if it was set by caller */
        if (self->config != NULL && !config_initd) {
            config = self->config;
            self->config = NULL;
        }
        RTI_TSFM_Transformation_finalize(self);
        self->config = config;
    }
    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_Transformation_finalize(RTI_TSFM_Transformation *self)
{
    DDS_UnsignedLong seq_len = 0, i = 0;
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;

    RTI_TSFM_LOG_FN(RTI_TSFM_Transformation_finalize)

    seq_len = RTI_TSFM_DDS_DynamicDataPtrSeq_get_length(&self->read_buffer);
    for (i = 0; i < seq_len; i++) {
        DDS_DynamicData **data_ref =
                RTI_TSFM_DDS_DynamicDataPtrSeq_get_reference(
                        &self->read_buffer,
                        i);

        if (*data_ref == NULL) {
            continue;
        }
        if (DDS_RETCODE_OK
            != DDS_DynamicDataTypeSupport_delete_data(
                    self->tsupport,
                    *data_ref)) {
            /* TODO Log error */
        }

        *data_ref = NULL;
    }
    if (!RTI_TSFM_DDS_DynamicDataPtrSeq_set_length(&self->read_buffer, 0)) {
        /* TODO Log error */
    }

    if (self->config != NULL) {
        RTI_TSFM_TransformationConfig_delete(self->config);
        self->config = NULL;
    }

    if (self->tsupport != NULL) {
        DDS_DynamicDataTypeSupport_delete(self->tsupport);
        self->tsupport = NULL;
    }

    if (!RTI_TSFM_DDS_DynamicDataPtrSeq_finalize(&self->read_buffer)) {
        /* TODO Log error */
        retcode = DDS_RETCODE_ERROR;
    }
#if RTI_TSFM_USE_MUTEX
    if (DDS_RETCODE_OK != RTI_TSFM_Mutex_finalize(&self->lock)) {
        /* TODO Log error */
        retcode = DDS_RETCODE_ERROR;
    }
#endif /* RTI_TSFM_USE_MUTEX */

    return retcode;
}


DDS_ReturnCode_t RTI_TSFM_Transformation_transform(
        RTI_TSFM_Transformation *self,
        RTI_RoutingServiceSample **out_sample_lst,
        RTI_RoutingServiceSampleInfo **out_info_lst,
        int *out_count,
        RTI_RoutingServiceSample *in_sample_lst,
        RTI_RoutingServiceSampleInfo *in_info_lst,
        int in_count,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_DynamicData **out_samples = NULL,
                    **in_samples = (DDS_DynamicData **) in_sample_lst;
    struct DDS_SampleInfo *out_infos = NULL,
                          *in_infos = (struct DDS_SampleInfo *) in_info_lst;
    DDS_UnsignedLong i = 0, out_samples_initd = 0, read_buffer_max = 0,
                     out_samples_len = 0;

    RTI_TSFM_LOG_FN(RTI_TSFM_Transformation_transform)

    if (self->read_buffer_loaned) {
        RTI_TSFM_ERROR_1(
                "read ALREADY in progress on transformation:",
                "%p",
                self)
        goto done;
    }

    read_buffer_max =
            RTI_TSFM_DDS_DynamicDataPtrSeq_get_maximum(&self->read_buffer);

    out_samples_len = (read_buffer_max > in_count) ? read_buffer_max : in_count;

    if (!RTI_TSFM_DDS_DynamicDataPtrSeq_ensure_length(
                &self->read_buffer,
                out_samples_len,
                out_samples_len)) {
        /* TODO Log error */
        goto done;
    }

    out_samples = RTI_TSFM_DDS_DynamicDataPtrSeq_get_contiguous_buffer(
            &self->read_buffer);
    if (out_samples == NULL) {
        /* TODO Log error */
        goto done;
    }

    for (i = 0; i < in_count; i++) {
        DDS_DynamicData *out_sample = out_samples[i],
                        *in_sample = in_samples[i];

        if (out_sample == NULL) {
            out_sample = DDS_DynamicDataTypeSupport_create_data(self->tsupport);
            out_samples[i] = out_sample;
        }

        if (in_sample == NULL || out_sample == NULL) {
            /* TODO Log error */
            goto done;
        }

        switch (self->config->type) {
        case RTI_TSFM_TransformationKind_SERIALIZER:
            if (DDS_RETCODE_OK
                != self->plugin->user_plugin->serialize_sample(
                        self->plugin->user_plugin,
                        self,
                        in_sample,
                        out_sample)) {
                /* TODO Log error */
                goto done;
            }
            break;

        case RTI_TSFM_TransformationKind_DESERIALIZER:
            if (DDS_RETCODE_OK
                != self->plugin->user_plugin->deserialize_sample(
                        self->plugin->user_plugin,
                        self,
                        in_sample,
                        out_sample)) {
                /* TODO Log error */
                goto done;
            }
            break;

        default:
            /* TODO Log error */
            goto done;
        }

        out_samples_initd += 1;
    }

    self->read_buffer_loaned = DDS_BOOLEAN_TRUE;

    *out_sample_lst = (void *) out_samples;
    *out_info_lst = in_info_lst;
    *out_count = out_samples_initd;

    retcode = DDS_RETCODE_OK;

done:

    if (retcode != DDS_RETCODE_OK) {
        if (out_samples == NULL && out_samples_initd > 0) {
            if (DDS_RETCODE_OK
                != RTI_TSFM_Transformation_return_loan(
                        self,
                        (RTI_RoutingServiceSample *) out_samples,
                        NULL,
                        out_samples_initd,
                        env)) {
                /* TODO Log error */
            }
        }
    }

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_Transformation_return_loan(
        RTI_TSFM_Transformation *self,
        RTI_RoutingServiceSample *sample_lst,
        RTI_RoutingServiceSampleInfo *info_lst,
        int count,
        RTI_RoutingServiceEnvironment *env)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong i = 0;
    DDS_DynamicData **out_samples = (DDS_DynamicData **) sample_lst;

    RTI_TSFM_LOG_FN(RTI_TSFM_Transformation_return_loan)

    if (!self->read_buffer_loaned) {
        RTI_TSFM_ERROR_1("read NOT in progress on transformation:", "%p", self)
    }

    for (i = 0; i < count; i++) {
        DDS_DynamicData *out_sample = out_samples[i];

        if (DDS_RETCODE_OK != DDS_DynamicData_clear_all_members(out_sample)) {
            /* TODO Log error */
        }
    }

    self->read_buffer_loaned = DDS_BOOLEAN_FALSE;

    retcode = DDS_RETCODE_OK;

done:

    return retcode;
}

DDS_ReturnCode_t RTI_TSFM_Transformation_update(
        RTI_RoutingServiceTransformation transformation,
        const struct RTI_RoutingServiceProperties *properties,
        RTI_RoutingServiceEnvironment *env)
{
    RTI_TSFM_LOG_FN(RTI_TSFM_Transformation_update)
    /* TODO Implement me */
    return DDS_RETCODE_ERROR;
}


RTIBool RTI_TSFM_TransformationPtr_initialize_w_params(
        RTI_TSFM_Transformation **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}


RTIBool RTI_TSFM_TransformationPtr_finalize_w_params(
        RTI_TSFM_Transformation **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_TSFM_TransformationPtr_copy(
        RTI_TSFM_Transformation **dst,
        const RTI_TSFM_Transformation **src)
{
    *dst = (RTI_TSFM_Transformation *) *src;
    return RTI_TRUE;
}

RTIBool RTI_TSFM_TransformationPluginConfig_copy(
        RTI_TSFM_TransformationPluginConfig *dst,
        const RTI_TSFM_TransformationPluginConfig *src)
{
    if (dst == NULL || src == NULL) {
        return RTI_FALSE;
    }

    if (!RTICdrType_copyStringEx(
                &dst->dll,
                src->dll,
                (RTIXCdrLong_MAX - 1) + 1,
                RTI_TRUE)) {
        return RTI_FALSE;
    }
    if (!RTICdrType_copyStringEx(
                &dst->create_fn,
                src->create_fn,
                (RTIXCdrLong_MAX - 1) + 1,
                RTI_TRUE)) {
        return RTI_FALSE;
    }

    return RTI_TRUE;
}

#define T RTI_TSFM_Transformation *
#define TSeq RTI_TSFM_TransformationPtrSeq
#define T_initialize_w_params RTI_TSFM_TransformationPtr_initialize_w_params
#define T_finalize_w_params RTI_TSFM_TransformationPtr_finalize_w_params
#define T_copy RTI_TSFM_TransformationPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T

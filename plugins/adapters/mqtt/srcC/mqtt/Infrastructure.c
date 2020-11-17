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

#include "Infrastructure.h"

#include <limits.h>

#if RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_POSIX
    #include <pthread.h>
#elif RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_WINDOWS
    #include <process.h>
    #include <windows.h>
#else

    #error "Please provide a thread implementation for this target platform"

#endif

#define RTI_MQTT_LOG_ARGS "RTI::MQTT::Infrastructure"

const RTI_MQTT_Time RTI_MQTT_Time_ZERO = RTI_MQTT_Time_INITIALIZER(0, 0);

const RTI_MQTT_Time RTI_MQTT_Time_INFINITE = RTI_MQTT_Time_INITIALIZER(-1, 0);

const RTI_MQTT_ClientConfig RTI_MQTT_ClientConfig_DEFAULT =
        RTI_MQTT_ClientConfig_INITIALIZER;

const RTI_MQTT_SubscriptionConfig RTI_MQTT_SubscriptionConfig_DEFAULT =
        RTI_MQTT_SubscriptionConfig_INITIALIZER;

const RTI_MQTT_PublicationConfig RTI_MQTT_PublicationConfig_DEFAULT =
        RTI_MQTT_PublicationConfig_INITIALIZER;

DDS_ReturnCode_t RTI_MQTT_Time_normalize(RTI_MQTT_Time *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong ns = 0;

    ns = self->nanoseconds / 1000000000;

    if (ns > 0) {
        /* Check if we would overflow adding ns to self->seconds*/
        if (INT_MAX - ns > self->seconds) {
            RTI_MQTT_TIME_OVERFLOW_DETECTED(self)
            goto done;
        }
        self->seconds += ns;
        self->nanoseconds = self->nanoseconds - (ns * 1000000000);
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}


DDS_ReturnCode_t RTI_MQTT_Time_to_seconds(RTI_MQTT_Time *self, int *seconds_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_Time self_norm = *self;

    if (DDS_RETCODE_OK != RTI_MQTT_Time_normalize(&self_norm)) {
        RTI_MQTT_TIME_NORMALIZE_FAILED(self)
        goto done;
    }

    *seconds_out = self_norm.seconds;

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Time_to_dds_time(
        RTI_MQTT_Time *self,
        struct DDS_Time_t *time_out)
{
    time_out->sec = self->seconds;
    time_out->nanosec = self->nanoseconds;
    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t RTI_MQTT_Time_to_dds_duration(
        RTI_MQTT_Time *self,
        struct DDS_Duration_t *dur_out)
{
    dur_out->sec = self->seconds;
    dur_out->nanosec = self->nanoseconds;
    return DDS_RETCODE_OK;
}


DDS_ReturnCode_t RTI_MQTT_DDS_OctetSeq_to_string(
        struct DDS_OctetSeq *self,
        char **str_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong buff_len = 0;
    const char *str_buff = NULL;
    char *str = NULL;

    if (sizeof(DDS_Octet) != sizeof(char)) {
        RTI_MQTT_INVALID_SIZE_OF_CHAR_DETECTED(sizeof(DDS_Octet))
        goto done;
    }
    buff_len = DDS_OctetSeq_get_length(self);
    str_buff = (const char *) DDS_OctetSeq_get_contiguous_buffer(self);
    str = (char *) RTI_MQTT_Heap_allocate(sizeof(char) * (buff_len + 1));
    if (str == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(sizeof(char) * (buff_len + 1))
        goto done;
    }

    RTI_MQTT_Memory_copy(str, str_buff, sizeof(char) * buff_len);
    str[buff_len] = '\0';

    *str_out = str;

    retval = DDS_RETCODE_OK;
done:

    if (retval != DDS_RETCODE_OK) {
        if (str != NULL) {
            RTI_MQTT_Heap_free(str);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Thread_spawn(
        RTI_MQTT_ThreadFn thread,
        void *arg,
        void **handle_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
#if RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_POSIX
    pthread_t *native_handle;
    pthread_attr_t pthread_attr;
    int rc = -1;

    RTI_MQTT_LOG_FN(RTI_MQTT_Thread_spawn)

    native_handle = (pthread_t *) RTI_MQTT_Heap_allocate(sizeof(pthread_t));
    if (native_handle == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(sizeof(pthread_t))
        goto done;
    }

    rc = pthread_attr_init(&pthread_attr);
    if (rc != 0) {
        RTI_MQTT_PTHREAD_ATTR_INIT_FAILED(rc)
        goto done;
    }

    if (handle_out == NULL) {
        rc = pthread_attr_setdetachstate(
                &pthread_attr,
                PTHREAD_CREATE_DETACHED);
        if (rc != 0) {
            RTI_MQTT_PTHREAD_SET_DETACHED_FAILED(rc)
            goto done;
        }
    }

    rc = pthread_create(native_handle, &pthread_attr, thread, arg);
    if (rc != 0) {
        RTI_MQTT_PTHREAD_CREATE_FAILED(rc)
        goto done;
    }

    rc = pthread_attr_destroy(&pthread_attr);
    if (rc != 0) {
        RTI_MQTT_PTHREAD_ATTR_DESTROY_FAILED(rc)
        goto done;
    }

    if (handle_out != NULL) {
        *handle_out = native_handle;
    }

#elif RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_WINDOWS
    HANDLE *native_handle = NULL;

    native_handle = (HANDLE *) RTI_MQTT_Heap_allocate(sizeof(HANDLE));
    if (native_handle == NULL) {
        RTI_MQTT_HEAP_ALLOCATE_FAILED(sizeof(HANDLE))
        goto done;
    }

    *native_handle =
            (HANDLE) _beginthread((void(__cdecl *)(void *)) thread, 0, arg);
    if (*native_handle == NULL) {
        RTI_MQTT_WIN_BEGIN_THREAD_FAILED()
        goto done;
    }

    if (handle_out != NULL) {
        *handle_out = native_handle;
    }
#endif
    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Thread_join(void *handle, void **result_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
#if RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_POSIX
    pthread_t *native_handle = (pthread_t *) handle;
    void *result = NULL;
    int rc = -1;

    RTI_MQTT_LOG_FN(RTI_MQTT_Thread_join)

    rc = pthread_join(*native_handle, &result);
    if (rc != 0) {
        RTI_MQTT_PTHREAD_JOIN_FAILED(native_handle, rc)
        goto done;
    }

    if (result_out != NULL) {
        *result_out = result;
    }


#elif RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_WINDOWS
    HANDLE *native_handle = (HANDLE *) handle;

    /* TODO Implement me */

    goto done;

#endif

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_TopicFilter_match(
        const char *filter,
        const char *value,
        DDS_Boolean *match_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_UnsignedLong f_i = 0, v_i = 0, value_len = 0, filter_len = 0;
    char f_ch = '\0', f_ch_next = '\0', v_ch = '\0';
    DDS_Boolean match = DDS_BOOLEAN_TRUE, match_done = DDS_BOOLEAN_FALSE;

    RTI_MQTT_LOG_FN(RTI_MQTT_TopicFilter_match)

    *match_out = DDS_BOOLEAN_FALSE;

    filter_len = RTI_MQTT_String_length(filter);
    value_len = RTI_MQTT_String_length(value);

    for (f_i = 0, v_i = 0;
         match && !match_done && f_i < filter_len && v_i < value_len;
         f_i++, v_i++) {
        f_ch = filter[f_i];
        f_ch_next = filter[f_i + 1];
        v_ch = value[v_i];

        switch (f_ch) {
        case '#': {
            /* Consume '#', assume next character in `filter`
               is either `/' or '\0' */
            if (f_ch_next != '/' && f_ch_next != '\0') {
                /* Invalid filter */
                goto done;
            }

            f_i++;
            f_ch = f_ch_next;

            /* The current character in `value` should not be a `/`
               (assuming empty path elements are not allowed, nor
               leading or trailing slashes e.g. "foo//bar/baz", "/foo/bar",
               "foo/bar/"). */
            if (v_ch == '/') {
#if 0
                match = DDS_BOOLEAN_FALSE;
                match_done = DDS_BOOLEAN_FALSE;
                break;
#endif
                goto done;
            }

            /* If we're at the end of `filter`, then we consume the
               rest of `value`. Basically, we're done, and it's a match. */
            if (f_ch == '\0') {
                v_i = value_len - 1;
                match_done = DDS_BOOLEAN_TRUE;
            } else {
                /* Consume `value` until we encounter '/' or
                   we consume it all */
                while (v_ch != '/' && v_i + 1 < value_len) {
                    v_i++;
                    v_ch = value[v_i];
                }
                /* If we consumed all of `value` without finding '/' then
                   we're done, it's not a match. Otherwise we can movo on
                   to the next character in `filter` and `value` */
                match = (v_ch == '/');
                match_done = !match;
            }
            break;
        }
        case '+': {
            /* Consume '+', assume next character in `filter`
               is either `/' or '\0' */
            if (f_ch_next != '/' && f_ch_next != '\0') {
                /* Invalid filter */
                goto done;
            }

            f_i++;
            f_ch = f_ch_next;

            /* The current character in `value` should not be a `/`
               (assuming empty path elements are not allowed, nor
               leading or trailing slashes e.g. "foo//bar/baz", "/foo/bar",
               "foo/bar/"). */
            if (v_ch == '/') {
                match = DDS_BOOLEAN_FALSE;
                match_done = DDS_BOOLEAN_TRUE;
                break;
            }
            /* Consume `value` until we find `/` or reach the end
               of the string */
            while (v_ch != '/' && v_ch != '\0') {
                v_i++;
                v_ch = value[v_i];
            }

            if (f_ch == '\0') {
                match = (v_ch == '\0');
                match_done = DDS_BOOLEAN_TRUE;
            } else {
                match = (v_ch == '/');
                match_done = !match;
            }

            break;
        }
        default: {
            /* Current character in `filter` must be equal to current
               character in `value` */
            match = (f_ch == v_ch) ? DDS_BOOLEAN_TRUE : DDS_BOOLEAN_FALSE;
            match_done = !match;
            break;
        }
        }
    }

    /* We must have consumed all of `value` */
    match = match && (v_i == value_len);

    *match_out = match;

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t
        RTI_MQTT_QosLevel_to_mqtt_qos(RTI_MQTT_QosLevel level, int *mqtt_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    switch (level) {
    case RTI_MQTT_QosLevel_ZERO:
        *mqtt_out = 0;
        break;
    case RTI_MQTT_QosLevel_ONE:
        *mqtt_out = 1;
        break;
    case RTI_MQTT_QosLevel_TWO:
        *mqtt_out = 2;
        break;
    default:
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_QosLevel_from_mqtt_qos(
        int mqtt_qos,
        RTI_MQTT_QosLevel *level_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    switch (mqtt_qos) {
    case 0:
        *level_out = RTI_MQTT_QosLevel_ZERO;
        break;
    case 1:
        *level_out = RTI_MQTT_QosLevel_ONE;
        break;
    case 2:
        *level_out = RTI_MQTT_QosLevel_TWO;
        break;
    default:
        goto done;
    }

    retval = DDS_RETCODE_OK;

done:
    return retval;
}


DDS_ReturnCode_t
        RTI_MQTT_ClientConfig_default(RTI_MQTT_ClientConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_MQTT_ClientConfig *config = *config_out;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_ClientConfig_new(DDS_BOOLEAN_FALSE, &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    if (!RTI_MQTT_ClientConfig_copy(config, &RTI_MQTT_ClientConfig_DEFAULT)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_MQTT_ClientConfig",
                config,
                &RTI_MQTT_ClientConfig_DEFAULT)
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (config != NULL && config_allocd) {
            RTI_MQTT_ClientConfig_delete(config);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_ClientConfig_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_ClientConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_ClientConfig *config = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientConfig_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    config = RTI_MQTT_ClientConfigTypeSupport_create_data_w_params(
            &alloc_params);
    if (config == NULL) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:

    return retval;
}

void RTI_MQTT_ClientConfig_delete(RTI_MQTT_ClientConfig *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_ClientConfig_delete)
    RTI_MQTT_ClientConfigTypeSupport_delete_data(self);
}

DDS_ReturnCode_t RTI_MQTT_ClientStatus_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_ClientStatus **state_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_ClientStatus *state = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_MQTT_ClientStatus_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    state = RTI_MQTT_ClientStatusTypeSupport_create_data_w_params(
            &alloc_params);
    if (state == NULL) {
        /* TODO Log error */
        goto done;
    }

    *state_out = state;

    retval = DDS_RETCODE_OK;
done:

    return retval;
}

void RTI_MQTT_ClientStatus_delete(RTI_MQTT_ClientStatus *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_ClientStatus_delete)
    RTI_MQTT_ClientStatusTypeSupport_delete_data(self);
}


DDS_ReturnCode_t RTI_MQTT_SubscriptionConfig_default(
        RTI_MQTT_SubscriptionConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_MQTT_SubscriptionConfig *config = *config_out;

    RTI_MQTT_LOG_FN(RTI_MQTT_SubscriptionConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_SubscriptionConfig_new(DDS_BOOLEAN_FALSE, &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    if (!RTI_MQTT_SubscriptionConfig_copy(
                config,
                &RTI_MQTT_SubscriptionConfig_DEFAULT)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_MQTT_SubscriptionConfig",
                config,
                &RTI_MQTT_SubscriptionConfig_DEFAULT)
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (config != NULL && config_allocd) {
            RTI_MQTT_SubscriptionConfig_delete(config);
        }
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_SubscriptionConfig_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_SubscriptionConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_SubscriptionConfig *config = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_MQTT_SubscriptionConfig_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    config = RTI_MQTT_SubscriptionConfigTypeSupport_create_data_w_params(
            &alloc_params);
    if (config == NULL) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:

    return retval;
}

void RTI_MQTT_SubscriptionConfig_delete(RTI_MQTT_SubscriptionConfig *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_SubscriptionConfig_delete)
    RTI_MQTT_SubscriptionConfigTypeSupport_delete_data(self);
}

DDS_ReturnCode_t RTI_MQTT_SubscriptionStatus_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_SubscriptionStatus **state_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_SubscriptionStatus *state = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_MQTT_SubscriptionStatus_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    state = RTI_MQTT_SubscriptionStatusTypeSupport_create_data_w_params(
            &alloc_params);
    if (state == NULL) {
        /* TODO Log error */
        goto done;
    }

    *state_out = state;

    retval = DDS_RETCODE_OK;
done:

    return retval;
}

void RTI_MQTT_SubscriptionStatus_delete(RTI_MQTT_SubscriptionStatus *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_SubscriptionStatus_delete)
    RTI_MQTT_SubscriptionStatusTypeSupport_delete_data(self);
}


DDS_ReturnCode_t RTI_MQTT_PublicationConfig_default(
        RTI_MQTT_PublicationConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DDS_Boolean config_allocd = DDS_BOOLEAN_FALSE;
    RTI_MQTT_PublicationConfig *config = *config_out;

    RTI_MQTT_LOG_FN(RTI_MQTT_PublicationConfig_default)

    if (config == NULL) {
        if (DDS_RETCODE_OK
            != RTI_MQTT_PublicationConfig_new(DDS_BOOLEAN_FALSE, &config)) {
            /* TODO Log error */
            goto done;
        }
        config_allocd = DDS_BOOLEAN_TRUE;
    }

    if (!RTI_MQTT_PublicationConfig_copy(
                config,
                &RTI_MQTT_PublicationConfig_DEFAULT)) {
        RTI_MQTT_LOG_COPY_DATA_FAILED(
                "RTI_MQTT_PublicationConfig",
                config,
                &RTI_MQTT_PublicationConfig_DEFAULT)
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:
    if (DDS_RETCODE_OK != retval) {
        if (config != NULL && config_allocd) {
            RTI_MQTT_PublicationConfig_delete(config);
        }
    }

    return retval;
}


DDS_ReturnCode_t RTI_MQTT_PublicationConfig_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_PublicationConfig **config_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_PublicationConfig *config = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_MQTT_PublicationConfig_new)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = allocate_optional;

    config = RTI_MQTT_PublicationConfigTypeSupport_create_data_w_params(
            &alloc_params);
    if (config == NULL) {
        /* TODO Log error */
        goto done;
    }

    *config_out = config;

    retval = DDS_RETCODE_OK;
done:

    return retval;
}

void RTI_MQTT_PublicationConfig_delete(RTI_MQTT_PublicationConfig *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_PublicationConfig_delete)
    RTI_MQTT_PublicationConfigTypeSupport_delete_data(self);
}

DDS_ReturnCode_t RTI_MQTT_PublicationStatus_new(
        DDS_Boolean allocate_optional,
        RTI_MQTT_PublicationStatus **state_out)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    RTI_MQTT_PublicationStatus *state = NULL;
    struct DDS_TypeAllocationParams_t alloc_params =
            DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

    RTI_MQTT_LOG_FN(RTI_MQTT_PublicationConfig_delete)

    alloc_params.allocate_memory = RTI_TRUE;
    alloc_params.allocate_pointers = RTI_TRUE;
    alloc_params.allocate_optional_members = RTI_TRUE;

    state = RTI_MQTT_PublicationStatusTypeSupport_create_data_w_params(
            &alloc_params);
    if (state == NULL) {
        /* TODO Log error */
        goto done;
    }

    *state_out = state;

    retval = DDS_RETCODE_OK;
done:

    return retval;
}

void RTI_MQTT_PublicationStatus_delete(RTI_MQTT_PublicationStatus *self)
{
    RTI_MQTT_LOG_FN(RTI_MQTT_PublicationConfig_delete)
    RTI_MQTT_PublicationStatusTypeSupport_delete_data(self);
}


RTIBool RTI_MQTT_MessagePtr_initialize_w_params(
        struct RTI_MQTT_Message **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_MessagePtr_finalize_w_params(
        struct RTI_MQTT_Message **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_MQTT_MessagePtr_copy(
        struct RTI_MQTT_Message **dst,
        const struct RTI_MQTT_Message **src)
{
    *dst = (struct RTI_MQTT_Message *) *src;
    return RTI_TRUE;
}

#define T struct RTI_MQTT_Message *
#define TSeq RTI_MQTT_MessagePtrSeq
#define T_initialize_w_params RTI_MQTT_MessagePtr_initialize_w_params
#define T_finalize_w_params RTI_MQTT_MessagePtr_finalize_w_params
#define T_copy RTI_MQTT_MessagePtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T


static RTIBool RTI_MQTT_DDS_DynamicDataPtr_initialize_w_params(
        DDS_DynamicData **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

static RTIBool RTI_MQTT_DDS_DynamicDataPtr_finalize_w_params(
        DDS_DynamicData **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

static RTIBool RTI_MQTT_DDS_DynamicDataPtr_copy(
        DDS_DynamicData **dst,
        const DDS_DynamicData **src)
{
    *dst = (DDS_DynamicData *) *src;
    return RTI_TRUE;
}

#define T DDS_DynamicData *
#define TSeq RTI_MQTT_DDS_DynamicDataPtrSeq
#define T_initialize_w_params RTI_MQTT_DDS_DynamicDataPtr_initialize_w_params
#define T_finalize_w_params RTI_MQTT_DDS_DynamicDataPtr_finalize_w_params
#define T_copy RTI_MQTT_DDS_DynamicDataPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"

#if RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_POSIX

DDS_ReturnCode_t RTI_MQTT_Mutex_initialize(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    pthread_mutexattr_t mutex_attr;
    DDS_Boolean attr_initd = DDS_BOOLEAN_FALSE;

    if (0 != pthread_mutexattr_init(&mutex_attr)) {
        /* TODO Log error */
        goto done;
    }

    if (0 != pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE)) {
        /* TODO Log error */
        goto done;
    }

    if (0 != pthread_mutex_init(self, &mutex_attr)) {
        RTI_MQTT_ERROR_1("failed to initialize mutex:", "mutex=%p", self)
        goto done;
    }

    RTI_MQTT_TRACE_1("NEW mutex:", "mutex=%p", self)

    retval = DDS_RETCODE_OK;
done:
    if (attr_initd) {
        pthread_mutexattr_destroy(&mutex_attr);
    }

    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Mutex_finalize(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    if (0 != pthread_mutex_destroy(self)) {
        RTI_MQTT_ERROR_1("failed to delete mutex:", "mutex=%p", self)
        goto done;
    }

    RTI_MQTT_TRACE_1("DELETED mutex:", "mutex=%p", self)

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Mutex_take(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    if (0 != pthread_mutex_lock(self)) {
        RTI_MQTT_ERROR_1("failed to take mutex:", "mutex=%p", self)
        goto done;
    }
    RTI_MQTT_TRACE_2(
            "TAKEN mutex:",
            "mutex=%p, thread=%llu",
            self,
            RTIOsapiThread_getCurrentThreadID())

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Mutex_give(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    if (0 != pthread_mutex_unlock(self)) {
        RTI_MQTT_ERROR_1("failed to release mutex:", "mutex=%p", self)
        goto done;
    }

    RTI_MQTT_TRACE_2(
            "GIVEN mutex:",
            "mutex=%p, thread=%llu",
            self,
            RTIOsapiThread_getCurrentThreadID())

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

#elif RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_WINDOWS

DDS_ReturnCode_t RTI_MQTT_Mutex_initialize(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    *self = CreateMutex(NULL, 0, NULL);

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Mutex_finalize(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    CloseHandle(*self);

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Mutex_take(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DWORD wait_res = 0;

    wait_res = WaitForSingleObject(*self, INFINITE);
    if (WAIT_OBJECT_0 != wait_res) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_MQTT_Mutex_give(RTI_MQTT_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    ReleaseMutex(*self);

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

#endif

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

#include "TransformationInfrastructure.h"

#define RTI_TSFM_LOG_ARGS "rtitransform::simple::infrastructure"

DDS_ReturnCode_t RTI_TSFM_realloc_buffer(
        DDS_UnsignedLong size_min,
        DDS_UnsignedLong size_incr,
        DDS_Long size_max,
        char **buffer_str,
        DDS_UnsignedLong *buffer_str_size)
{
    DDS_ReturnCode_t retcode = DDS_RETCODE_ERROR;
    DDS_UnsignedLong cur_len = 0;

    if (*buffer_str != NULL) {
        cur_len = *buffer_str_size;
        DDS_String_free(*buffer_str);
        *buffer_str = NULL;
    }

    if (cur_len == 0) {
        cur_len = size_min;
    } else {
        cur_len += size_incr;
    }

    if (size_max > 0 && cur_len > size_max) {
        /* TODO Log error */
        goto done;
    }

    *buffer_str = DDS_String_alloc(cur_len);
    if (*buffer_str == NULL) {
        /* TODO Log error */
        goto done;
    }

    (*buffer_str)[0] = '\0';
    *buffer_str_size = cur_len;

    retcode = DDS_RETCODE_OK;

done:
    if (retcode != DDS_RETCODE_OK) {
        if (*buffer_str != NULL) {
            DDS_String_free(*buffer_str);
            *buffer_str = NULL;
        }
    }
    return retcode;
}

RTIBool RTI_TSFM_DDS_DynamicDataPtr_initialize_w_params(
        DDS_DynamicData **self,
        const struct DDS_TypeAllocationParams_t *allocParams)
{
    *self = NULL;
    return RTI_TRUE;
}


RTIBool RTI_TSFM_DDS_DynamicDataPtr_finalize_w_params(
        DDS_DynamicData **self,
        const struct DDS_TypeDeallocationParams_t *deallocParams)
{
    *self = NULL;
    return RTI_TRUE;
}

RTIBool RTI_TSFM_DDS_DynamicDataPtr_copy(
        DDS_DynamicData **dst,
        const DDS_DynamicData **src)
{
    *dst = (DDS_DynamicData *) *src;
    return RTI_TRUE;
}

#define T DDS_DynamicData *
#define TSeq RTI_TSFM_DDS_DynamicDataPtrSeq
#define T_initialize_w_params RTI_TSFM_DDS_DynamicDataPtr_initialize_w_params
#define T_finalize_w_params RTI_TSFM_DDS_DynamicDataPtr_finalize_w_params
#define T_copy RTI_TSFM_DDS_DynamicDataPtr_copy
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#undef T_copy
#undef T_finalize_w_params
#undef T_initialize_w_params
#undef TSeq
#undef T

#if RTI_TSFM_USE_MUTEX

    #if RTI_TSFM_PLATFORM == RTI_TSFM_PLATFORM_POSIX

DDS_ReturnCode_t RTI_TSFM_Mutex_initialize(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    if (0 != pthread_mutex_init(self, NULL)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_TSFM_Mutex_finalize(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    if (0 != pthread_mutex_destroy(self)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_TSFM_Mutex_take(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    if (0 != pthread_mutex_lock(self)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_TSFM_Mutex_give(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    if (0 != pthread_mutex_unlock(self)) {
        /* TODO Log error */
        goto done;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

    #elif RTI_TSFM_PLATFORM == RTI_TSFM_PLATFORM_WINDOWS

DDS_ReturnCode_t RTI_TSFM_Mutex_initialize(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    *self = CreateMutex(NULL, 0, NULL);

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_TSFM_Mutex_finalize(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    CloseHandle(*self);

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_TSFM_Mutex_take(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;
    DWORD wait_res = 0;

    wait_res = WaitForSingleObject(*self, INFINITE);
    if (WAIT_OBJECT_0 != wait_res) {
        /* TODO Log error */
        goto error;
    }

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

DDS_ReturnCode_t RTI_TSFM_Mutex_give(RTI_TSFM_Mutex *self)
{
    DDS_ReturnCode_t retval = DDS_RETCODE_ERROR;

    ReleaseMutex(*self);

    retval = DDS_RETCODE_OK;
done:
    return retval;
}

    #endif

#endif /* RTI_TSFM_USE_MUTEX */
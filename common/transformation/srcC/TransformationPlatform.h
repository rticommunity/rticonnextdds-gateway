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

#ifndef TransformationPlatform_h
#define TransformationPlatform_h

/*****************************************************************************
 *                          DDS C API Selection
 *****************************************************************************/
#include "ndds/ndds_c.h"

/*****************************************************************************
 *                       RTI Routing Service SDK
 *****************************************************************************/
#include "routingservice/routingservice_transformation.h"

/*****************************************************************************
 *                            DDS Type Definitions
 *****************************************************************************/
#include "TransformationTypes.h"

/*****************************************************************************
 *                   Target Build Platform Detection
 *****************************************************************************/

#define RTI_TSFM_PLATFORM_UNKNOWN 0
#define RTI_TSFM_PLATFORM_POSIX 1
#define RTI_TSFM_PLATFORM_WINDOWS 2

/* If an RTI_TSFM_PLATFORM wasn't specified to the compiler,
   then try to detect it automatically from known macros */
#ifndef RTI_TSFM_PLATFORM

    #if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) \
            || defined(__linux__) || defined(__QNXNTO__)

        #define RTI_TSFM_PLATFORM RTI_TSFM_PLATFORM_POSIX

    #elif defined(_MSC_VER) || defined(WIN32)

        #define RTI_TSFM_PLATFORM RTI_TSFM_PLATFORM_WINDOWS

    #else

        #error "failed to detect type of target platform. Please define RTI_TSFM_PLATFORM"

    #endif

#endif /* RTI_TSFM_PLATFORM */

#if RTI_TSFM_PLATFORM != RTI_TSFM_PLATFORM_POSIX \
        && RTI_TSFM_PLATFORM != RTI_TSFM_PLATFORM_WINDOWS
    #warning "unsupported target platform detected"
#endif

/*****************************************************************************
 *                          Platform helpers
 *****************************************************************************/

#if RTI_TSFM_PLATFORM == RTI_TSFM_PLATFORM_POSIX \
        || RTI_TSFM_PLATFORM == RTI_TSFM_PLATFORM_WINDOWS

    #define RTI_TSFM_String_length strlen
    #define RTI_TSFM_String_compare strcmp
    #define RTI_TSFM_String_to_long strtol
    #define RTI_TSFM_String_to_ulong strtoul
    #define RTI_TSFM_String_to_double strtod
    #define RTI_TSFM_Heap_allocate malloc
    #if 0
        #define RTI_TSFM_Heap_free(ptr_)                 \
            {                                            \
                RTI_TSFM_LOG_1("FREE", "ptr=%p", (ptr_)) \
                free((ptr_));                            \
            }
    #else
        #define RTI_TSFM_Heap_free free
    #endif
    #define RTI_TSFM_Memory_copy memcpy
    #define RTI_TSFM_Memory_set memset
    #define RTI_TSFM_Memory_compare memcmp
    #define RTI_TSFM_Memory_zero(ptr_, size_) (memset((ptr_), 0, (size_)))

#endif

#define RTI_TSFM_String_is_equal(s_, o_) \
    (((s_) == NULL && (o_) == NULL)      \
     || ((s_) != NULL && (o_) != NULL    \
         && RTI_TSFM_String_compare((s_), (o_)) == 0))

#define RTI_TSFM_lookup_property(pp_, p_, set_)                            \
    {                                                                      \
        const char *pval =                                                 \
                RTI_RoutingServiceProperties_lookup_property((pp_), (p_)); \
        if (pval != NULL) {                                                \
            set_                                                           \
        }                                                                  \
    }

/****************************************************************************/
/*                              Mutex utilities                            */
/****************************************************************************/

#if RTI_TSFM_PLATFORM == RTI_TSFM_PLATFORM_POSIX

    #include <pthread.h>

typedef pthread_mutex_t RTI_TSFM_Mutex;

#elif RTI_TSFM_PLATFORM == RTI_TSFM_PLATFORM_WINDOWS

typedef HANDLE RTI_TSFM_Mutex;

#endif

#ifdef RTI_TSFM_ENABLE_MUTEX
    #define RTI_TSFM_USE_MUTEX 1
#else
    #define RTI_TSFM_USE_MUTEX 0
#endif /* RTI_TSFM_ENABLE_MUTEX */

#if RTI_TSFM_USE_MUTEX
DDS_ReturnCode_t RTI_TSFM_Mutex_initialize(RTI_TSFM_Mutex *self);

DDS_ReturnCode_t RTI_TSFM_Mutex_finalize(RTI_TSFM_Mutex *self);

DDS_ReturnCode_t RTI_TSFM_Mutex_take(RTI_TSFM_Mutex *self);

DDS_ReturnCode_t RTI_TSFM_Mutex_give(RTI_TSFM_Mutex *self);
#endif /* RTI_TSFM_USE_MUTEX */

#endif /* TransformationPlatform_h */

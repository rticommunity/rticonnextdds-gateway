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

/**
 * @file rtiadapt_mqtt_log.h
 * @brief Header file for the MQTT Adapter's logging module.
 * 
 * This header file defines several macros that can be used to print out
 * messages with an optional format and a varying number of arguments.
 * 
 * The macros avoid use of "var args" to enable porting to platform which
 * do not support them.
 * 
 */

#ifndef rtiadapt_mqtt_log_h
#define rtiadapt_mqtt_log_h

/** 
 * @internal 
 */

#define RTI_MQTT_XSTR(x_)           #x_
#define RTI_MQTT_STRINGIFY(x_)      RTI_MQTT_XSTR(x_)

/*****************************************************************************
 *
 *****************************************************************************/

#define RTI_MQTT_LOG_HEAD          "|| %s || I ||> "

#define RTI_MQTT_LOG_HEAD_WARNING  "|| %s || W ||> "

#define RTI_MQTT_LOG_HEAD_ERROR    "|| %s || E ||> "

#define RTI_MQTT_LOG_HEAD_TRACE    "|| %s || T ||> "

/*****************************************************************************
 *
 *****************************************************************************/

#if RTI_MQTT_USE_LOG

#define RTI_MQTT_LOG(msg_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s\n", RTI_MQTT_LOG_ARGS, (msg_));

#define RTI_MQTT_LOG_1(msg_,fmt_,a1_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_));

#define RTI_MQTT_LOG_2(msg_,fmt_,a1_,a2_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_), (a2_));

#define RTI_MQTT_LOG_3(msg_,fmt_,a1_,a2_,a3_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_));

#define RTI_MQTT_LOG_4(msg_,fmt_,a1_,a2_,a3_,a4_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_));

#define RTI_MQTT_LOG_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_));

#define RTI_MQTT_LOG_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_));

#define RTI_MQTT_LOG_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_));

#define RTI_MQTT_LOG_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_));

#define RTI_MQTT_LOG_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), (a9_));

#define RTI_MQTT_LOG_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_));

#define RTI_MQTT_LOG_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_));

#define RTI_MQTT_LOG_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_));

#define RTI_MQTT_LOG_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_), \
            (a13_));

#define RTI_MQTT_LOG_16(msg_,fmt_,\
                   a1_,a2_,a3_,a4_,\
                   a5_,a6_,a7_,a8_,\
                   a9_,a10_,a11_,a12_,\
                   a13_,a14_,a15_,a16_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_), \
            (a13_), (a14_), (a15_), (a16_));

#define RTI_MQTT_WARNING(msg_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_WARNING "%s\n", RTI_MQTT_LOG_ARGS, (msg_));

#define RTI_MQTT_ERROR(msg_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_ERROR "%s\n", RTI_MQTT_LOG_ARGS, (msg_));

#define RTI_MQTT_ERROR_1(msg_,fmt_,a1_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_ERROR "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_));

#define RTI_MQTT_ERROR_2(msg_,fmt_,a1_,a2_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_ERROR "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_), (a2_));

#define RTI_MQTT_ERROR_3(msg_,fmt_,a1_,a2_,a3_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_ERROR "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_), (a2_), (a3_));

#define RTI_MQTT_ERROR_4(msg_,fmt_,a1_,a2_,a3_,a4_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_ERROR "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_), (a2_), (a3_), (a4_));

#define RTI_MQTT_ERROR_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_ERROR "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_), (a2_), (a3_), (a4_), (a5_));

#else

#define RTI_MQTT_LOG(msg_)
#define RTI_MQTT_LOG_1(msg_,fmt_,a1_)
#define RTI_MQTT_LOG_2(msg_,fmt_,a1_,a2_)
#define RTI_MQTT_LOG_3(msg_,fmt_,a1_,a2_,a3_)
#define RTI_MQTT_LOG_4(msg_,fmt_,a1_,a2_,a3_,a4_)
#define RTI_MQTT_LOG_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_)
#define RTI_MQTT_LOG_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_)
#define RTI_MQTT_LOG_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_)
#define RTI_MQTT_LOG_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_)
#define RTI_MQTT_LOG_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_)
#define RTI_MQTT_LOG_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_)
#define RTI_MQTT_LOG_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_)
#define RTI_MQTT_LOG_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_)
#define RTI_MQTT_LOG_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_)
#define RTI_MQTT_LOG_16(msg_,fmt_,\
                   a1_,a2_,a3_,a4_,\
                   a5_,a6_,a7_,a8_,\
                   a9_,a10_,a11_,a12_,\
                   a13_,a14_,a15_,a16_)
#define RTI_MQTT_WARNING(msg_)
#define RTI_MQTT_ERROR(msg_)
#define RTI_MQTT_ERROR_1(msg_,fmt_,a1_)
#define RTI_MQTT_ERROR_2(msg_,fmt_,a1_,a2_)
#define RTI_MQTT_ERROR_3(msg_,fmt_,a1_,a2_,a3_)
#define RTI_MQTT_ERROR_4(msg_,fmt_,a1_,a2_,a3_,a4_)
#define RTI_MQTT_ERROR_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_)

#endif /* RTI_MQTT_USE_LOG */

/*****************************************************************************
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *****************************************************************************/



#if RTI_MQTT_USE_TRACE

#define RTI_MQTT_TRACE(msg_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s\n",RTI_MQTT_LOG_ARGS, (msg_));

#define RTI_MQTT_TRACE_1(msg_,fmt_,a1_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_));

#define RTI_MQTT_TRACE_2(msg_,fmt_,a1_,a2_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), (a1_), (a2_));

#define RTI_MQTT_TRACE_3(msg_,fmt_,a1_,a2_,a3_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_));

#define RTI_MQTT_TRACE_4(msg_,fmt_,a1_,a2_,a3_,a4_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_));

#define RTI_MQTT_TRACE_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_));

#define RTI_MQTT_TRACE_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_));

#define RTI_MQTT_TRACE_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_));

#define RTI_MQTT_TRACE_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_));

#define RTI_MQTT_TRACE_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_), (a9_));

#define RTI_MQTT_TRACE_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_), (a9_), (a10_));

#define RTI_MQTT_TRACE_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_), (a9_), (a10_), (a11_));

#define RTI_MQTT_TRACE_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_));

#define RTI_MQTT_TRACE_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_) \
    fprintf(stdout,RTI_MQTT_LOG_HEAD "%s " fmt_ "\n", RTI_MQTT_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_), \
            (a13_));

#else

#define RTI_MQTT_TRACE(msg_)
#define RTI_MQTT_TRACE_1(msg_,fmt_,a1_)
#define RTI_MQTT_TRACE_2(msg_,fmt_,a1_,a2_)
#define RTI_MQTT_TRACE_3(msg_,fmt_,a1_,a2_,a3_)
#define RTI_MQTT_TRACE_4(msg_,fmt_,a1_,a2_,a3_,a4_)
#define RTI_MQTT_TRACE_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_)
#define RTI_MQTT_TRACE_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_)
#define RTI_MQTT_TRACE_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_)
#define RTI_MQTT_TRACE_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_)
#define RTI_MQTT_TRACE_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_)
#define RTI_MQTT_TRACE_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_)
#define RTI_MQTT_TRACE_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_)
#define RTI_MQTT_TRACE_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_)
#define RTI_MQTT_TRACE_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_)

#endif /* RTI_MQTT_USE_TRACE */

#define RTI_MQTT_LOG_FN(fn_)         RTI_MQTT_TRACE_1("CALL","%s",RTI_MQTT_STRINGIFY(fn_))

#define RTI_MQTT_LOG_GUID_FMT   "%02X%02X%02X%02X.%02X%02X%02X%02X."\
                            "%02X%02X%02X%02X.%02X%02X%02X%02X"
#define RTI_MQTT_LOG_GUID_ARGS(g_) \
    RTI_MQTT_LOG_GUID_BIT(g_,0), RTI_MQTT_LOG_GUID_BIT(g_,1), RTI_MQTT_LOG_GUID_BIT(g_,2), RTI_MQTT_LOG_GUID_BIT(g_,3), \
    RTI_MQTT_LOG_GUID_BIT(g_,4), RTI_MQTT_LOG_GUID_BIT(g_,5), RTI_MQTT_LOG_GUID_BIT(g_,6), RTI_MQTT_LOG_GUID_BIT(g_,7), \
    RTI_MQTT_LOG_GUID_BIT(g_,8), RTI_MQTT_LOG_GUID_BIT(g_,9), RTI_MQTT_LOG_GUID_BIT(g_,10), RTI_MQTT_LOG_GUID_BIT(g_,11), \
    RTI_MQTT_LOG_GUID_BIT(g_,12), RTI_MQTT_LOG_GUID_BIT(g_,13), RTI_MQTT_LOG_GUID_BIT(g_,14), RTI_MQTT_LOG_GUID_BIT(g_,15)

#define RTI_MQTT_LOG_GUID_ARGS_BE(g_) \
    RTI_MQTT_LOG_GUID_BIT(g_,3), RTI_MQTT_LOG_GUID_BIT(g_,2), RTI_MQTT_LOG_GUID_BIT(g_,1), RTI_MQTT_LOG_GUID_BIT(g_,0), \
    RTI_MQTT_LOG_GUID_BIT(g_,7), RTI_MQTT_LOG_GUID_BIT(g_,6), RTI_MQTT_LOG_GUID_BIT(g_,5), RTI_MQTT_LOG_GUID_BIT(g_,4), \
    RTI_MQTT_LOG_GUID_BIT(g_,11), RTI_MQTT_LOG_GUID_BIT(g_,10), RTI_MQTT_LOG_GUID_BIT(g_,9), RTI_MQTT_LOG_GUID_BIT(g_,8), \
    RTI_MQTT_LOG_GUID_BIT(g_,15), RTI_MQTT_LOG_GUID_BIT(g_,14), RTI_MQTT_LOG_GUID_BIT(g_,13), RTI_MQTT_LOG_GUID_BIT(g_,12)

#define RTI_MQTT_LOG_GUID_BIT(g_,i_) \
    ((unsigned char)((char*)(g_))[i_])

#define RTI_MQTT_LOG_GUID(msg_,g_) \
    RTI_MQTT_LOG_16(msg_,RTI_MQTT_LOG_GUID_FMT,\
        RTI_MQTT_LOG_GUID_BIT(g_,0), RTI_MQTT_LOG_GUID_BIT(g_,1), RTI_MQTT_LOG_GUID_BIT(g_,2), RTI_MQTT_LOG_GUID_BIT(g_,3), \
        RTI_MQTT_LOG_GUID_BIT(g_,4), RTI_MQTT_LOG_GUID_BIT(g_,5), RTI_MQTT_LOG_GUID_BIT(g_,6), RTI_MQTT_LOG_GUID_BIT(g_,7), \
        RTI_MQTT_LOG_GUID_BIT(g_,8), RTI_MQTT_LOG_GUID_BIT(g_,9), RTI_MQTT_LOG_GUID_BIT(g_,10), RTI_MQTT_LOG_GUID_BIT(g_,11), \
        RTI_MQTT_LOG_GUID_BIT(g_,12), RTI_MQTT_LOG_GUID_BIT(g_,13), RTI_MQTT_LOG_GUID_BIT(g_,14), RTI_MQTT_LOG_GUID_BIT(g_,15))

#define RTI_MQTT_LOG_GUID_BE(msg_,g_) \
    RTI_MQTT_LOG_16(msg_,RTI_MQTT_LOG_GUID_FMT,\
        RTI_MQTT_LOG_GUID_BIT(g_,3), RTI_MQTT_LOG_GUID_BIT(g_,2), RTI_MQTT_LOG_GUID_BIT(g_,1), RTI_MQTT_LOG_GUID_BIT(g_,0), \
        RTI_MQTT_LOG_GUID_BIT(g_,7), RTI_MQTT_LOG_GUID_BIT(g_,6), RTI_MQTT_LOG_GUID_BIT(g_,5), RTI_MQTT_LOG_GUID_BIT(g_,4), \
        RTI_MQTT_LOG_GUID_BIT(g_,11), RTI_MQTT_LOG_GUID_BIT(g_,10), RTI_MQTT_LOG_GUID_BIT(g_,9), RTI_MQTT_LOG_GUID_BIT(g_,8), \
        RTI_MQTT_LOG_GUID_BIT(g_,15), RTI_MQTT_LOG_GUID_BIT(g_,14), RTI_MQTT_LOG_GUID_BIT(g_,13), RTI_MQTT_LOG_GUID_BIT(g_,12))

/** @endinternal */

#include "rtiadapt_mqtt_log_messages.h"

#endif /* rtiadapt_mqtt_log_h */


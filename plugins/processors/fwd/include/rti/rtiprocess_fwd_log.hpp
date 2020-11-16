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

#ifndef rtiprocess_fwd_log_hpp
#define rtiprocess_fwd_log_hpp

#include <stdio.h>

#ifndef RTI_PRCS_FWD_ENABLE_LOG
#define RTI_PRCS_FWD_USE_LOG                    0
#else
#define RTI_PRCS_FWD_USE_LOG                    1
#endif

#ifndef RTI_PRCS_FWD_ENABLE_TRACE
#define RTI_PRCS_FWD_USE_TRACE                  0
#else
#define RTI_PRCS_FWD_USE_TRACE                  1
#endif

#define RTI_PRCS_FWD_XSTR(x_)           #x_
#define RTI_PRCS_FWD_STRINGIFY(x_)      RTI_PRCS_FWD_XSTR(x_)

/*****************************************************************************
 *
 *****************************************************************************/

#define RTI_PRCS_FWD_LOG_HEAD          "|| %s || I ||> "

#define RTI_PRCS_FWD_LOG_HEAD_WARNING  "|| %s || W ||> "

#define RTI_PRCS_FWD_LOG_HEAD_ERROR    "|| %s || E ||> "

#define RTI_PRCS_FWD_LOG_HEAD_TRACE    "|| %s || T ||> "

/*****************************************************************************
 *
 *****************************************************************************/

#if RTI_PRCS_FWD_USE_LOG

#define RTI_PRCS_FWD_LOG(msg_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s\n", RTI_PRCS_FWD_LOG_ARGS, (msg_));

#define RTI_PRCS_FWD_LOG_1(msg_,fmt_,a1_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_));

#define RTI_PRCS_FWD_LOG_2(msg_,fmt_,a1_,a2_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_), (a2_));

#define RTI_PRCS_FWD_LOG_3(msg_,fmt_,a1_,a2_,a3_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_));

#define RTI_PRCS_FWD_LOG_4(msg_,fmt_,a1_,a2_,a3_,a4_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_));

#define RTI_PRCS_FWD_LOG_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_));

#define RTI_PRCS_FWD_LOG_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_));

#define RTI_PRCS_FWD_LOG_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_));

#define RTI_PRCS_FWD_LOG_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_));

#define RTI_PRCS_FWD_LOG_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), (a9_));

#define RTI_PRCS_FWD_LOG_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_));

#define RTI_PRCS_FWD_LOG_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_));

#define RTI_PRCS_FWD_LOG_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_));

#define RTI_PRCS_FWD_LOG_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_), \
            (a13_));

#define RTI_PRCS_FWD_LOG_16(msg_,fmt_,\
                   a1_,a2_,a3_,a4_,\
                   a5_,a6_,a7_,a8_,\
                   a9_,a10_,a11_,a12_,\
                   a13_,a14_,a15_,a16_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_), \
            (a13_), (a14_), (a15_), (a16_));

#define RTI_PRCS_FWD_WARNING(msg_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_WARNING "%s\n", RTI_PRCS_FWD_LOG_ARGS, (msg_));

#define RTI_PRCS_FWD_ERROR(msg_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_ERROR "%s\n", RTI_PRCS_FWD_LOG_ARGS, (msg_));

#define RTI_PRCS_FWD_ERROR_1(msg_,fmt_,a1_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_ERROR "%s:" fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_));

#define RTI_PRCS_FWD_ERROR_2(msg_,fmt_,a1_,a2_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_ERROR "%s:" fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_), (a2_));

#define RTI_PRCS_FWD_ERROR_3(msg_,fmt_,a1_,a2_,a3_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_ERROR "%s:" fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_), (a2_), (a3_));

#define RTI_PRCS_FWD_ERROR_4(msg_,fmt_,a1_,a2_,a3_,a4_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_ERROR "%s:" fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_), (a2_), (a3_), (a4_));

#define RTI_PRCS_FWD_ERROR_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_ERROR "%s:" fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_), (a2_), (a3_), (a4_), (a5_));

#else

#define RTI_PRCS_FWD_LOG(msg_)
#define RTI_PRCS_FWD_LOG_1(msg_,fmt_,a1_)
#define RTI_PRCS_FWD_LOG_2(msg_,fmt_,a1_,a2_)
#define RTI_PRCS_FWD_LOG_3(msg_,fmt_,a1_,a2_,a3_)
#define RTI_PRCS_FWD_LOG_4(msg_,fmt_,a1_,a2_,a3_,a4_)
#define RTI_PRCS_FWD_LOG_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_)
#define RTI_PRCS_FWD_LOG_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_)
#define RTI_PRCS_FWD_LOG_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_)
#define RTI_PRCS_FWD_LOG_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_)
#define RTI_PRCS_FWD_LOG_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_)
#define RTI_PRCS_FWD_LOG_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_)
#define RTI_PRCS_FWD_LOG_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_)
#define RTI_PRCS_FWD_LOG_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_)
#define RTI_PRCS_FWD_LOG_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_)
#define RTI_PRCS_FWD_LOG_16(msg_,fmt_,\
                   a1_,a2_,a3_,a4_,\
                   a5_,a6_,a7_,a8_,\
                   a9_,a10_,a11_,a12_,\
                   a13_,a14_,a15_,a16_)
#define RTI_PRCS_FWD_WARNING(msg_)
#define RTI_PRCS_FWD_ERROR(msg_)
#define RTI_PRCS_FWD_ERROR_1(msg_,fmt_,a1_)
#define RTI_PRCS_FWD_ERROR_2(msg_,fmt_,a1_,a2_)
#define RTI_PRCS_FWD_ERROR_3(msg_,fmt_,a1_,a2_,a3_)
#define RTI_PRCS_FWD_ERROR_4(msg_,fmt_,a1_,a2_,a3_,a4_)
#define RTI_PRCS_FWD_ERROR_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_)

#endif /* RTI_PRCS_FWD_USE_LOG */

/*****************************************************************************
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *****************************************************************************/



#if RTI_PRCS_FWD_USE_TRACE

#define RTI_PRCS_FWD_TRACE(msg_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s\n",RTI_PRCS_FWD_LOG_ARGS, (msg_));

#define RTI_PRCS_FWD_TRACE_1(msg_,fmt_,a1_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_));

#define RTI_PRCS_FWD_TRACE_2(msg_,fmt_,a1_,a2_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), (a1_), (a2_));

#define RTI_PRCS_FWD_TRACE_3(msg_,fmt_,a1_,a2_,a3_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_));

#define RTI_PRCS_FWD_TRACE_4(msg_,fmt_,a1_,a2_,a3_,a4_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_));

#define RTI_PRCS_FWD_TRACE_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_));

#define RTI_PRCS_FWD_TRACE_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_));

#define RTI_PRCS_FWD_TRACE_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_));

#define RTI_PRCS_FWD_TRACE_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_));

#define RTI_PRCS_FWD_TRACE_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_), (a9_));

#define RTI_PRCS_FWD_TRACE_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_), (a9_), (a10_));

#define RTI_PRCS_FWD_TRACE_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD_TRACE "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), (a5_), (a6_), (a7_), (a8_), (a9_), (a10_), (a11_));

#define RTI_PRCS_FWD_TRACE_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_));

#define RTI_PRCS_FWD_TRACE_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_) \
    fprintf(stdout,RTI_PRCS_FWD_LOG_HEAD "%s " fmt_ "\n", RTI_PRCS_FWD_LOG_ARGS, (msg_), \
            (a1_), (a2_), (a3_), (a4_), \
            (a5_), (a6_), (a7_), (a8_), \
            (a9_), (a10_), (a11_), (a12_), \
            (a13_));

#else

#define RTI_PRCS_FWD_TRACE(msg_)
#define RTI_PRCS_FWD_TRACE_1(msg_,fmt_,a1_)
#define RTI_PRCS_FWD_TRACE_2(msg_,fmt_,a1_,a2_)
#define RTI_PRCS_FWD_TRACE_3(msg_,fmt_,a1_,a2_,a3_)
#define RTI_PRCS_FWD_TRACE_4(msg_,fmt_,a1_,a2_,a3_,a4_)
#define RTI_PRCS_FWD_TRACE_5(msg_,fmt_,a1_,a2_,a3_,a4_,a5_)
#define RTI_PRCS_FWD_TRACE_6(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_)
#define RTI_PRCS_FWD_TRACE_7(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_)
#define RTI_PRCS_FWD_TRACE_8(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_)
#define RTI_PRCS_FWD_TRACE_9(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_)
#define RTI_PRCS_FWD_TRACE_10(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_)
#define RTI_PRCS_FWD_TRACE_11(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_)
#define RTI_PRCS_FWD_TRACE_12(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_)
#define RTI_PRCS_FWD_TRACE_13(msg_,fmt_,a1_,a2_,a3_,a4_,a5_,a6_,a7_,a8_,a9_,a10_,a11_,a12_,a13_)

#endif /* RTI_PRCS_FWD_USE_TRACE */

#define RTI_PRCS_FWD_LOG_FN(fn_)         RTI_PRCS_FWD_TRACE_1("CALL","%s",RTI_PRCS_FWD_STRINGIFY(fn_))

#define RTI_PRCS_FWD_LOG_GUID_FMT   "%02X%02X%02X%02X.%02X%02X%02X%02X."\
                            "%02X%02X%02X%02X.%02X%02X%02X%02X"
#define RTI_PRCS_FWD_LOG_GUID_ARGS(g_) \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,0), RTI_PRCS_FWD_LOG_GUID_BIT(g_,1), RTI_PRCS_FWD_LOG_GUID_BIT(g_,2), RTI_PRCS_FWD_LOG_GUID_BIT(g_,3), \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,4), RTI_PRCS_FWD_LOG_GUID_BIT(g_,5), RTI_PRCS_FWD_LOG_GUID_BIT(g_,6), RTI_PRCS_FWD_LOG_GUID_BIT(g_,7), \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,8), RTI_PRCS_FWD_LOG_GUID_BIT(g_,9), RTI_PRCS_FWD_LOG_GUID_BIT(g_,10), RTI_PRCS_FWD_LOG_GUID_BIT(g_,11), \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,12), RTI_PRCS_FWD_LOG_GUID_BIT(g_,13), RTI_PRCS_FWD_LOG_GUID_BIT(g_,14), RTI_PRCS_FWD_LOG_GUID_BIT(g_,15)

#define RTI_PRCS_FWD_LOG_GUID_ARGS_BE(g_) \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,3), RTI_PRCS_FWD_LOG_GUID_BIT(g_,2), RTI_PRCS_FWD_LOG_GUID_BIT(g_,1), RTI_PRCS_FWD_LOG_GUID_BIT(g_,0), \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,7), RTI_PRCS_FWD_LOG_GUID_BIT(g_,6), RTI_PRCS_FWD_LOG_GUID_BIT(g_,5), RTI_PRCS_FWD_LOG_GUID_BIT(g_,4), \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,11), RTI_PRCS_FWD_LOG_GUID_BIT(g_,10), RTI_PRCS_FWD_LOG_GUID_BIT(g_,9), RTI_PRCS_FWD_LOG_GUID_BIT(g_,8), \
    RTI_PRCS_FWD_LOG_GUID_BIT(g_,15), RTI_PRCS_FWD_LOG_GUID_BIT(g_,14), RTI_PRCS_FWD_LOG_GUID_BIT(g_,13), RTI_PRCS_FWD_LOG_GUID_BIT(g_,12)

#define RTI_PRCS_FWD_LOG_GUID_BIT(g_,i_) \
    ((unsigned char)((char*)(g_))[i_])

#define RTI_PRCS_FWD_LOG_GUID(msg_,g_) \
    RTI_PRCS_FWD_LOG_16(msg_,RTI_PRCS_FWD_LOG_GUID_FMT,\
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,0), RTI_PRCS_FWD_LOG_GUID_BIT(g_,1), RTI_PRCS_FWD_LOG_GUID_BIT(g_,2), RTI_PRCS_FWD_LOG_GUID_BIT(g_,3), \
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,4), RTI_PRCS_FWD_LOG_GUID_BIT(g_,5), RTI_PRCS_FWD_LOG_GUID_BIT(g_,6), RTI_PRCS_FWD_LOG_GUID_BIT(g_,7), \
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,8), RTI_PRCS_FWD_LOG_GUID_BIT(g_,9), RTI_PRCS_FWD_LOG_GUID_BIT(g_,10), RTI_PRCS_FWD_LOG_GUID_BIT(g_,11), \
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,12), RTI_PRCS_FWD_LOG_GUID_BIT(g_,13), RTI_PRCS_FWD_LOG_GUID_BIT(g_,14), RTI_PRCS_FWD_LOG_GUID_BIT(g_,15))

#define RTI_PRCS_FWD_LOG_GUID_BE(msg_,g_) \
    RTI_PRCS_FWD_LOG_16(msg_,RTI_PRCS_FWD_LOG_GUID_FMT,\
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,3), RTI_PRCS_FWD_LOG_GUID_BIT(g_,2), RTI_PRCS_FWD_LOG_GUID_BIT(g_,1), RTI_PRCS_FWD_LOG_GUID_BIT(g_,0), \
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,7), RTI_PRCS_FWD_LOG_GUID_BIT(g_,6), RTI_PRCS_FWD_LOG_GUID_BIT(g_,5), RTI_PRCS_FWD_LOG_GUID_BIT(g_,4), \
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,11), RTI_PRCS_FWD_LOG_GUID_BIT(g_,10), RTI_PRCS_FWD_LOG_GUID_BIT(g_,9), RTI_PRCS_FWD_LOG_GUID_BIT(g_,8), \
        RTI_PRCS_FWD_LOG_GUID_BIT(g_,15), RTI_PRCS_FWD_LOG_GUID_BIT(g_,14), RTI_PRCS_FWD_LOG_GUID_BIT(g_,13), RTI_PRCS_FWD_LOG_GUID_BIT(g_,12))

#endif /* rtiprocess_fwd_log_hpp */

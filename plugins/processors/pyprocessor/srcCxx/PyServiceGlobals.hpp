/******************************************************************************/
/* (c) 2021 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
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

#ifndef PYSERVICEGLOBALS_HPP
    #define PYSERVICEGLOBALS_HPP

#include <string>
#include <list>

#include "Python.h"
#include "osapi/osapi_thread.h"

namespace rti { namespace routing { namespace py {
class PyServiceGlobals {
public:
    static const std::string SERVICE_MODULE_NAME;
    static const std::string PROCESSOR_MODULE_NAME;

    static PyServiceGlobals& instance();

    void init_service();
    void init_processor();

    bool from_service();

    PyObject* proc_module();

    PyThreadState* assert_state();

    void cleanup_states();

    void set_signals();

    bool is_main();

private:
    PyServiceGlobals();
    ~PyServiceGlobals();
    bool service_initd_;
    /* Reference to the proc module */
    PyObject *proc_module_;
    /* Thread state management */
    RTIOsapiThreadTssFactory *tss_factory_;
    RTI_UINT32 state_key_;
    RTI_UINT64 main_thread_id_;
    PyThreadState *main_thread_state_;
    /* keeping all the created states for cleanup purposes */
    std::list<PyThreadState*> states_;

};


class PyGilScopedHandler {
public:

    PyGilScopedHandler(): needs_gil_(false), gstate_(PyGILState_UNLOCKED)
    {
        if (PyServiceGlobals::instance().from_service()) {
            if (!_Py_IsFinalizing()) {
                gstate_ = PyGILState_Ensure();
                needs_gil_ = true;
            }
        } else {
            PyEval_RestoreThread(PyServiceGlobals::instance().assert_state());
        }
    }

    ~PyGilScopedHandler()
    {
        if (PyServiceGlobals::instance().from_service()) {
            if (needs_gil_) {
                PyGILState_Release(gstate_);
            }
        } else {
            PyEval_SaveThread();
        }
    }

    PyGILState_STATE gstate_;
    bool needs_gil_;
};

} } }
#endif /* PYSERVICEGLOBALS_HPP */


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PyServiceGlobals.hpp
 * Author: asanchez
 *
 * Created on January 11, 2020, 4:53 PM
 */

#ifndef PYSERVICEGLOBALS_HPP
    #define PYSERVICEGLOBALS_HPP

#include "Python.h"
#include "NativeUtils.hpp"
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


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
#include <csignal>

#include "Python.h"

#include "PyServiceGlobals.hpp"
#include "PyService.hpp"

namespace rti { namespace routing { namespace py {

class SafeInitializer {
public:
    template <typename INIT_FUNC>
    static void safe_init(const INIT_FUNC& init_func)
    {
        static SafeInitializer _init(init_func);
    }
private:
    template <typename INIT_FUNC>
    SafeInitializer(const INIT_FUNC& init)
    {
        init();
    }
};

const std::string PyServiceGlobals::SERVICE_MODULE_NAME = "rti.routing.service";
const std::string PyServiceGlobals::PROCESSOR_MODULE_NAME = "rti.routing.proc";

PyServiceGlobals::PyServiceGlobals()
        :service_initd_(false),
        tss_factory_(RTIOsapiThread_createTssFactory()),
        state_key_(0),
        main_thread_id_(RTIOsapiThread_getCurrentThreadID())
{
    RTIOsapiThread_createKey(&state_key_, tss_factory_);
    bool release_thread =  false;
    if (!Py_IsInitialized()) {
        Py_Initialize();
        release_thread = true;
    }

    main_thread_state_ = PyThreadState_Get();
    RTIOsapiThread_setTss(state_key_, main_thread_state_);
    if (release_thread) {
        PyEval_SaveThread();
    }
    this->set_signals();
}


PyServiceGlobals::~PyServiceGlobals()
{
    if (!service_initd_) {
        PyThreadState *current_state = PyServiceGlobals::instance().assert_state();
        PyEval_RestoreThread(current_state);
        for (auto state : states_) {
            if (state != current_state) {
                PyThreadState_Clear(state);
                PyThreadState_Delete(state);
            }
        }

        /*
         * TODO: review the memory management of the python objects. It appears
         * that this will run the GC and delete unexpected objects that were
         * cycle referenced after an uncaught exception.
         *
         * Py_Finalize();
         */
    }

    if (tss_factory_ != NULL) {
        RTIOsapiThread_deleteKey(tss_factory_, state_key_);
        RTIOsapiThread_deleteTssFactory(tss_factory_);
    }
}

PyThreadState * PyServiceGlobals::assert_state()
{
    PyThreadState *state = reinterpret_cast<PyThreadState *>(
            RTIOsapiThread_getTss(state_key_));
    if (state == NULL) {
        state = PyThreadState_New(main_thread_state_->interp);
        RTIOsapiThread_setTss(state_key_, state);
        states_.push_back(state);
    }

    return state;
}

void PyServiceGlobals::cleanup_states()
{
    for (auto state : states_) {
        PyThreadState_Clear(state);
        PyThreadState_Delete(state);
    }
}

void PyServiceGlobals::set_signals()
{
#if RTI_WIN32
    /* We need to register the signal interruptions to get handled by RS */
    signal(SIGINT, NULL);
    signal(SIGINT, NULL);
    signal(SIGABRT, NULL);
#endif
}

bool PyServiceGlobals::is_main()
{
    return (RTIOsapiThread_getCurrentThreadID()  == main_thread_id_);
}

PyServiceGlobals& PyServiceGlobals::instance()
{
    static PyServiceGlobals _instance;

    return _instance;
}

void PyServiceGlobals::init_service()
{
    using rti::routing::py::PyService;
    using rti::routing::py::PyServiceType;
    using rti::routing::py::PyObjectGuard;

    SafeInitializer::safe_init([]() {
        PyObjectGuard module_name = PyUnicode_FromString(
                SERVICE_MODULE_NAME.c_str());
        PyObject *service_module = PyImport_GetModule(module_name.get());
        if (service_module == NULL
                || PyModule_GetDict(service_module) == NULL
                || PyDict_Size(PyModule_GetDict(service_module)) == 0) {
            throw dds::core::Error(
                    "error finding module="
                    + SERVICE_MODULE_NAME
                    + ". It must be imported from your application module.");
        }

        if (PyType_Ready(PyServiceType::type()) < 0) {
            PyErr_Print();
                    throw dds::core::Error(
                    "error initializing Python Service type");
        }
        Py_INCREF(PyServiceType::type());
        PyObjectGuard type_guard = (PyObject *) PyServiceType::type();
        if (PyModule_AddObject(
                service_module,
                PyServiceType::name().c_str(),
                type_guard.get()) == -1) {
            throw dds::core::Error(
                    "error adding type="
                    + PyServiceType::name());
        }

        type_guard.release();
        PyServiceGlobals::instance().service_initd_ = true;
    }
    );
}

void PyServiceGlobals::init_processor()
{
    SafeInitializer::safe_init([]() {
        // Obtain rti.routing.proc
        PyObjectGuard py_module_name = PyUnicode_FromString(
                PROCESSOR_MODULE_NAME.c_str());
        PyObject* proc_module = PyImport_GetModule(py_module_name.get());
        if (proc_module == NULL) {
            throw dds::core::Error(
                    "error finding module="
                    + PROCESSOR_MODULE_NAME
                    + ". It must be imported from your Processor module.");
        }
        PyServiceGlobals::instance().proc_module_ = proc_module;
    }
    );

}


bool PyServiceGlobals::from_service()
{
    return service_initd_;
}

PyObject* PyServiceGlobals::proc_module()
{
    return proc_module_;
}




} } }
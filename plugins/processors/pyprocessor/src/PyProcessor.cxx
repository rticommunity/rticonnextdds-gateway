/*
 * (c) 2018 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */
#include <stdio.h>
#include <stdlib.h>
#include <iterator>

#include "Python.h"
#include <dds/core/corefwd.hpp>

#include <rti/routing/processor/ProcessorPlugin.hpp>
#include <rti/routing/processor/Processor.hpp>
#include <dds/core/xtypes/DynamicData.hpp>
#include <dds/core/xtypes/StructType.hpp>

#include "PyServiceGlobals.hpp"
#include "NativeUtils.hpp"
#include "PyProcessor.hpp"
#include "PyInput.hpp"
#include "PySamples.hpp"
#include "PyOutput.hpp"

using namespace rti::routing;
using namespace rti::routing::processor;
using namespace rti::routing::adapter;
using namespace dds::core::xtypes;
using namespace dds::sub::status;


namespace rti { namespace routing { namespace py {

/*
 * --- PyProcessor -------------------------------------------------
 */
const char *PyProcessor_METHOD_NAMES[] = {
    "", /* NONE */
    "on_input_enabled", /*  INPUT_ENABLED */
    "on_input_disabled", /* INPUT_DISABLED */
    "on_output_enabled", /* OUTPUT_ENABLED */
    "on_output_disabled", /* OUTPUT_DISABLED */
    "on_start", /* ROUTE_STARTED */
    "on_stop", /* ROUTE_STOPPED */
    "on_run", /* ROUTE_RUNNING */
    "on_pause", /* ROUTE_PAUSED */
    "on_data_available", /* ROUTE_PAUSED */
    "on_periodic_action" /* ROUTE_PAUSED */
};


PyProcessor::PyProcessor(
        PyProcessorPlugin *plugin,
        PyObject *py_processor,
        PyRoute *py_route)
        : plugin_(plugin),
          py_processor_(py_processor),
          py_route_(py_route)
{
    RTIOsapiMemory_zero(&native_, sizeof(native_));
    /* initialize native implementation */
    native_.processor_data =
            static_cast<void*> (this);
    native_.on_route_event =
            PyProcessor::forward_on_route_event;
}

PyProcessor::~PyProcessor()
{
    Py_DECREF(py_processor_);
    delete py_route_;
}

RTI_RoutingServiceProcessor *
PyProcessor::create_native(
        PyProcessorPlugin *plugin,
        RTI_RoutingServiceRoute *native_route,
        const struct RTI_RoutingServiceProperties *native_properties,
        RTI_RoutingServiceEnvironment *environment)
{
    PyGilScopedHandler gil_handler;

    PyProcessor *forwarder = NULL;
    try {
        PyRoute *py_route = new PyRoute(native_route);        
        PyObjectGuard py_properties = from_native(native_properties);

        forwarder = new PyProcessor(
                plugin,
                plugin->create_processor(py_route, py_properties.get()),
                py_route);
    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "unexpected exception");
    }


    return (forwarder != NULL) ? forwarder->native() : NULL;
}

void PyProcessor::delete_native(
        PyProcessorPlugin *,
        RTI_RoutingServiceProcessor *native_processor,
        RTI_RoutingServiceEnvironment *environment)
{
    PyGilScopedHandler gil_handler;
    
    PyProcessor *processor_forwarder =
            static_cast<PyProcessor*> (native_processor->processor_data);
    try {
        delete processor_forwarder;
    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "unexpected exception");
    };
}

RTI_RoutingServiceProcessor* PyProcessor::native()
{
    return &native_;
}


void PyProcessor::forward_on_route_event(
        void *native_processor_data,
        RTI_RoutingServiceRouteEvent *native_route_event,
        RTI_RoutingServiceEnvironment *environment)
{   
    PyGilScopedHandler gil_handler;
    
    PyProcessor *forwarder =
            static_cast<PyProcessor*> (native_processor_data);

    try {
        if (forwarder->plugin_->property().autoreload()) {
            forwarder->plugin_->reload();
        }

        RTI_RoutingServiceRouteEventKind event_kind =
                RTI_RoutingServiceRouteEvent_get_kind(native_route_event);
        // build up wrapper objects based on the event
        switch (event_kind) {

        case RTI_ROUTING_SERVICE_ROUTE_EVENT_DATA_ON_INPUTS:
        case RTI_ROUTING_SERVICE_ROUTE_EVENT_PERIODIC_ACTION:
        {
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    PyProcessor_METHOD_NAMES[event_kind],
                    "O",
                    forwarder->py_route_) == NULL) {
                PyErr_Print();
                throw dds::core::Error(std::string(
                        PyProcessor_METHOD_NAMES[event_kind])
                        + ": error calling Python processor");
            }

        }
            break;

        case RTI_ROUTING_SERVICE_ROUTE_EVENT_INPUT_ENABLED:
        {
            void *affected_entity =
                    RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
            void *event_data =
                    RTI_RoutingServiceRouteEvent_get_event_data(native_route_event);
            PyObjectGuard py_input = new PyInput(
                    static_cast<PyInput::native_type *>(affected_entity),
                    *(static_cast<int32_t*>(event_data)));
            RTI_RoutingServiceInput_set_user_data(
                    static_cast<PyInput::native_type *> (affected_entity),
                    py_input.get());
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    PyProcessor_METHOD_NAMES[event_kind],
                    "OO",
                    forwarder->py_route_,
                    py_input.get()) == NULL) {
                PyErr_Print();
                throw dds::core::Error(std::string(
                        PyProcessor_METHOD_NAMES[event_kind])
                        + ": error calling Python processor");
            }

            py_input.release();
        }
            break;

        case RTI_ROUTING_SERVICE_ROUTE_EVENT_INPUT_DISABLED:
        {
            void *affected_entity =
                    RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
            PyInput::native_type *native_input =
                    static_cast<PyInput::native_type *> (affected_entity);
            PyObjectGuard py_input =
                    forwarder->py_route_->input(native_input);
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    PyProcessor_METHOD_NAMES[event_kind],
                    "OO",
                    forwarder->py_route_,
                    py_input.get()) == NULL) {
                PyErr_Print();
                py_input.release();
                throw dds::core::Error(std::string(
                        PyProcessor_METHOD_NAMES[event_kind])
                        + ": error calling Python processor");
            }

            RTI_RoutingServiceInput_set_user_data(native_input, NULL);
        }
            break;


        case RTI_ROUTING_SERVICE_ROUTE_EVENT_OUTPUT_ENABLED:
        {
            void *affected_entity =
                    RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
            void *event_data =
                    RTI_RoutingServiceRouteEvent_get_event_data(native_route_event);
            PyObjectGuard py_output = new PyOutput(
                    static_cast<PyOutput::native_type *>(affected_entity),
                    *(static_cast<int32_t*>(event_data)));
            RTI_RoutingServiceOutput_set_user_data(
                    static_cast<PyOutput::native_type *> (affected_entity),
                    py_output.get());
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    PyProcessor_METHOD_NAMES[event_kind],
                    "OO",
                    forwarder->py_route_,
                    py_output.get()) == NULL) {
                PyErr_Print();
                throw dds::core::Error(std::string(
                        PyProcessor_METHOD_NAMES[event_kind])
                        + ": error calling Python processor");
            }

            py_output.release();
        }
            break;


        case RTI_ROUTING_SERVICE_ROUTE_EVENT_OUTPUT_DISABLED:
        {
            void *affected_entity =
                    RTI_RoutingServiceRouteEvent_get_affected_entity(native_route_event);
            PyOutput::native_type *native_output =
                    static_cast<PyOutput::native_type *> (affected_entity);
            PyObjectGuard py_output =
                    forwarder->py_route_->output(native_output);
            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    PyProcessor_METHOD_NAMES[event_kind],
                    "OO",
                    forwarder->py_route_,
                    py_output.get()) == NULL) {
                PyErr_Print();
                py_output.release();
                throw dds::core::Error(std::string(
                        PyProcessor_METHOD_NAMES[event_kind])
                        + ": error calling Python processor");
            }

            RTI_RoutingServiceOutput_set_user_data(native_output, NULL);
        }
            break;

        case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_STARTED:
        case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_STOPPED:
        case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_RUNNING:
        case RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_PAUSED:
        {
            if (event_kind == RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_STARTED) {
                forwarder->py_route_->started(true);
            } else if (event_kind == RTI_ROUTING_SERVICE_ROUTE_EVENT_ROUTE_STOPPED) {
                forwarder->py_route_->started(false);
            }

            if (PyObject_CallMethod(
                    forwarder->py_processor_,
                    PyProcessor_METHOD_NAMES[event_kind],
                    "O",
                    forwarder->py_route_) == NULL) {
                PyErr_Print();
                throw dds::core::Error(std::string(
                        PyProcessor_METHOD_NAMES[event_kind])
                        + ": error calling Python processor");
            }
        }
            break;


        default:
            // nothing to do
            break;
        }

    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                "unexpected exception");
    }   
}


/*
 * --- PyProcessorPlugin --------------------------------------------------
 */
PyProcessorPluginProperty::PyProcessorPluginProperty()
        : module_autoreload_(false)
{

}


PyProcessorPluginProperty::PyProcessorPluginProperty(
        const std::string& class_name,
        const std::string& module_name = "")
        : create_function_(class_name),
        module_(module_name),
        module_autoreload_(false)
{

}


void PyProcessorPluginProperty::create_function(const std::string& class_name)
{
    create_function_ = class_name;
}

const std::string& PyProcessorPluginProperty::create_function() const
{
    return create_function_;
}

void PyProcessorPluginProperty::module(const std::string& module_name)
{
    module_ = module_name;
}

const std::string& PyProcessorPluginProperty::module() const
{
    return module_;
}

void PyProcessorPluginProperty::module_path(const std::string& module_path)
{
    module_path_ = module_path;
}

const std::string& PyProcessorPluginProperty::module_path() const
{
    return module_path_;
}

void PyProcessorPluginProperty::autoreload(bool value)
{
    module_autoreload_ = value;
}

bool PyProcessorPluginProperty::autoreload() const
{
    return module_autoreload_;
}


const std::string PyProcessorPlugin::BASE_PROCESSOR_TYPE_NAME =
        "Processor";

const std::string PyProcessorPlugin::MODULE_PROPERTY_NAME =
        "rti.routing.proc.py.module.name";

const std::string PyProcessorPlugin::MODULE_PATH_PROPERTY_NAME =
        "rti.routing.proc.py.module.path";

const std::string PyProcessorPlugin::MODULE_PATH_VALUE_DEFAULT = ".";

const std::string PyProcessorPlugin::CREATE_FUNCTION_PROPERTY_NAME =
        "rti.routing.proc.py.create_function";

const std::string PyProcessorPlugin::MODULE_AUTORELOAD_PROPERTY_NAME =
        "rti.routing.proc.py.module.autoreload";


PyProcessorPlugin::PyProcessorPlugin(
        const struct RTI_RoutingServiceProperties *native_properties)
        : pyproc_type_(NULL),
          create_function_(NULL)
{
    // Check module properties
    property_.module_path(MODULE_PATH_VALUE_DEFAULT);

//    if (PyServiceGlobals::instance().from_service()) {
//        //PyEval_RestoreThread(PyServiceGlobals::instance().assert_state());
//        PyGILState_Ensure();
//    }

    for (int i = 0; i < native_properties->count; i++) {
        if (MODULE_PATH_PROPERTY_NAME
                == native_properties->properties[i].name) {
            property_.module_path((char *) native_properties->properties[i].value);
        } else if (MODULE_PROPERTY_NAME
                == native_properties->properties[i].name) {
            property_.module((char *) native_properties->properties[i].value);
        } else if (CREATE_FUNCTION_PROPERTY_NAME
                == native_properties->properties[i].name) {
            property_.create_function((char *) native_properties->properties[i].value);
        } else if (MODULE_AUTORELOAD_PROPERTY_NAME
                == native_properties->properties[i].name) {
            RTIBool boolValue = false;

            if (*((char *) native_properties->properties[i].value) == '\0') {
                boolValue = true;
            } else if (!REDAString_strToBoolean(
                    (char *) native_properties->properties[i].value,
                    &boolValue)) {
                throw dds::core::Error(
                        "PyProcessorPlugin: invalid value for property name="
                        + std::string((char *) native_properties->properties[i].value));
            }
            property_.autoreload(boolValue ? true : false);
        }
    }

    if (property_.module().empty()) {
        throw dds::core::Error(
                "PyProcessorPlugin: module name must be provided");
    }

    if (property_.create_function().empty()) {
        throw dds::core::Error(
                "PyProcessorPlugin: Processor class name must be provided");
    }

    /* update python global path so it can find the user module */
    PyObject *sys_path = PySys_GetObject("path");
    assert(sys_path != NULL);
    PyObject *py_module_path = Py_BuildValue("s", property_.module_path().c_str());
    int append_result = PyList_Append(
            sys_path,
            py_module_path);
    Py_DECREF(py_module_path);
    if (append_result == -1) {
        PyErr_Print();
        throw dds::core::Error("PyProcessorPlugin: add module path");
    }

    load_module();

    /*
     * IMPORTANT: When running RS executable we need to relinquish the control
     * of this thread of the GIL. The thread that instantiates the
     * PyProcessorPlugin will be the 'main' thread and hence the one currently
     * onwing the GIL.
     */
//    if (PyServiceGlobals::instance().from_service()) {
//        PyGILState_Release(PyGILState_LOCKED);
//    } else {
//        PyEval_SaveThread();
//    }
}

const PyProcessorPluginProperty& PyProcessorPlugin::property() const
{
    return property_;
}

template<typename PYOBJECTTYPE>
void PyProcessorPlugin::add_type()
{
    if (PyType_Ready(PYOBJECTTYPE::type()) < 0) {
        PyErr_Print();
        return;
    }
    Py_INCREF(PYOBJECTTYPE::type());
    PyObjectGuard type_guard = (PyObject *) PYOBJECTTYPE::type();
    if (PyModule_AddObject(
            PyServiceGlobals::instance().proc_module(),
            PYOBJECTTYPE::name().c_str(),
            type_guard.get()) == -1) {
        PyErr_Print();
        throw dds::core::Error(
                "add_type: error inserting type="
                + PYOBJECTTYPE::name()
                + "in module="
                + PyServiceGlobals::PROCESSOR_MODULE_NAME);
    }
    type_guard.release();
}


PyObject* PyProcessorPlugin::find_pyproc_type(const std::string& name)
{
    PyObject *pyproc_type = PyDict_GetItemString(
            PyModule_GetDict(PyServiceGlobals::instance().proc_module()),
            name.c_str());
    if (pyproc_type == NULL) {
        PyErr_Print();
        throw dds::core::Error(
                "load_module: error getting type="
                + name);
    }

    return pyproc_type;
}

void PyProcessorPlugin::load_module()
{
    // Import user module
    py_user_module_ = PyImport_ImportModule(property_.module().c_str());
    if (py_user_module_.get() == NULL) {
        PyErr_Print();
        throw dds::core::Error(
                "load_module: error importing module="
                + property_.module());
    }
    PyObject *user_dict = PyModule_GetDict(py_user_module_.get());
    if (user_dict == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: error getting user dictionary");
    }

    PyServiceGlobals::instance().init_processor();
    // Obtain pyproc.Processor type
    pyproc_type_= find_pyproc_type(BASE_PROCESSOR_TYPE_NAME);
    if (pyproc_type_ == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: error getting Processor type");
    }

    // Add processor types to module
    add_type<PyRouteType>();
    add_type<PyInputAccessorType>();
    add_type<PyOutputAccessorType>();
    add_type<PyInputType>();
    add_type<PyOutputType>();
    add_type<PySampleType>();
    add_type<PyLoanedSamplesType>();

    create_function_ = PyDict_GetItemString(
            user_dict,
            property_.create_function().c_str());
    if (create_function_ == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: create processor method not found");
    }
    if (!PyCallable_Check(create_function_)) {
        throw dds::core::Error("load_module: create processor is not callable");
    }
}


void PyProcessorPlugin::reload()
{

    PyObject *new_module = PyImport_ReloadModule(py_user_module_.get());
    if (new_module == NULL) {
        PyErr_Print();
        throw dds::core::Error("PyProcessorPlugin:reload error loading user module");
    }
    py_user_module_ = std::move(new_module);

    PyObject *user_dict = PyModule_GetDict(py_user_module_.get());
    if (user_dict == NULL) {
        PyErr_Print();
        throw dds::core::Error("load_module: error getting user dictionary");
    }

    create_function_ = PyDict_GetItemString(
            user_dict,
            property_.create_function().c_str());
    if (create_function_ == NULL) {
        PyErr_Print();
        throw dds::core::Error("reload: create processor method not found");
    }
    if (!PyCallable_Check(create_function_)) {
        throw dds::core::Error("reload: create processor is not callable");
    }
}


PyProcessorPlugin::~PyProcessorPlugin()
{
}

PyObject* PyProcessorPlugin::create_processor(
        PyRoute *py_route,
        PyObject *py_properties)
{   
    PyObject *py_proc = PyObject_CallFunctionObjArgs(
            create_function_,
            py_route,
            py_properties,
            NULL /* end of params */);
    if (py_proc == NULL) {
        PyErr_Print();
        throw dds::core::Error("PyProcessorPlugin: failed to create processor from plugin");
    }
    if (!PyObject_IsInstance(py_proc, pyproc_type_)) {
        PyErr_Print();
        Py_DECREF(py_proc);
        throw dds::core::Error("PyProcessorPlugin: create_processor must return an implementation of Processor");
    }

    return py_proc;
}

RTI_RoutingServiceProcessorPlugin *
PyProcessorPlugin::create_plugin(
        PyProcessorPlugin* plugin)
{
    RTI_RoutingServiceProcessorPlugin *native_plugin = NULL;
    RTIOsapiHeap_allocateStructure(
            &native_plugin,
            struct RTI_RoutingServiceProcessorPlugin);
    rti::core::check_create_entity(
            native_plugin,
            "RTI_RoutingServiceProcessorPlugin");
    RTI_RoutingServiceProcessorPlugin_initialize(native_plugin);

    // Initialize native implementation
    native_plugin->processor_plugin_data =
            static_cast<void *> (plugin);
    native_plugin->plugin_delete =
            PyProcessorPlugin::delete_plugin;
    native_plugin->create_processor =
            PyProcessorPlugin::forward_create_processor;
    native_plugin->delete_processor =
            PyProcessorPlugin::forward_delete_processor;

    return native_plugin;
}

void PyProcessorPlugin::delete_plugin(
        RTI_RoutingServiceProcessorPlugin* native_plugin,
        RTI_RoutingServiceEnvironment*)
{
    PyProcessorPlugin *plugin = static_cast<PyProcessorPlugin*> (
            native_plugin->processor_plugin_data);
    // Plug-in is destructor not allowed to throw
    delete plugin;
    RTIOsapiHeap_freeStructure(native_plugin);
}

RTI_RoutingServiceProcessor *
PyProcessorPlugin::forward_create_processor(
        void *native_plugin_data,
        RTI_RoutingServiceRoute *native_route,
        const struct RTI_RoutingServiceProperties *native_properties,
        RTI_RoutingServiceEnvironment *environment)
{
    return PyProcessor::create_native(
            static_cast<PyProcessorPlugin *> (native_plugin_data),
            native_route,
            native_properties,
            environment);
}

void PyProcessorPlugin::forward_delete_processor(
        void *native_plugin_data,
        struct RTI_RoutingServiceProcessor *native_processor,
        RTI_RoutingServiceRoute *,
        RTI_RoutingServiceEnvironment *environment)
{

    PyProcessor::delete_native(
            static_cast<PyProcessorPlugin *> (native_plugin_data),
            native_processor,
            environment);
}

struct RTI_RoutingServiceProcessorPlugin * PyProcessorPlugin_create_processor_plugin(
        const struct RTI_RoutingServiceProperties * native_properties,
        RTI_RoutingServiceEnvironment *environment)
{
    RTI_RoutingServiceProcessorPlugin *plugin = NULL;
    bool py_init = Py_IsInitialized();

    if (PyServiceGlobals::instance().from_service()) {
        PyGILState_Ensure();
    } else {
        if (py_init) {
            PyEval_RestoreThread(PyServiceGlobals::instance().assert_state());
        } 
    }
    
    try {
        plugin = PyProcessorPlugin::create_plugin(
                new PyProcessorPlugin(native_properties));
    } catch (const std::exception& ex) {
        RTI_RoutingServiceEnvironment_set_error(
                environment,
                "%s",
                ex.what());
    } catch (...) {}

    /*
     * IMPORTANT: When running RS executable we need to relinquish the control
     * of this thread of the GIL. The thread that instantiates the
     * PyProcessorPlugin will be the 'main' thread and hence the one currently
     * onwing the GIL.
     */
    if (PyServiceGlobals::instance().from_service()) {
        PyGILState_Release(PyGILState_LOCKED);
    } else {
        PyEval_SaveThread();
    }

    return plugin;
}

} } }

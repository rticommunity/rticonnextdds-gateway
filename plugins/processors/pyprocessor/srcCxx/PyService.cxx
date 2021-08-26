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

#ifndef RTI_routingservice_DLL_VARIABLE
  #define RTI_routingservice_DLL_VARIABLE
#endif

#include "Python.h"

#include "routingservice/routingservice_service.h"
#include "NativeUtils.hpp"
#include "PyService.hpp"
#include "PyServiceGlobals.hpp"

namespace rti { namespace routing { namespace py {
/*
 * --- PyService Python methods ------------------------------------------------
 */

RTI_RoutingServiceProperty& to_native(
        RTI_RoutingServiceProperty &dest,
        PyObject *py_dict)
{
    if (!PyDict_Check(py_dict)) {
        throw dds::core::Error("to_native: object is not a dictionary");
    }

    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, cfg_file, Unicode);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, service_name, Unicode);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, enforce_xsd_validation, Bool);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, service_verbosity, Long);
    switch(dest.service_verbosity) {
    case 0:
        dest.service_verbosity = RTI_ROUTING_SERVICE_LOG_VERBOSITY_SILENT;
        break;

    case 1:
        dest.service_verbosity = RTI_ROUTING_SERVICE_LOG_VERBOSITY_EXCEPTIONS;
        break;

    case 2:
        dest.service_verbosity = RTI_ROUTING_SERVICE_LOG_VERBOSITY_WARNINGS;
        break;

    default:
        dest.service_verbosity = RTI_ROUTING_SERVICE_LOG_VERBOSITY_ALL;
    }

    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, dds_verbosity, Long);
    switch(dest.dds_verbosity) {
    case 0:
        dest.dds_verbosity = NDDS_CONFIG_LOG_VERBOSITY_SILENT;
        break;

    case 1:
        dest.dds_verbosity = NDDS_CONFIG_LOG_VERBOSITY_ERROR;
        break;

    case 2:
        dest.dds_verbosity = NDDS_CONFIG_LOG_VERBOSITY_WARNING;
        break;

    case 3:
        dest.dds_verbosity =
                NDDS_CONFIG_LOG_VERBOSITY_STATUS_LOCAL
                | NDDS_CONFIG_LOG_VERBOSITY_STATUS_REMOTE;
        break;

    default:
        dest.dds_verbosity =
                NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL;
    }

    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, domain_id_base, Long);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, plugin_search_path, Unicode);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, dont_start_service, Bool);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, enable_administration, Bool);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, administration_domain_id, Long);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, enable_monitoring, Bool);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, monitoring_domain_id, Long);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, skip_default_files, Bool);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, identify_execution, Bool);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, license_file_name, Unicode);
    RTI_PY_TO_NATIVE_MEMBER(py_dict, dest, user_environment, Dict);

    return dest;
}

PyObject* PyService::start(PyService *self, PyObject *Py_UNUSED(ignored))
{
    if (!RTI_RoutingService_start(self->native_)) {
        PyErr_SetString(PyExc_RuntimeError, "error starting service");
        return NULL;
    }

    Py_RETURN_NONE;
}

PyObject* PyService::stop(PyService* self, PyObject *Py_UNUSED(ignored))
{
    if (!RTI_RoutingService_stop(self->native_)) {
        PyErr_SetString(PyExc_RuntimeError, "error stopping service");
        return NULL;
    }

    Py_RETURN_NONE;

}

PyObject* PyService::started(PyService* self, void *closure)
{
    return PyBool_FromLong(RTI_RoutingService_is_started(self->native_));
}

PyObject* PyService::new_service(
        PyTypeObject* type,
        PyObject* args,
        PyObject* kwds)
{
    PyObject *py_dict = NULL;

    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &py_dict)) {
        return NULL;
    }
    if (kwds != NULL) {
        if (py_dict != NULL) {
            PyErr_SetString(
                    PyExc_ValueError,
                    "specifying both arguments and keywords is not allowed");
            return NULL;
        }
        assert(PyDict_Check(kwds));
        py_dict = kwds;
    }

    RTI_RoutingServiceProperty native_property =
            RTI_RoutingServiceProperty_INITIALIZER;
    RTI_RoutingServiceProperty_copy(
            &native_property,
            &RTI_ROUTING_SERVICE_PROPERTY_DEFAULT);
    native_property.service_verbosity = 0;
    native_property.dds_verbosity = 0;
    PyService *py_service = NULL;
    try {
        if (py_dict != NULL) {
            to_native(native_property, py_dict);
        }
        py_service = new PyService(native_property);
    } catch (const std::exception& ex) {
        PyErr_SetString(PyExc_RuntimeError, ex.what());
    }

    RTI_RoutingServiceProperty_finalize(&native_property);

    return py_service;
}


static PyGetSetDef PyService_g_getsetters[] = {
    {
        (char *) "started",
        (getter) PyService::started,
        (setter) NULL,
        (char *) "whether this service is started or not",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyService_g_methods[] = {
    {
        "start",
        (PyCFunction) PyService::start,
        METH_VARARGS,
        "starts the servide"
    },
    {
        "stop",
        (PyCFunction) PyService::stop,
        METH_VARARGS,
        "stops the service"
    },
    {NULL}  /* Sentinel */
};


PyService::PyService(
        const RTI_RoutingServiceProperty& property)
        :PyNativeWrapper(RTI_RoutingService_new(&property)),
        state_(NULL)

{
    if (native_ == NULL) {
        throw dds::core::Error("error creating RoutingService");
    }
}

PyService::~PyService()
{
    if (native_ != NULL) {
        RTI_RoutingService_delete(native_);
        native_ = NULL;
    }
}



PyTypeObject* PyServiceType::type()
{
    static PyTypeObject _service_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_service_type, sizeof (_service_type));
        _service_type.tp_name = "rti.gateway.service.RoutingService";
        _service_type.tp_doc = "RoutingService object";
        _service_type.tp_basicsize = sizeof (PyService);
        _service_type.tp_itemsize = 0;
        _service_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _service_type.tp_dealloc = PyNativeWrapper<PyServiceType, PyService>::delete_object;
        _service_type.tp_methods = PyService_g_methods;
        _service_type.tp_getset = PyService_g_getsetters;
        _service_type.tp_new = PyService::new_service;
        _init = true;
    }

    return &_service_type;
}

const std::string& PyServiceType::name()
{
    static std::string __name("RoutingService");

    return __name;
}

} } }


int PyService_load()
{
    try {
        rti::routing::py::PyServiceGlobals::instance().init_service();
    } catch (const std::exception& ex) {
        PyErr_SetString(PyExc_RuntimeError, ex.what());
        return -1;
    }

    return 0;
}



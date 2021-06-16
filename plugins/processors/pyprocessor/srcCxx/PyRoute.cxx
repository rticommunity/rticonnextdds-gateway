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

#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <iterator>
#include <dds/core/corefwd.hpp>

#include "PyRoute.hpp"

namespace rti { namespace routing { namespace py {

/*
 * --- PyInputAccessor --------------------------------------------------------
 */
PyInputAccessor::PyInputAccessor(PyRoute* py_route, int32_t count = 0)
        : PyAllocatorGeneric(),
        py_route_(py_route),
        count_(count),
        iterator_(0)
{
}

Py_ssize_t PyInputAccessor::count(PyInputAccessor* self)
{
    return self->count_;
}

PyObject* PyInputAccessor::binary(
        PyInputAccessor* self,
        PyObject* key)
{
    PyObject *py_input;
    if (PyLong_Check(key)) {
        py_input = self->py_route_->input(PyLong_AsLong(key));
    } else if (PyUnicode_Check(key)) {
        py_input = self->py_route_->input(PyUnicode_AsUTF8(key));
    } else {
        PyErr_SetString(
                PyExc_TypeError,
                "PyInputAccessor::binary: key must be an string representing an input name "
                "or an integer representing an input index");
        return NULL;
    }

    if (py_input != Py_None) {
        Py_INCREF(py_input);
    }

    return py_input;
}


PyObject* PyInputAccessor::get_iterator(PyInputAccessor *self)
{
    return new PyInputAccessor(
            self->py_route_,
            self->py_route_->it_input_count());
}

PyObject* PyInputAccessor::iterator_next(PyInputAccessor *self)
{
    if (self->iterator_ == self->count_) {
        PyErr_Format(PyExc_StopIteration, "%i", self->count_);
        return NULL;
    }
    PyObject *py_input = (PyObject *) self->py_route_->input(self->iterator_);
    Py_INCREF(py_input);
    ++self->iterator_;

    return py_input;
}

static PyMappingMethods PyInputAccessor_g_mapping = {
    (lenfunc) PyInputAccessor::count,
    (binaryfunc) PyInputAccessor::binary,
    NULL
};


PyTypeObject* PyInputAccessorType::type()
{
    static PyTypeObject _input_ac_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_input_ac_type, sizeof (_input_ac_type));
        _input_ac_type.tp_name = "rti.gateway.proc.InputAccessor";
        _input_ac_type.tp_doc = "InputAccessor object";
        _input_ac_type.tp_basicsize = sizeof (PyInputAccessor);
        _input_ac_type.tp_itemsize = 0;
        _input_ac_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _input_ac_type.tp_dealloc = PyAllocatorGeneric<PyInputAccessorType, PyInputAccessor>::delete_object;
        _input_ac_type.tp_as_mapping = &PyInputAccessor_g_mapping;
        _input_ac_type.tp_iter = (getiterfunc) PyInputAccessor::get_iterator;
        _input_ac_type.tp_iternext = (iternextfunc) PyInputAccessor::iterator_next;
        _init = true;
    }

    return &_input_ac_type;
}

const std::string& PyInputAccessorType::name()
{
    static std::string __name("InputAccessor");

    return __name;
}


/*
 * --- PyOutputAccessor --------------------------------------------------------
 */
PyOutputAccessor::PyOutputAccessor(PyRoute* py_route, int32_t count = 0)
        : PyAllocatorGeneric(),
        py_route_(py_route),
        count_(count),
        iterator_(0)
{
}

Py_ssize_t PyOutputAccessor::count(PyOutputAccessor* self)
{
    return self->count_;
}

PyObject* PyOutputAccessor::binary(
        PyOutputAccessor* self,
        PyObject* key)
{
    PyObject *py_output;
    if (PyLong_Check(key)) {
        py_output = self->py_route_->output(PyLong_AsLong(key));
    } else if (PyUnicode_Check(key)) {
        py_output = self->py_route_->output(PyUnicode_AsUTF8(key));
    } else {
        PyErr_SetString(
                PyExc_ValueError,
                "PyOutputAccessor::binary: key must be an string representing an output name "
                "or an integer representing an output index");
        return NULL;
    }

    if (py_output != Py_None) {
        Py_INCREF(py_output);
    }

    return py_output;
}


PyObject* PyOutputAccessor::get_iterator(PyOutputAccessor *self)
{
    return new PyOutputAccessor(
            self->py_route_,
            self->py_route_->it_output_count());
}

PyObject* PyOutputAccessor::iterator_next(PyOutputAccessor *self)
{
    if (self->iterator_ == self->count_) {
        PyErr_Format(PyExc_StopIteration, "%i", self->count_);
        return NULL;
    }
    PyObject *py_output = (PyObject *) self->py_route_->output(self->iterator_);
    Py_INCREF(py_output);
    ++self->iterator_;

    return py_output;
}

static PyMappingMethods PyOutputAccessor_g_mapping = {
    (lenfunc) PyOutputAccessor::count,
    (binaryfunc) PyOutputAccessor::binary,
    NULL
};

PyTypeObject* PyOutputAccessorType::type()
{
    static PyTypeObject _output_ac_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_output_ac_type, sizeof (_output_ac_type));
        _output_ac_type.tp_name = "rti.gateway.proc.OutputAccessor";
        _output_ac_type.tp_doc = "OutputAccessor object";
        _output_ac_type.tp_basicsize = sizeof (PyOutputAccessor);
        _output_ac_type.tp_itemsize = 0;
        _output_ac_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _output_ac_type.tp_dealloc = PyAllocatorGeneric<PyOutputAccessorType, PyOutputAccessor>::delete_object;
        _output_ac_type.tp_as_mapping = &PyOutputAccessor_g_mapping;
        _output_ac_type.tp_iter = (getiterfunc) PyOutputAccessor::get_iterator;
        _output_ac_type.tp_iternext = (iternextfunc) PyOutputAccessor::iterator_next;
        _init = true;
    }

    return &_output_ac_type;
}

const std::string& PyOutputAccessorType::name()
{
    static std::string __name("OutputAccessor");

    return __name;
}


/*
 * --- PyRoute Python methods -------------------------------------------------
 */


PyObject* PyRoute::in_accessor(PyRoute *self, void *)
{
    Py_INCREF(self->input_accessor_);
    return self->input_accessor_;
}

PyObject* PyRoute::out_accessor(PyRoute* self, void*)
{
    Py_INCREF(self->output_accessor_);
    return self->output_accessor_;
}


static PyGetSetDef PyRoute_getsetters[] = {
    {
        (char *) "inputs",
        (getter) PyRoute::in_accessor,
        (setter) NULL,
        (char *) "returns the input accessor",
        NULL
    },
    {
        (char *) "outputs",
        (getter) PyRoute::out_accessor,
        (setter) NULL,
        (char *) "returns the output accessor",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyRoute_g_methods[] = {
    {NULL}  /* Sentinel */
};


/*
 * --- PyRoute class Implementation -------------------------------------------
 */


PyRoute::PyRoute(RTI_RoutingServiceRoute *native_route)
      : PyNativeWrapper(native_route),
        started_(false),
        input_accessor_(new PyInputAccessor(this)),
        output_accessor_(new PyOutputAccessor(this))
{
    input_accessor_->count_ = RTI_RoutingServiceRoute_get_input_count(get());
    output_accessor_->count_ = RTI_RoutingServiceRoute_get_output_count(get());
}

PyRoute::~PyRoute()
{
    Py_DECREF(input_accessor_);
    Py_DECREF(output_accessor_);
}

void PyRoute::started(bool state)
{
    started_ = state;
}

int32_t PyRoute::it_input_count()
{
    return started_
            ? RTI_RoutingServiceRoute_get_input_count(get())
            : 0;
}

int32_t PyRoute::it_output_count()
{
    return started_
            ? RTI_RoutingServiceRoute_get_output_count(get())
            : 0;
}


PyInput* PyRoute::input(int32_t index)
{
    PyInput::native_type *native_input =
            RTI_RoutingServiceRoute_get_input_at(native_, index);
    return native_input == NULL ? (PyInput*) Py_None : input(native_input);
}

PyInput* PyRoute::input(const char *name)
{
    PyInput::native_type *native_input =
            RTI_RoutingServiceRoute_lookup_input_by_name(native_, name);
    if (native_input != NULL) {
        return input(native_input);
    }

    return (PyInput *) Py_None;
}

PyInput* PyRoute::input(PyInput::native_type *native_input)
{
    void *py_input = RTI_RoutingServiceInput_get_user_data(native_input);
    assert(py_input != NULL);

    return static_cast<PyInput*>(py_input);
}


PyOutput* PyRoute::output(int32_t index)
{
    PyOutput::native_type *native_output =
            RTI_RoutingServiceRoute_get_output_at(native_, index);
    return native_output == NULL ? (PyOutput*) Py_None : output(native_output);
}


PyOutput* PyRoute::output(const char *name)
{
    PyOutput::native_type *native_output =
            RTI_RoutingServiceRoute_lookup_output_by_name(native_, name);
    if (native_output != NULL) {
        return output(native_output);
    }

    return (PyOutput*) Py_None;
}

PyOutput * PyRoute::output(PyOutput::native_type* native_output)
{
    void *py_output = RTI_RoutingServiceOutput_get_user_data(native_output);
    assert(py_output != NULL);

    return static_cast<PyOutput*>(py_output);
}


PyTypeObject* PyRouteType::type()
{
    static PyTypeObject _route_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_route_type, sizeof (_route_type));
        _route_type.tp_name = "rti.gateway.proc.Route";
        _route_type.tp_doc = "Route object";
        _route_type.tp_basicsize = sizeof (PyRoute);
        _route_type.tp_itemsize = 0;
        _route_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _route_type.tp_dealloc = PyAllocatorGeneric<PyRouteType, PyRoute>::delete_object;
        _route_type.tp_methods = PyRoute_g_methods;
        _route_type.tp_getset = PyRoute_getsetters;
        _init = true;
    }

    return &_route_type;
}

const std::string& PyRouteType::name()
{
    static std::string __name("Route");

    return __name;
}


} } }




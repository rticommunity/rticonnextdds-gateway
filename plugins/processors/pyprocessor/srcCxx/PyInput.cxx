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
#include "PyInput.hpp"


namespace rti { namespace routing { namespace py {



PySelectorBuilder::PySelectorBuilder(
        PyInput* the_input,
        PyObject *py_dict)
        : input(the_input),
          state(DEFAULT_STATE())

{
    build(py_dict);
}

PySelectorBuilder::~PySelectorBuilder()
{
    if (state.query_data != NULL) {
        RTI_RoutingServiceInput_delete_content_query(input->get(), state.query_data);
    }
}


const RTI_RoutingServiceSelectorState&
PySelectorBuilder::DEFAULT_STATE()
{
    static RTI_RoutingServiceSelectorState init_state =
            RTI_RoutingServiceSelectorState_INITIALIZER;

    return init_state;
}

#define RTI_PY_CHECK_AND_THROW(TYPE, OBJECT, MEMBER) \
if(!Py ## TYPE ## _Check((OBJECT))) {\
    throw dds::core::InvalidArgumentError(\
            "element=" #MEMBER " is not a " #TYPE); \
}

void PySelectorBuilder::create_content_query()
{
    if (state.content.expression != NULL) {
        RTI_RoutingServiceInput_create_content_query(
                input->get(),
                state.query_data,
                &state.content);
    }
}


void PySelectorBuilder::build(PyObject *py_dict)
{
    PyObject *py_item;

    //sample_state
    py_item = PyDict_GetItemString(py_dict, "sample_state");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, sample_state);
        state.sample_state = (DDS_SampleStateMask) PyLong_AsLong(py_item);
    }

    // view_state
    py_item = PyDict_GetItemString(py_dict, "view_state");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, view_state);
        state.view_state = (DDS_ViewStateKind) PyLong_AsLong(py_item);
    }

    // instance_state
    py_item = PyDict_GetItemString(py_dict, "instance_state");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, instance_state);
        state.instance_state = (DDS_InstanceStateKind) PyLong_AsLong(py_item);
    }

    // instance
    py_item = PyDict_GetItemString(py_dict, "instance");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Dict, py_item, instance);
        to_native(state.instance_handle, py_item);
        state.instance_selection = RTI_ROUTING_SERVICE_THIS_INSTANCE_SELECTION;
    }

    // next_instance
    py_item = PyDict_GetItemString(py_dict, "next_instance");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Dict, py_item, instance);
        to_native(state.instance_handle, py_item);
        state.instance_selection = RTI_ROUTING_SERVICE_NEXT_INSTANCE_SELECTION;
    }

    // max_samples
    py_item = PyDict_GetItemString(py_dict, "max_samples");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Long, py_item, max_samples);
        state.sample_count_max = (int32_t) PyLong_AsLong(py_item);
    }

    // filter
    py_item = PyDict_GetItemString(py_dict, "filter");
    if (py_item != NULL) {
        RTI_PY_CHECK_AND_THROW(Dict, py_item, filter);
        // expression
        PyObject *py_exp = PyDict_GetItemString(py_item, "expression");
        if (py_exp != NULL) {
            RTI_PY_CHECK_AND_THROW(Unicode, py_exp, expression);
            state.content.expression = (char *) PyUnicode_AsUTF8(py_exp);
        }
        create_content_query();
    }

}

/*
 * --- PyInput Python methods -------------------------------------------------
 */

PyObject* PyInput::info(PyInput* self, void* closure)
{
    Py_INCREF(self->info_.get());
    return self->info_.get();
}


PyObject* PyInput::take(PyInput *self, PyObject *args)
{
    return PyInput::read_or_take_w_selector(
            self,
            args,
            RTI_RoutingServiceInput_take_w_selector);
}

PyObject* PyInput::read(PyInput* self, PyObject* args)
{
    return PyInput::read_or_take_w_selector(
            self,
            args,
            RTI_RoutingServiceInput_read_w_selector);
}

static PyGetSetDef PyInput_g_getsetters[] = {
    {
        (char *) "info",
        (getter) PyInput::info,
        (setter) NULL,
        (char *) "information properties of this input",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyInput_g_methods[] = {
    {
        "take",
        (PyCFunction) PyInput::take,
        METH_VARARGS,
        "takes all available samples from the input's cache"
    },
    {
        "read",
        (PyCFunction) PyInput::read,
        METH_VARARGS,
        "reads all available samples from the input's cache"
    },
    {NULL}  /* Sentinel */
};


PyInput::PyInput(
        PyInput::native_type* native,
        int32_t index)
        : PyNativeWrapper(native),
        index_(index),
        info_(PyDict_New())
{
    build_info();
}



const char* PyInput::name()
{
    return RTI_RoutingServiceInput_get_name(get());
}


PyObject* PyInput::read_or_take_w_selector(
        PyInput *self,
        PyObject *args,
        PyInput::read_or_take_w_selector_fcn read_or_take_w_selector)
{
    using dds::core::xtypes::DynamicData;
    using rti::routing::processor::LoanedSamples;

    PyObject *py_dict = NULL;
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &py_dict)) {
        return NULL;
    }
    
    PyLoanedSamples::native_loaned_samples native_samples =
            RTI_RoutingServiceLoanedSamples_INITIALIZER;
    PyObject *py_samples = NULL;
    try {
        if (py_dict == NULL) {
            if (!read_or_take_w_selector(
                    self->get(),
                    &native_samples,
                    &(PySelectorBuilder::DEFAULT_STATE()))) {
                throw dds::core::Error("error reading from input");
            }
        } else {
            PySelectorBuilder selector_builder(self, py_dict);

            if (!read_or_take_w_selector(
                    self->get(),
                    &native_samples,
                    &selector_builder.state)) {
                throw dds::core::Error("error reading from input with selector");
            }
        }
        py_samples = new PyLoanedSamples(native_samples, self->get());
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        py_samples = NULL;
    }

    return py_samples;
}

void PyInput::build_info()
{
    int32_t index = this->index_;
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), index, PyLong_FromLong);

    const char *name = this->name();
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), name, PyUnicode_FromString);

    const RTI_RoutingServiceStreamInfo& stream_info =
            *RTI_RoutingServiceInput_get_stream_info(get());
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), stream_info, from_native);
}

PyTypeObject* PyInputType::type()
{
    static PyTypeObject _input_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_input_type, sizeof (_input_type));
        _input_type.tp_name = "rti.gateway.proc.Input";
        _input_type.tp_doc = "Input object";
        _input_type.tp_basicsize = sizeof (PyInput);
        _input_type.tp_itemsize = 0;
        _input_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _input_type.tp_dealloc = PyNativeWrapper<PyInputType, PyInput>::delete_object;
        _input_type.tp_methods = PyInput_g_methods;
        _input_type.tp_getset = PyInput_g_getsetters;
        _init = true;
    }

    return &_input_type;
}

const std::string& PyInputType::name()
{
    static std::string __name("Input");

    return __name;
}


} } }


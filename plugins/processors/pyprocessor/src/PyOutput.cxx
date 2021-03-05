#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <iterator>
#include <dds/core/corefwd.hpp>
#include "PyOutput.hpp"

namespace rti { namespace routing { namespace py {
/*
 * --- PyOutput Python methods -------------------------------------------------
 */
PyObject* PyOutput::info(PyOutput* self, void* closure)
{
    Py_INCREF(self->info_.get());
    return self->info_.get();
}


PyObject* PyOutput::write(PyOutput *self, PyObject *args)
{
    RTI_RoutingServiceSample out_data =
            reinterpret_cast<void *>(&self->output_data_);
    RTI_RoutingServiceSampleInfo out_info = NULL;

    PyObject *py_sample = NULL;
    PyObject *py_data = NULL;
    PyObject *py_info = NULL;
    if (!PyArg_ParseTuple(
            args,
            "O",
            &py_sample)) {
        return NULL;
    }

    if (py_sample->ob_type == PySampleType::type()) {
        py_data = ((PySample *) py_sample)->data_;
        py_info = ((PySample *) py_sample)->info_;
    } else if (PyDict_Check(py_sample)) {
        py_data = py_sample;
    } else {
        PyErr_SetString(
                PyExc_ValueError,
                "sample parameter must be a Sample or data dictionary");
        return NULL;
    }

    try {
        DynamicDataConverter::to_dynamic_data(
                self->output_data_,
                py_data);
        if (py_info != NULL) {
            to_native(self->output_info_->native(), py_info);
            out_info = reinterpret_cast<void *>(&self->output_info_);
        }

        /*
         * Relinquish GIL to allow other RS session threads to execute python.
         * We do this for the write() operation which is I/O and may also
         * block.
         */
        bool ok;
        Py_BEGIN_ALLOW_THREADS

        ok = RTI_RoutingServiceOutput_write_sample(
                self->get(),
                out_data,
                out_info);

        Py_END_ALLOW_THREADS
                
        if (!ok) {
            throw dds::core::Error("error writing sample");
        }
    } catch (const std::exception &ex) {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
    }

    Py_RETURN_NONE;
}

static PyGetSetDef PyOutput_g_getsetters[] = {
    {
        (char *) "info",
        (getter) PyOutput::info,
        (setter) NULL,
        (char *) "information properties of this output",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PyOutput_g_methods[] = {
    {
        "write",
        (PyCFunction) PyOutput::write,
        METH_VARARGS,
        "write a sample by its data and info portion"
    },
    {NULL}  /* Sentinel */
};

const dds::core::xtypes::DynamicType& dynamic_type(
        RTI_RoutingServiceOutput* native_output)
{
    const RTI_RoutingServiceStreamInfo *stream_info =
            RTI_RoutingServiceOutput_get_stream_info(native_output);
    dds::core::xtypes::DynamicType *type_code =
            static_cast<dds::core::xtypes::DynamicType *> (
            stream_info->type_info.type_representation);
    return *type_code;
}


PyOutput::PyOutput(
        PyOutput::native_type* native,
        int32_t index)
        : PyNativeWrapper(native),
        index_(index),
        info_(PyDict_New()),
        output_data_(dynamic_type(native))
{
    build_info();
}

void PyOutput::build_info()
{
    int32_t index = this->index_;
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), index, PyLong_FromLong);

    const char *name =
            RTI_RoutingServiceOutput_get_name(get());
    RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), name, PyUnicode_FromString);


    const RTI_RoutingServiceStreamInfo& stream_info =
            *RTI_RoutingServiceOutput_get_stream_info(get());
     RTI_PY_ADD_DICT_ITEM_VALUE(info_.get(), stream_info, from_native);
}


PyTypeObject* PyOutputType::type()
{
    static PyTypeObject _output_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_output_type, sizeof (_output_type));
        _output_type.tp_name = "rti.routing.proc.Output";
        _output_type.tp_doc = "Output object";
        _output_type.tp_basicsize = sizeof (PyOutput);
        _output_type.tp_itemsize = 0;
        _output_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _output_type.tp_dealloc = PyNativeWrapper<PyOutputType, PyOutput>::delete_object;
        _output_type.tp_methods = PyOutput_g_methods;
        _output_type.tp_getset = PyOutput_g_getsetters;
        _init = true;
    }

    return &_output_type;
}

const std::string& PyOutputType::name()
{
    static std::string __name("Output");

    return __name;
}



} } }


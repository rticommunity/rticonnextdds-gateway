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

#include "PySamples.hpp"

namespace rti { namespace routing { namespace py {

/*
 * --- PySample Python methods ------------------------------------------------
 */

PyObject* PySample::data(PySample* self, void*)
{
    Py_INCREF(self->data_);
    return self->data_;
}

PyObject* PySample::valid_data(PySample* self, void*)
{
    Py_INCREF(self->valid_);
    return self->valid_;
}

PyObject* PySample::info(PySample* self, void*)
{
    Py_INCREF(self->info_);
    return self->info_;
}
/*
 * --- PySample class implementation ------------------------------------------
 */


PyObject* PySample::build_data(
        const native_data* data,
        const native_info* info)
{
    if (info != NULL && !info->valid()) {
        /* empty data */
        return PyDict_New();
    }

    return DynamicDataConverter::from_dynamic_data(*data);
}

PyObject* PySample::build_info(
        const native_info* info)
{
    if (info == NULL) {
        /* empty data */
        return PyDict_New();
    }

    return from_native(info->extensions().native());
}

PySample::PySample(
        const native_data* native_data, const native_info *native_info)
        :data_(build_data(native_data, native_info)),
        info_(build_info(native_info)),
        valid_(PyBool_FromLong(native_info == NULL ? 1 : native_info->valid())),
        native_data_(native_data),
        native_info_(native_info)
{
}

PySample::~PySample()
{
    if (data_ != NULL) {
        Py_DECREF(data_);
    }
    if (info_ != NULL) {
        Py_DECREF(info_);
    }
    if (valid_ != NULL) {
        Py_DECREF(valid_);
    }
}


PyObject* PySample::print(PySample* self)
{
    return PyDict_Type.tp_str(self->data_);
}

PyObject* PySample::representation(PySample* self)
{
    return PyDict_Type.tp_repr(self->data_);
}


static PyGetSetDef PySample_g_getsetters[] = {
    {
        (char *) "valid_data",
        (getter) PySample::valid_data,
        (setter) NULL,
        (char *) "returns whether the data portion is valid or not",
        NULL
    },
        {
        (char *) "data",
        (getter) PySample::data,
        (setter) NULL,
        (char *) "returns whether the data portion is valid or not",
        NULL
    },
    {
        (char *) "info",
        (getter) PySample::info,
        (setter) NULL,
        (char *) "info portion of the sample name",
        NULL
    },
    {NULL}  /* Sentinel */
};

static PyMethodDef PySample_g_methods[] = {
    {NULL}  /* Sentinel */
};



PyTypeObject* PySampleType::type()
{
    static PyTypeObject _sample_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&_sample_type, sizeof (_sample_type));
        _sample_type.tp_name = "rti.routing.proc.Sample";
        _sample_type.tp_basicsize = sizeof (PySample);
        _sample_type.tp_itemsize = 0;
        _sample_type.tp_dealloc = PyAllocatorGeneric<PySampleType, PySample>::delete_object;
        _sample_type.tp_repr = (reprfunc) PySample::representation;
        _sample_type.tp_str = (reprfunc) PySample::print;
        _sample_type.tp_flags = Py_TPFLAGS_DEFAULT;
        _sample_type.tp_doc = "Sample object";
        _sample_type.tp_methods = PySample_g_methods;
        _sample_type.tp_getset = PySample_g_getsetters;
        _sample_type.tp_new = NULL;
        _init = true;
    }

    return &_sample_type;
}

const std::string& PySampleType::name()
{
    static std::string __name("Sample");

    return __name;
}

/*
 * --- PyLoanedSamples --------------------------------------------------------
 */

PyLoanedSamples::PyLoanedSamples(
            native_loaned_samples& native_samples,
            RTI_RoutingServiceInput *native_input)
        :py_list_(PyList_New(native_samples.length)),
        native_samples_(native_samples),
        native_input_(native_input)
{
    // Convert samples into dictionaries
    for (int32_t i = 0; i < native_samples.length; ++i) {
        RTI_RoutingServiceSampleInfo native_info = NULL;

        if (native_samples.info_array != NULL) {
            native_info = native_samples.info_array[i];
        }
        PyList_SET_ITEM(
                py_list_,
                i,
                new PySample(
                    static_cast<const PySample::native_data*> (native_samples.data_array[i]),
                    static_cast<const PySample::native_info*> (native_info)));
    }
}

PyLoanedSamples::~PyLoanedSamples()
{
    PyLoanedSamples::return_loan(this);
}


PyObject* PyLoanedSamples::binary(PyLoanedSamples* self, PyObject* key)
{
    return PyObject_GetItem(self->py_list_, key);
}

Py_ssize_t PyLoanedSamples::count(PyLoanedSamples* self)
{
    return PyList_GET_SIZE(self->py_list_);
}

PyObject* PyLoanedSamples::get_iterator(PyLoanedSamples* self)
{
    return PyObject_GetIter(self->py_list_);
}

PyObject* PyLoanedSamples::iterator_next(PyObject* self)
{
    return PyIter_Next(self);
}

PyObject* PyLoanedSamples::return_loan(PyLoanedSamples *self)
{
    if (self->py_list_ == NULL) {
        Py_RETURN_NONE;
    }
  
    if (!RTI_RoutingServiceInput_return_loan(
            self->native_input_,
            &self->native_samples_)) {
        PyErr_Format(
                PyExc_RuntimeError,
                "%s",
                "PyLoanedSamples::~PyLoanedSamples:: error returning loaned samples");
        Py_RETURN_NONE;
    }
    Py_DECREF(self->py_list_);
    self->py_list_ = NULL;

    Py_RETURN_NONE;
}

PyObject* PyLoanedSamples::__enter__(PyLoanedSamples *self)
{
    Py_INCREF(self);
    return self;
}

PyObject* PyLoanedSamples::__exit__(
        PyLoanedSamples *self,
        PyObject *type,
        PyObject *value,
        PyObject *tb)
{
    PyLoanedSamples::return_loan(self);

    return PyBool_FromLong(0);
}

static PyMappingMethods PyLoanedSamples_g_mapping = {
    (lenfunc) PyLoanedSamples::count,
    (binaryfunc) PyLoanedSamples::binary,
    NULL
};

static PyMethodDef PyLoanedSamples_g_methods[] = {
    {
        "__enter__",
        (PyCFunction) PyLoanedSamples::__enter__,
        METH_NOARGS,
        ""
    },
    {
        "__exit__",
        (PyCFunction) PyLoanedSamples::__exit__,
        METH_VARARGS,
        ""
    },
    {
        "return_loan",
        (PyCFunction) PyLoanedSamples::return_loan,
        METH_NOARGS,
        "returns the current outstanding loan. This operation does nothing if"
        "the loan has already been returned."
    },
    {NULL}  /* Sentinel */
};

PyTypeObject* PyLoanedSamplesType::type()
{
    static PyTypeObject __loaned_samples_type;
    static bool _init = false;

    if (!_init) {
        RTIOsapiMemory_zero(&__loaned_samples_type, sizeof (__loaned_samples_type));
        __loaned_samples_type.tp_name = "rti.routing.proc.LoanedSamples";
        __loaned_samples_type.tp_basicsize = sizeof (PyLoanedSamples);
        __loaned_samples_type.tp_itemsize = 0;
        __loaned_samples_type.tp_dealloc =
                PyAllocatorGeneric<PyLoanedSamplesType, PyLoanedSamples>::delete_object;
        __loaned_samples_type.tp_as_mapping = &PyLoanedSamples_g_mapping;
        __loaned_samples_type.tp_iter = (getiterfunc) PyLoanedSamples::get_iterator;
        __loaned_samples_type.tp_iternext = (iternextfunc) PyLoanedSamples::iterator_next;
        __loaned_samples_type.tp_flags = Py_TPFLAGS_DEFAULT;
        __loaned_samples_type.tp_doc = "LoanedSamples object";
        __loaned_samples_type.tp_new = NULL;
        __loaned_samples_type.tp_methods = PyLoanedSamples_g_methods;
        _init = true;
    }

    return &__loaned_samples_type;
}

const std::string& PyLoanedSamplesType::name()
{
    static std::string __name("LoanedSamples");

    return __name;
}




} } }


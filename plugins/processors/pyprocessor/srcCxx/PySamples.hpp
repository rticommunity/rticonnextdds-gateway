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

#ifndef LOANEDSAMPLES_HPP
#define LOANEDSAMPLES_HPP

#include "Python.h"

#include "routingservice/routingservice_processor.h"
#include "routingservice/routingservice_infrastructure.h"
#include "routingservice/routingservice_adapter_new.h"
#include <dds/core/corefwd.hpp>
#include <dds/core/xtypes/DynamicData.hpp>
#include <rti/routing/processor/LoanedSamples.hpp>

#include "NativeUtils.hpp"
#include "PyDynamicData.hpp"

namespace rti { namespace routing { namespace py {

class PyOutput;

class PySampleType : public PyObject {
public:
    typedef dds::sub::SampleInfo native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PySample : public PyAllocatorGeneric<PySampleType, PySample> {
public:
    typedef dds::core::xtypes::DynamicData native_data;
    typedef dds::sub::SampleInfo native_info;
    PySample(const native_data* data, const native_info *info);
    ~PySample();

    /* python methods */
    static PyObject* valid_data(PySample *self, void* closure);
    static PyObject* data(PySample *self, void* closure);
    static PyObject* info(PySample *self, void* closure);


    static PyObject * print(PySample *self);
    static PyObject * representation(PySample *self);

private:
    static PyObject* build_data(
            const native_data* data,
            const native_info *info);

    static PyObject* build_info(
            const native_info *info);
private:
    friend class PyOutput;
    PyObject *data_; //as dictionary
    PyObject *info_; //as dictionary
    PyObject *valid_;
    const native_data *native_data_;
    const native_info *native_info_;
};


class PyLoanedSamplesType : public PyObject {
public:
    typedef dds::sub::SampleInfo native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyLoanedSamples : public PyAllocatorGeneric<PyLoanedSamplesType, PyLoanedSamples> {

public:
    typedef RTI_RoutingServiceLoanedSamples native_loaned_samples;

    PyLoanedSamples(
            native_loaned_samples& native_samples,
            RTI_RoutingServiceInput *native_input);

    ~PyLoanedSamples();    

    /* python methods */
    static Py_ssize_t count(PyLoanedSamples *self);
    static PyObject* binary(PyLoanedSamples *self, PyObject *key);
    static PyObject* get_iterator(PyLoanedSamples *self);
    static PyObject* iterator_next(PyObject *self);
    static PyObject* __enter__(PyLoanedSamples *self);
    static PyObject* __exit__(
            PyLoanedSamples *self,
            PyObject *type,
            PyObject *value,
            PyObject *tb);
    static PyObject* return_loan(PyLoanedSamples *self);


private:
    PyObject *py_list_;
    native_loaned_samples native_samples_;
    RTI_RoutingServiceInput *native_input_;
};

} } }

#endif /* LOANEDSAMPLES_HPP */


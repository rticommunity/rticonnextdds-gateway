/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PyInput.hpp
 * Author: asanchez
 *
 * Created on December 25, 2019, 7:24 PM
 */

#ifndef PYINPUT_HPP
#define PYINPUT_HPP


#include "Python.h"

#include "routingservice/routingservice_processor.h"
#include <dds/core/xtypes/DynamicData.hpp>
#include <rti/routing/processor/Processor.hpp>
#include "NativeUtils.hpp"
#include "PySamples.hpp"

namespace rti { namespace routing { namespace py {

class PyInput;

class PySelectorBuilder
{
public:
    const static RTI_RoutingServiceSelectorState& DEFAULT_STATE();


    PySelectorBuilder(PyInput *input, PyObject *py_dict);
    ~PySelectorBuilder();

private:
    void build(PyObject *py_dict);
    void create_content_query();
private:
    friend class PyInput;
    PyInput *input;
    RTI_RoutingServiceSelectorState state;
};

class PyInputType
{
public:
    typedef RTI_RoutingServiceInput native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyInput : public PyNativeWrapper<PyInputType, PyInput>
{
public:
    typedef PyInputType::native_type native_type;
    typedef dds::core::xtypes::DynamicData native_data;
    typedef dds::sub::SampleInfo native_info;
    typedef RTI_RoutingServiceLoanedSamples native_samples;
    typedef DDS_Boolean (*read_or_take_w_selector_fcn) (
            native_type *self,
            struct RTI_RoutingServiceLoanedSamples *loaned_samples,
            const struct RTI_RoutingServiceSelectorState *selector);

    PyInput(native_type *native,
            int32_t index);

    const char *name();
    static PyObject* info(PyInput *self,void *closure );
    static PyObject* take(PyInput *self, PyObject *arg);
    static PyObject* read(PyInput *self, PyObject *arg);

private:
    static PyObject* read_or_take_w_selector(
            PyInput *self,
            PyObject *args,
            read_or_take_w_selector_fcn read_or_take);
    void build_info();


private:
    friend class PySelectorBuilder;
    int32_t index_;
    PyObjectGuard info_;
};


} } }

#endif /* PYINPUT_HPP */


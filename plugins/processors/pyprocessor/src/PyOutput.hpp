/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PyOutput.hpp
 * Author: asanchez
 *
 * Created on December 26, 2019, 9:59 PM
 */

#ifndef PYOUTPUT_HPP
    #define PYOUTPUT_HPP

#include "Python.h"
#include <dds/core/xtypes/DynamicData.hpp>
#include <rti/routing/processor/Processor.hpp>
#include "NativeUtils.hpp"
#include "PySamples.hpp"

namespace rti { namespace routing { namespace py {

class PyOutputType
{
public:
    typedef RTI_RoutingServiceOutput native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyOutput : public PyNativeWrapper<PyOutputType, PyOutput>
{
public:
    typedef PyOutputType::native_type native_type;
    typedef dds::core::xtypes::DynamicData native_data_type;

    PyOutput(
            native_type *native,
            int32_t index);

    static PyObject* info(PyOutput *self, void *closure);
    static PyObject* write(PyOutput *self, PyObject *args);

private:
    void build_info();
private:
    int32_t index_;
    RTI_RoutingServiceEnvironment *native_env_;
    PyObjectGuard info_;
    dds::core::xtypes::DynamicData output_data_;
    dds::sub::SampleInfo output_info_;
};

} } }

#endif /* OUTPUT_HPP */


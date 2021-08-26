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


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

#ifndef PYSERVICE_HPP
#define PYSERVICE_HPP

#include "Python.h"

#include "routingservice/routingservice_service.h"
#include "NativeUtils.hpp"
#include "PySamples.hpp"

namespace rti { namespace routing { namespace py {

class PyService;


class PyServiceType
{
public:
    typedef RTI_RoutingService native_type;
    static PyTypeObject* type();
    static const std::string& name();
};

class PyService : public PyNativeWrapper<PyServiceType, RTI_RoutingService>
{
public:
    typedef RTI_RoutingService native_type;

    PyService(const RTI_RoutingServiceProperty& property);
    ~PyService();

    static PyObject* start(PyService *self, PyObject *arg);
    static PyObject* stop(PyService *self, PyObject *arg);
    static PyObject* started(PyService *self, void *closure);
    static PyObject* new_service(PyTypeObject *type, PyObject *args, PyObject *kwds);

private:
    PyThreadState* state_;
};

} } }

extern "C" RTI_USER_DLL_EXPORT
int PyService_load();

#endif /* PYSERVICE_HPP */


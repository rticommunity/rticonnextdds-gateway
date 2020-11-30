/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PyService.hpp
 * Author: asanchez
 *
 * Created on December 25, 2019, 7:24 PM
 */

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

};

} } }

extern "C" RTI_USER_DLL_EXPORT
int PyService_load();

#endif /* PYSERVICE_HPP */


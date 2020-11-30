/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PyServiceGlobals.hpp
 * Author: asanchez
 *
 * Created on January 11, 2020, 4:53 PM
 */

#ifndef PYSERVICEGLOBALS_HPP
    #define PYSERVICEGLOBALS_HPP

#include "Python.h"
#include "NativeUtils.hpp"

namespace rti { namespace routing { namespace py {
class PyServiceGlobals {
public:
    static const std::string SERVICE_MODULE_NAME;
    static const std::string PROCESSOR_MODULE_NAME;

    static PyServiceGlobals& instance();

    void init_service();
    void init_processor();

    bool from_service();

    PyObject* proc_module();

private:
    PyServiceGlobals();
    ~PyServiceGlobals();
    bool service_initd_;
    /* Reference to the proc module */
    PyObject *proc_module_;

};

} } }
#endif /* PYSERVICEGLOBALS_HPP */


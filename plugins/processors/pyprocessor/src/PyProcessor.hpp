/*
 * (c) 2018 Copyright, Real-Time Innovations, Inc.  All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software.  Licensee has the right to distribute object form
 * only for use with RTI products.  The Software is provided "as is", with no
 * warranty of any type, including any warranty for fitness for any purpose.
 * RTI is under no obligation to maintain or support the Software.  RTI shall
 * not be liable for any incidental or consequential damages arising out of the
 * use or inability to use the software.
 */

#ifndef SHAPES_PROCESSOR_HPP_
#define SHAPES_PROCESSOR_HPP_

#include "Python.h"

#include <stdio.h>
#include <stdlib.h>
#include <iterator>

#include <dds/core/corefwd.hpp>
#include <dds/core/xtypes/DynamicData.hpp>
#include <dds/core/xtypes/StructType.hpp>
#include <dds/core/Optional.hpp>
#include <rti/routing/processor/ProcessorPlugin.hpp>
#include "NativeUtils.hpp"
#include "PyRoute.hpp"

namespace rti { namespace routing { namespace py {

class PyProcessorPluginProperty {
public:
    PyProcessorPluginProperty();

    PyProcessorPluginProperty(
            const std::string& class_name,
            const std::string& module);

    void class_name(const std::string& class_name);
    const std::string& class_name() const;
    void module_path(const std::string& module_path);
    const std::string& module_path() const;
    void module(const std::string& module_name);
    const std::string& module() const;
    void autoreload(bool value);
    bool autoreload() const;

private:
    std::string class_name_;
    std::string module_;
    std::string module_path_;
    bool module_autoreload_;
};

class PyProcessor;
class PyProcessorPlugin {

public:
    static const std::string BASE_PROCESSOR_TYPE_NAME;
    static const std::string MODULE_PROPERTY_NAME;
    static const std::string MODULE_PATH_PROPERTY_NAME;
    static const std::string CLASS_NAME_PROPERTY_NAME;
    static const std::string MODULE_AUTORELOAD_PROPERTY_NAME;
    static const std::string MODULE_PATH_VALUE_DEFAULT;

    PyProcessorPlugin(
            const struct RTI_RoutingServiceProperties *native_properties);

    ~PyProcessorPlugin();

    const PyProcessorPluginProperty& property() const;

    void reload();

    PyObject* create_processor(
            PyRoute *route,
            PyObject *properties);

    /* --- */
    static RTI_RoutingServiceProcessorPlugin * create_plugin(
            PyProcessorPlugin *plugin);

    static void delete_plugin(
            RTI_RoutingServiceProcessorPlugin *native_plugin,
            RTI_RoutingServiceEnvironment *);

    static RTI_RoutingServiceProcessor * forward_create_processor(
            void *native_plugin_data,
            RTI_RoutingServiceRoute *native_route,
            const struct RTI_RoutingServiceProperties *native_properties,
            RTI_RoutingServiceEnvironment *environment);

    static void forward_delete_processor(
            void *native_plugin_data,
            struct RTI_RoutingServiceProcessor *native_processor,
            RTI_RoutingServiceRoute *,
            RTI_RoutingServiceEnvironment *environment);

private:
    template <typename PYOBJECTTYPE> void add_type();
    void load_module();
    PyObject* find_pyproc_type(const std::string& name);
private:
    friend class PyProcessor;
    PyProcessorPluginProperty property_;
    /* user module */
    PyObjectGuard py_user_module_;
    /* Reference to the Processor base type*/
    PyObject *pyproc_type_;
    /*
     * @brief Reference to the create_processor function
     */
    PyObject *processor_class_;
    /* indicates whether module is reloading before each event */
    bool autoreload_;
};


class PyProcessor {

public:
    static RTI_RoutingServiceProcessor * create_native(
            PyProcessorPlugin *plugin,
            RTI_RoutingServiceRoute *native_route,
            const struct RTI_RoutingServiceProperties *native_properties,
            RTI_RoutingServiceEnvironment *environment);

    static void delete_native(
            PyProcessorPlugin *plugin,
            RTI_RoutingServiceProcessor *native_processor,
            RTI_RoutingServiceEnvironment *environment);

    static void forward_on_route_event(
        void *native_processor_data,
        RTI_RoutingServiceRouteEvent *native_route_event,
        RTI_RoutingServiceEnvironment *environment);

    RTI_RoutingServiceProcessor* native();

    PyProcessor(
            PyProcessorPlugin *plugin,
            PyObject *py_processor,
            PyRoute *py_route);

    ~PyProcessor();

    PyObject* get();

private:
    PyProcessorPlugin *plugin_;
    RTI_RoutingServiceProcessor native_;
    // Reference to the Python processor implementation
    PyObject *py_processor_;
    // Reference to the Python Route
    PyRoute *py_route_;
};


/**
 * This macro defines a C-linkage symbol that can be used as create function
 * for plug-in registration through XML.
 *
 * The generated symbol has the name:
 *
 * \code
 * PyProcessorPlugin_create_processor_plugin
 * \endcode
 */
RTI_PROCESSOR_PLUGIN_CREATE_FUNCTION_DECL(PyProcessorPlugin);

}}}

#endif
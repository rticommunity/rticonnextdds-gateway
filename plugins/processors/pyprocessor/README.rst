.. |br| raw:: html

   <br />


********************
Processor Python API
********************

A Python binding for the Routing Service Processor API. Implemented as a
native (C/C++) plug-in that provides the conversions to and from Python.

Concept
=======

*Routing Service Processor* is a pluggable-component that allows controlling the
forwarding process that occurs within *Routes*. The *Routing Service Software
Development Kit* offers a set of APIs that you can implement to customize
the *Route* behavior. See `Chapter SDK in Routing Service User's Manual
<https://community.rti.com/static/documentation/connext-dds/6.0.0/images/manuals/routing_service/sdk.html>`_
for a list of officially supported languages.

This plug-in provides an implementation of a *Processor* (to provide an user-level
Python binding. The plug-in relies on the native interface of Python (known
as `ctype`).

The general model of a *Processor* is shown in figure below.

.. figure:: ./images/RouterProcessorComponent.svg
    :align: center
    :width: 700px


A *Processor* represents a multiple-input, multiple-output component attached
to a *Route*, which dispatches events to the *Processor*. A *Processor* can
access the N inputs and M outputs of the owner *Route*. Upon event
notification, the *Processor* can read data from any input, perform any
manipulation on the data, and write it on any of the outputs.

An example snippet is shown below. The example code is reading data from two
inputs, merging the data together to write a single output sample.

.. code-block::

    def on_data_available(self, route):
        samples_first = route.inputs[0].read()
        for first in samples:
            # read from the second input, the same instance of the sample to merge
            samples_second = route.inputs[1].read(dict(instance=first.info['instance_handle']))

            output_data = {
                'id':first.data['id'],
                'config':first['config'],
                'periodic':samples_second[0]['latency']
            }

            route.outputs[0].write(output_data)


Building the binding plug-in
============================

Requirements
------------

To build and run the python binding plug-in you will need:

- RTI Connext Professional version 6.0.1 or higher.
- CMake version 3.10 or higher
- A target platform supported by *RTI* *RoutingService*.
- Python 3.5

Steps
-----

In order to build this plug-in, you need to provide the following variables to
`CMake`:

-   `CONNEXTDDS_DIR`: pointing to the installation of RTI Connext DDS to be
    used.

-   `CONNEXTDDS_ARCH`: the RTI Connext DDS Target architecture to be used in
    your system.

-   `CMAKE_BUILD_TYPE`: specifies the build mode. Only Release is a valid value.

-   `BUILD_SHARED_LIBS`: specifies the link mode. Only ``ON`` is a valid value.

Build the example code by running the following command:

.. code-block:: bash

    mkdir build
    cd build
    cmake -DCONNEXTDDS_DIR=<connext dir> \
          -DCONNEXTDDS_ARCH=<connext architecture> \
          -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=ON ..
    cmake --build .

.. note::

    When using a multi-configuration generator, make sure you specify
    the `--config` parameter in your call to `cmake --build .`. In general,
    it's a good practice to always provide it.

In case you are using Windows x64, you have to add the option -A in the cmake
command as follow:

.. code-block:: bash

    cmake -DCONNEXTDDS_DIR=<connext dir> \
          -DCONNEXTDDS_ARCH=<connext architecture> \
          -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=ON .. \
          -A x64


Upon success it will create a shared library file with name ``(lib)rtipyroutingservice``
in the build directory.

Customizing the Build
---------------------

Configuring Connext DDS Installation Path and Architecture
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The CMake build infrastructure will try to guess the location of your Connext
DDS installation and the Connext DDS architecture based on the default settings
for your host platform.If you installed Connext DDS in a custom location, you
can use the ``CONNEXTDDS_DIR`` variable to indicate the path to your RTI Connext DDS
installation folder. For example:

.. code-block:: bash

    cmake -DCONNEXTDDS_DIR=/home/rti/rti_connext_dds-x.y.z ..


Also, If you installed libraries for multiple target architecture on your system
(i.e., you installed more than one target rtipkg), you can use the
``CONNEXTDDS_ARCH`` variable to indicate the architecture of the specific libraries
you want to link against. For example:

.. code-block:: bash

    cmake -DCONNEXTDDS_ARCH=x64Linux3gcc5.4.0 ..


Plug-in Usage
=============

The binding plug-in built in the previous step is a native dynamic library
that will provide the binding between the Python Processor code and *Routing
Service*.

In order to provide your Python Processor implementation, you will need to
load the binding plug-in as any other C/C++ plug-in Processor. That is, you
will need to register the plug-in in XML as follows:

.. code-block::xml

    <plugin_library name="PythonPluginLib">
        <processor_plugin name="PyProcessor">
            <dll>rtirspyprocessor</dll>
            <create_function>
                PyProcessorPlugin_create_processor_plugin
            </create_function>
            <property>
                <!-- list of configuration properties for this plug-in (See below) -->
            </property>
        </processor_plugin>
    </plugin_library>

and then refer to this plug-in from the Route where you want to install the
Python Processor

.. code-block::xml

    <processor plugin_name="PythonPluginLib::PyProcessor">
        <property>
            <!-- List of configuration properties that you can pass to your
                 Processor implementation
            -->
        </property>
    </processor>

The binding plug-in requires information about how to load your Python Processor
implementation. This information is provided through the plugin configuration
properties. The complete set of properties is shown in table below.

.. list-table:: Plugin Configuration Properties
    :name: TablePluginProperties
    :widths: 30 10 60
    :header-rows: 1

    * - Name |br|
        <base_name> = **rti.routing.proc.py**
      - Value
      - Description
    * - **<base_name>.class_name**
      - ``<string>``
      - Name of class that implements the Processor.
        Required
    * - **<base_name>.module.name**
      - ``<string>``
      - Name of the Python module (file) that contains the code of your
        Processor implementation. |br|
        Required.
    * - **<base_name>.module.path**
      - ``<string>``
      - Absolute or relative path to where the module file is located. |br|
        Optional |br|
        Default: **.** (working directory).
    * - **<base_name>.module.autoreload**
      - ``<boolean>``
      - Specifies whether the module that contains the Processor implementation
        is reloaded on each occurrence of an event dispatch. Note that that
        reloading a module will affect only to the methods on the Processor
        but not state of the Processor object already created. That is, new
        method code can be executed but on the existing Processor instance.

Creating your Processor
-----------------------

To make your Python *Processor* pluggable, you will need to define a class
that inherits from ``proc.Processor``. This is an abstract class that defines
an abstract method for each one of the possible event notifications from the
Route.

For example:

.. code-block:: Python

    import proc

    class MyProcessor(proc.Processor):
        def on_data_available(self, route):
            ...


API Overview
============

The Processor Python API component model is shown below.

.. figure:: ./images/RouterPythonProcessor.svg
    :align: center


The Python API model is based of two different of components:

- Classes: These are representations of *Routing Service* entities that provide
  behavior usable by *Processor* implementations. There are five classes:

    - ``Processor``: The plug-in implementation. This is an abstract class from which
      implementations inherit from to customize the behavior of *Routes*. Note that
      all the callbacks are optional, so you can implement those you need.
    - ``Route``: This class represent instances of the *Routes*, which provide access
      to the inputs and outputs within.
    - ``Input``: Representation of a *Route* input from which you can read data.
    - ``Output``: Representation of a *Route* output to which you can write data.
    - ``Sample``: Representation of the information returned by a read operation
      on an input.

- Dictionaries: These are representation of pure data structures, containing
  as member names as keys and member values as values:

   - ``Input`` and ``Output`` description properties (name, stream and type names, etc).
   - ``Sample``'s data and information elements
   - ``Selector`` objects used to read subsets of data from the `Input`


A ``Processor`` implementation can perform operations on any of the other
components as needed. The user provides an implementation of a ``Processor``
whereas the other objects are constructed and provided by *Routing Service* during
the execution of the ``Processor``


API Documentation
-----------------

Documentation for all the components of the Processor API is embedded in
the definition of the types in the `proc` module. You can build the documentation
to generate an html output as follows:

.. code-block:: bash

    cd ./api_doc
    sphinx-build . <output_dir>

where ``<output_dir`` is the directory path where you want the generated html
files to be placed.


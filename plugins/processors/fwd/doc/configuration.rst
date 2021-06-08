.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RSFWD|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the Forwarding Processor Plugin
====================================

|RSFWD| must be registered as a |RS| plugin by using the ``<processor_plugin>``
tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPlugins">
            <processor_plugin name="FwdByValue">
                <dll>rtifwdprocessor</dll>
                <create_function>
                    RTI_PRCS_FWD_ByInputValueForwardingEnginePlugin_create
                </create_function>
            </processor_plugin>
            <processor_plugin name="FwdByName">
                <dll>rtifwdprocessor</dll>
                <create_function>
                    RTI_PRCS_FWD_ByInputNameForwardingEnginePlugin_create
                </create_function>
            </processor_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |RSFWD| dynamic library
             (|FWD_PROCESSOR_LIB_NAME_LINUX| on Linux® systems,
             |FWD_PROCESSOR_LIB_NAME_MAC| on macOS® systems,
             or |FWD_PROCESSOR_LIB_NAME_WIN| on Windows® systems). Make
             sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on
             macOS systems, or ``PATH`` on Windows systems, etc.).

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up.

In the snippet above, there are two processors registered, one per
functionality of the |RSFWD|.

.. _section-processor-configuration-attributes:

Configuration Properties
------------------------

There are some properties in order to configure the |RSFWD|. Depending on the
forwarding method used (by name or by value), there is a different set of
properties you can configure.

Forwarding Processor by Name
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The |RSFWD| uses the following properties to configure its behavior when
using the *forwarding by name* method:

.. csv-table:: Forwarding by Name Configuration Properties
   :file: _static/csv/forwarding_input_name_properties.csv
   :widths: 25, 25, 50
   :header-rows: 1

The |ATTRIBUTE_INPUT| should contain the topic name from where the processor
is reading the samples. The |ATTRIBUTE_OUTPUT| contains the output topic name
where the sample will be publish into.

The following snippet shows how to configure this:

.. code-block:: xml

    <processor plugin_name="MqttShapesPlugins::FwdByName">
        <property>
            <value>
                <element>
                    <name>forwarding_table</name>
                    <value>
                        [
                            {
                                "input": "Square",
                                "output": "mqtt_squares"
                            },
                            {
                                "input": "Triangle",
                                "output": "mqtt_triangles"
                            },
                            {
                                "input": "Circle",
                                "output": "mqtt_circles"
                            }
                        ]
                    </value>
                </element>
            </value>
        </property>
    </processor>

Forwarding Processor by Value
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The |RSFWD| uses the following properties to configure its behavior when using
the *forwarding by value* method:

.. csv-table:: Forwarding by Value Configuration Properties
   :file: _static/csv/forwarding_input_value_properties.csv
   :widths: 25, 25, 50
   :header-rows: 1

In the property |PROP_INPUT_MEMBERS|, the |ATTRIBUTE_INPUT| should contain the
value of the field specified in |ATTRIBUTE_MEMBER| that will be forwarded. On
the other hard, the |PROP_FWD_TABLE| shall contain the |ATTRIBUTE_INPUT| and
|ATTRIBUTE_OUTPUT| where the sample will be forwarded from and to.

The |ATTRIBUTE_INPUT| may contain wildcards as this exapmle shows:

The following snippet shows how to configure this:

.. code-block:: xml

    <processor plugin_name="MqttShapesPlugins::FwdByValue">
        <property>
            <value>
                <element>
                    <name>input_members</name>
                    <value>
                        [
                            {
                                "input": "*",
                                "member": "topic"
                            }
                        ]
                    </value>
                </element>
                <element>
                    <name>forwarding_table</name>
                    <value>
                        [
                            {
                                "input": "*/squares",
                                "output": "Square"
                            },
                            {
                                "input": "*/triangles",
                                "output": "Triangle"
                            },
                            {
                                "input": "*/circles",
                                "output": "Circle"
                            }
                        ]
                    </value>
                </element>
            </value>
        </property>
    </processor>

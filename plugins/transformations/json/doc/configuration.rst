.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RSTSFM|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the |JSON_TRANSFORMATION_NAME| Plugin
===========================================

|RSTSFM| must be registered as a |RS| plugin by using the ``<transformation_plugin>``
tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPlugins">
            <transformation_plugin name="PrimitiveField">
            <dll>rtijsontransf</dll>
            <create_function>
                RTI_TSFM_Json_FlatTypeTransformationPlugin_create
            </create_function>
        </transformation_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |RSTSFM| dynamic library
             (|JSON_TRANSFORMATION_LIB_NAME_LINUX| on Linux systems,
             |JSON_TRANSFORMATION_LIB_NAME_MAC| on macOS systems,
             or |JSON_TRANSFORMATION_LIB_NAME_WIN| on Windows systems). Make
             sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on macOS
             systems, or ``Path`` on Windows systems).

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up.

.. _section-processor-configuration-attributes:

Configuration Properties
------------------------

The |RSTSFM| uses the following variables to configure its behavior:

.. csv-table:: JSON Transformation Configuration Properties
   :file: static/csv/json_transformation_properties.csv
   :widths: 30 15 15 40
   :header-rows: 1

The following snippet shows how to configure these properties:

.. code-block:: xml

    <transformation plugin_name="MqttShapesPlugins::JsonShapes">
        <input_type_name>RTI::MQTT::Message</input_type_name>
        <property>
            <value>
                <element>
                    <name>transform_type</name>
                    <value>deserialize</value>
                </element>
                <element>
                    <name>buffer_member</name>
                    <value>payload.data</value>
                </element>
            </value>
        </property>
    </transformation>

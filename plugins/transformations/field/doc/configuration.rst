.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RS_FIELD_TSFM|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the |FIELD_TRANSFORMATION_NAME| Plugin
===========================================

|RS_FIELD_TSFM| must be registered as a |RS| plugin by using the
``<transformation_plugin>`` tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPlugins">
            <transformation_plugin name="PrimitiveField">
            <dll>rtifieldtransf</dll>
            <create_function>
                RTI_TSFM_Field_PrimitiveTransformationPlugin_create
            </create_function>
        </transformation_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |RS_FIELD_TSFM| dynamic library
             (|FIELD_TRANSFORMATION_LIB_NAME_LINUX| on Linux® systems,
             |FIELD_TRANSFORMATION_LIB_NAME_MAC| on macOS® systems,
             or |FIELD_TRANSFORMATION_LIB_NAME_WIN| on Windows® systems). Make
             sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on
             macOS systems, or ``PATH`` on Windows systems, etc.).

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up.

.. _section-processor-configuration-attributes:

Configuration Properties
------------------------

The |RS_FIELD_TSFM| uses the following variables to configure its behavior:

.. csv-table:: Field Transformation Configuration Properties
   :file: static/csv/field_transformation_properties.csv
   :widths: 30 15 15 40
   :header-rows: 1

The following snippet shows how to configure these properties:

.. code-block:: xml

    <transformation plugin_name="MyPlugins::PrimitiveField">
        <property>
            <value>
                <element>
                    <name>buffer_member</name>
                    <value>payload.data</value>
                </element>
                <element>
                    <name>field</name>
                    <value>x</value>
                </element>
                <element>
                    <name>field_type</name>
                    <value>long</value>
                </element>
            </value>
        </property>
    </transformation>

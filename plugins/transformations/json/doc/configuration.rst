.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RS_JSON_TSFM|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Supported Data Types
====================

|RS_JSON_TSFM| requires users to specify the name of a member that the
transformation will either read as a JSON string (when parsing a DDS sample
from JSON), or set the contents of using a JSON string (obtained by converting
a DDS sample to JSON).

The type of this input/output members can be any of the following "string-like"
types:

* ``string``
* ``sequence<octet>``
* ``sequence<char>``
* ``octet[N]``
* ``char[N]``

When going from DDS to JSON, the transformation will always store a
"well-terminated" string in the output member (i.e. a string which ends with
a "nul" terminator, '\0').

When parsing DDS samples from JSON, the input string should be
properly terminated, but it doesn't need to be.

If the value retrieved from the input member does not already
end with a `\0`, the transformation will add one, possibly by first
allocating a sufficiently large buffer and then copying the original value into
it. This reallocation is only performed for ``sequence`` members, and it will
only be performed if the value does not already meet the maximum size defined
for those members (if not unbounded).

Load the JSON Transformation Plugin
===================================

|RS_JSON_TSFM| must be registered as a |RS| plugin by using the
``<transformation_plugin>`` tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPlugins">
            <transformation_plugin name="JsonTransformation">
            <dll>rtijsontransf</dll>
            <create_function>
                RTI_TSFM_JsonTransformationPlugin_create
            </create_function>
        </transformation_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |RS_JSON_TSFM| dynamic library
             (|JSON_TRANSFORMATION_LIB_NAME_LINUX| on Linux® systems,
             |JSON_TRANSFORMATION_LIB_NAME_MAC| on macOS® systems,
             or |JSON_TRANSFORMATION_LIB_NAME_WIN| on Windows® systems). Make
             sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on
             macOS systems, or ``PATH`` on Windows systems, etc.).

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up.

.. _section-processor-configuration-attributes:

Configuration Properties
------------------------

The |RS_JSON_TSFM| uses the following properties to configure its behavior:

.. csv-table:: JSON Transformation Configuration Properties
   :file: static/csv/json_transformation_properties.csv
   :widths: 15 15 15 15 40
   :header-rows: 1

For example, having the following type:

.. code-block:: xml

    <types>
        <struct name="MessagePayload">
            <member name="data" type="byte" sequenceMaxLength="-1"/>
        </struct>
        <struct name="MyType">
            <member name="payload" type="nonBasic" nonBasicTypeName="MessagePayload"/>
        </struct>
    </types>

We can configure the properties as follow:

.. code-block:: xml

    <transformation plugin_name="MyPlugins::JsonTransformation">
        <input_type_name>MyType</input_type_name>
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

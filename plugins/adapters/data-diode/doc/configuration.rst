.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |DATADIODE_NAME|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the Data Diode Adapter Plugin
==================================

|DATADIODE_NAME| must be registered as a |RS| plugin by using the ``<adapter_plugin>``
tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <adapter_library name="AdapterLib">
        <adapter_plugin name="SocketAdapter">
            <dll>rtidatadiodeadapter</dll>
            <create_function>SocketAdapter_create_adapter_plugin</create_function>
        </adapter_plugin>
    </adapter_library>
    </dds>

.. warning:: |RS| must be able to find the |DATADIODE_NAME| dynamic library
             (|DATADIODE_ADAPTER_LIB_NAME_LINUX| on Linux® systems,
             |DATADIODE_ADAPTER_LIB_NAME_MAC| on macOS® systems,
             or |DATADIODE_ADAPTER_LIB_NAME_WIN| on Windows® systems). Make
             sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on
             macOS systems, or ``PATH`` on Windows systems, etc.).

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up, and
you can use the plugin to create one or more connections to |DATADIODE_DEVICEs|.

.. _section-how-to-config-connection:

Socket Connection configuration
===============================

Once the plugin has been registered with |RS|, you can use it to create
``<connection>`` elements within a ``<domain_route>``.

A Connection identifies a UDP socket to send or to receive data.
Depending on whether it is an input or an output the configuration varies


.. _section-socet-dds:

UDP socket to DDS
-----------------

The following snippet shows an example ``<connection>`` that received serialized data and sends it to a DDS endpoint.

The input requires setting the receive address and port.

The output requires setting the type, topic name and QoS of the DDS data writer.

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <route>
                <input connection="SocketConnection">
                    <creation_mode>ON_ROUTE_MATCH</creation_mode>
                    <registered_type_name>PingType</registered_type_name>
                    <stream_name>DataDiode</stream_name>
                    <property>
                        <value>
                            <element>
                                <name>receive_address</name>
                                <value>127.0.0.1</value>
                            </element>
                            <element>
                                <name>receive_port</name>
                                <value>10203</value>
                            </element>
                        </value>
                    </property>
                </input>
                <dds_output participant="DDSConnectionOutput">
                        <creation_mode>ON_DOMAIN_MATCH</creation_mode>
                        <registered_type_name>PingType</registered_type_name>
                        <topic_name>PingTopic</topic_name>
                        <datawriter_qos base_name="BuiltinQosLib::Generic.Common" />
                    </dds_output>
        </routing_service>
    </dds>


.. _section-dds-socket:

DDS to UDP socket
-----------------

The following snippet shows an example ``<connection>`` that received DDS data, serializes it and sends it to remote UDP socket.

The input requires setting the type, topic name and QoS of the DDS data writer.

The output requires setting the send and destination address and port.

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <route>
                <dds_input participant="DDSConnectionInput">
                    <creation_mode>ON_DOMAIN_MATCH</creation_mode>
                    <registered_type_name>PingType</registered_type_name>
                    <topic_name>PingTopic</topic_name>
                    <datareader_qos base_name="BuiltinQosLib::Generic.Common" />
                </dds_input>
                <output connection="SocketConnection">
                        <creation_mode>ON_ROUTE_MATCH</creation_mode>
                        <registered_type_name>PingType</registered_type_name>
                        <stream_name>DataDiode</stream_name>
                        <property>
                            <value>
                                <element>
                                    <name>send_address</name>
                                    <value>127.0.0.1</value>
                                </element>
                                <element>
                                    <name>send_port</name>
                                    <value>0</value>
                                </element>
                                <element>
                                    <name>dest_address</name>
                                    <value>127.0.0.1</value>
                                </element>
                                <element>
                                    <name>dest_port</name>
                                    <value>10203</value>
                                </element>
                            </value>
                        </property>
                    </output>
        </routing_service>
    </dds>

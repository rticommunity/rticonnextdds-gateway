.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RSKAFKA|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the Kafka Adapter Plugin
=============================

|RSKAFKA| must be registered as a |RS| plugin by using the ``<adapter_plugin>``
tag.

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up, and
you can use the plugin to create one or more connections to a |KAFKA_BROKER|.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPlugins">
            <adapter_plugin name="KafkaAdapter">
                <dll>rtikafkaadapter</dll>
                <create_function>RTI_RS_Kafka_AdapterPlugin_create</create_function>
            </adapter_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |RSKAFKA| dynamic library
             (|KAFKA_ADAPTER_LIB_NAME_LINUX| on Linux® systems,
             |KAFKA_ADAPTER_LIB_NAME_MAC| on macOS® systems,
             or |KAFKA_ADAPTER_LIB_NAME_WIN| on Windows® systems). Make
             sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on
             macOS systems, or ``PATH`` on Windows systems, etc.).

.. _section-register-kafka-datatypes:

Register KAFKA Data Types
=========================

|RSKAFKA| exposes data to |RS| using the |DDS_TYPE| named:
``RTI::Kafka::Message``.

This data type needs to be registered. It is used to store converted DDS data in
a format that a Kafka application can understand (e.g., JSON). The format can be
configured with a transformation plugin.

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <types>
            <module name="RTI">
                <module name="Kafka">
                    <struct name="MessagePayload" nested="true">
                        <member name="data" sequenceMaxLength="-1" type="byte" />
                    </struct>
                    <struct name="Message">
                        <member name="payload" type="nonBasic" nonBasicTypeName="RTI::Kafka::MessagePayload" />
                    </struct>
                </module>
            </module>
        </types>
    </dds>


.. _section-adapter-qos:

Configure QoS
=============

The types used by |RSKAFKA| use "unbounded" sequences.

The following QoS parameters must be configured in order to enable use of
"unbounded" types:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <qos_library name="MyQosLib">
            <qos_profile name="MyQos" is_default_qos="true">
                <datareader_qos>
                    <reader_resource_limits>
                        <dynamically_allocate_fragmented_samples>
                            true
                        </dynamically_allocate_fragmented_samples>
                    </reader_resource_limits>
                    <property>
                        <value>
                            <element>
                                <name>dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size</name>
                                <!-- Modify this value according to your preference -->
                                <value>10485760</value>
                            </element>
                        </value>
                    </property>
                </datareader_qos>
                <datawriter_qos>
                    <property>
                        <value>
                            <element>
                                <name>dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size</name>
                                <!-- Modify this value according to your preference -->
                                <value>10485760</value>
                            </element>
                        </value>
                    </property>
                </datawriter_qos>
            </qos_profile>
        </qos_library>
    </dds>


.. _section-connect-kafka-broker:

Connect to a |KAFKA_BROKER|
===========================

Once the plugin has been registered with |RS|, it can be used to create
``<connection>`` elements within a ``<domain_route>``.

The ``<connection>``'s configuration must include properties to configure the
associated |KAFKA_CLIENT|.

The ``bootstrap.servers`` property should contains a comma-separated list of
host and port pairs that are the addresses of the |KAFKA_BROKERs| in a
"bootstrap" *Kafka cluster* that a |KAFKA_CLIENT| connects to initially to
bootstrap itself. A host and port pair uses : as the separator
(e.g., "localhost:9092, another.host:9092").


The following snippet shows an example ``<connection>`` that connects to
a local |KAFKA_BROKER|:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <domain_route name="domain_route">
                <participant name="dds">
                    <domain_id>0</domain_id>
                    <registered_type name="ShapeType" type_name="ShapeType" />
                    <registered_type name="RTI::Kafka::Message" type_name="RTI::Kafka::Message" />
                </participant>
                <connection name="kafka" plugin_name="MyPlugins::KafkaAdapter">
                    <property>
                        <value>
                            <element>
                                <name>bootstrap.servers</name>
                                <value>localhost:9092</value>
                            </element>
                        </value>
                    </property>
                    <registered_type name="ShapeType" type_name="ShapeType" />
                    <registered_type name="RTI::Kafka::Message" type_name="RTI::Kafka::Message" />
                </connection>
            </domain_route>
        </routing_service>
    </dds>


.. _section-how-to-route-kafka-to-dds:

Route Data from Kafka to DDS
============================

A ``<connection>`` created by |RSKAFKA| can be used to define ``<input>``
elements which will subscribe to |KAFKA_MESSAGEs| from the |KAFKA_BROKER|, and
expose them to the enclosing ``<route>`` as |DDS_SAMPLES| of
``RTI::Kafka::Message``.

The specific |DDS_TYPE| to use is determined from the value of the
``<registered_type_name>`` element.

The |KAFKA_CLIENT| associated with the ``<connection>`` will be used to create
a subscription on the |KAFKA_BROKER| for the topic name defined in the the
``topic`` property.

The following snippet demonstrate how to create an ``<input>`` which will
subscribe to a topic named ``"kafka_dds"`` available on the |KAFKA_BROKER|, and
route them to |DDS_TOPIC| "dds_kafka" using DDS Type ``RTI::Kafka:Message``. The
|KAFKA_MESSAGEs| in this example are formatted in JSON, so we use the JSON
transformation plugin to convert the |KAFKA_MESSAGEs| in JSON to DDS samples.

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <session>
                <route>
                    <input connection="kafka">
                        <registered_type_name>RTI::Kafka::Message</registered_type_name>
                        <property>
                            <value>
                                <element>
                                    <name>topic</name>
                                    <value>kafka_dds</value>
                                </element>
                            </value>
                        </property>
                    </input>
                    <dds_output name="dds_kafka" participant="dds">
                        <registered_type_name>ShapeType</registered_type_name>
                        <transformation plugin_name="plugin_library::json_transformation">
                            <input_type_name>RTI::Kafka::Message</input_type_name>
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
                    </dds_output>
                </route>
            </session>
        </routing_service>
    </dds>


.. _section-how-to-route-dds-to_kafka:

Route Data from DDS to Kafka
============================

A ``<connection>`` created by |RSKAFKA| can be used to define ``<output>``
elements that will convert |DDS_SAMPLES| into |KAFKA_MESSAGEs|, and publish
them to a topic on the ``<connection>``'s |KAFKA_BROKER|.

|DDS_SAMPLES| must be of type ``RTI::Kafka::Message``.

The following snippet demonstrate how to configure an ``<output>`` to
publish the payload of ``RTI::Kafka::Message`` |DDS_SAMPLES| published
on topic ``"dds_kafka"`` to |KAFKA_TOPIC| ``"kafka_dds"``. The |KAFKA_MESSAGEs|
in this example are formatted in JSON, so we use the JSON transformation plugin
to convert the |DDS_SAMPLES| to |KAFKA_MESSAGEs| in JSON.

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <session>
                <route>
                    <dds_input name="dds_kafka" participant="dds">
                        <registered_type_name>ShapeType</registered_type_name>
                    </dds_input>
                    <output connection="kafka">
                        <registered_type_name>RTI::Kafka::Message</registered_type_name>
                        <property>
                            <value>
                                <element>
                                    <name>topic</name>
                                    <value>kafka_dds</value>
                                </element>
                            </value>
                        </property>
                        <transformation plugin_name="plugin_library::json_transformation">
                            <input_type_name>ShapeType</input_type_name>
                            <property>
                                <value>
                                    <element>
                                        <name>transform_type</name>
                                        <value>serialize</value>
                                    </element>
                                    <element>
                                        <name>buffer_member</name>
                                        <value>payload.data</value>
                                    </element>
                                </value>
                            </property>
                        </transformation>
                    </output>
                </route>
            </session>
        </routing_service>
    </dds>


.. _section-adapter-xml-properties:

Kafka Configuration properties
==============================

This section describes the properties that can be used to configure |RSKAFKA|.

.. _section-adapter-kafka-properties-client:

:litrep:`<connection>` Properties
---------------------------------

Each ``<connection>`` created by the |RSKAFKA| plugin is associated with a
|KAFKA_CLIENT| which must be configured using the ``<properties>`` tag.

.. list-table:: :litrep:`<connection>` Properties
    :widths: 20 10 20 20 30
    :header-rows: 1

    * - Property
      - Required
      - Default
      - Type
      - Description
    * - ``bootstrap.servers``
      - Yes
      - None
      - string
      - Initial list of brokers as a CSV list of broker host or host:port.

:litrep:`<input>` Properties
----------------------------

``<input>`` created by the |RSKAFKA| plugin is associated with a Kafka Consumer
which can be configured using the ``<properties>`` tag.

.. list-table:: :litrep:`<input>` Properties
    :widths: 20 10 20 20 30
    :header-rows: 1

    * - Property
      - Required
      - Default
      - Type
      - Description
    * - ``topic``
      - Yes
      - None
      - string
      - A topic name used by a Kafka Consumer.

:litrep:`<output>` Properties
-----------------------------

``<output>`` created by the |RSKAFKA| plugin is associated with a Kafka Producer
which can be configured using the ``<properties>`` tag.

.. list-table:: :litrep:`<input>` Properties
    :widths: 20 10 20 20 30
    :header-rows: 1

    * - Property
      - Required
      - Default
      - Type
      - Description
    * - ``topic``
      - Yes
      - None
      - string
      - A topic name used by a Kafka Producer.

librdkafka Producer/Consumer Properties
---------------------------------------

Librdkafka supports several configuration properties for Kafka producers and
consumers that can be used as |RS| ``<output>`` and ``<input>`` properties. The
full list of these ``librdkafka`` properties can be found
`in this table <https://github.com/rticommunity/librdkafka/blob/1.6.1/CONFIGURATION.md>`_.

.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RSMQTT|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the MQTT Adapter Plugin
============================

|RSMQTT| must be registered as a plugin on |RS| using the ``<adapter_plugin>``
tag.

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up, and
the plugin can be used to create one or more connections to an |MQTT_BROKER|.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPlugins">
            <adapter_plugin name="Mqtt">
                <dll>rtimqttadapter</dll>
                <create_function>
                    RTI_RS_MQTT_AdapterPlugin_create
                </create_function>
            </adapter_plugin>
        </plugin_library>
    </dds>

.. warning:: The |RSMQTT| dynamic library (``librtimqttadapter.so`` on Linux
             and Darwin, ``rtimqttadapter.dll`` on Windows) must be found
             within |RS|'s dynamic library path. Make sure to include the
             library's directory in the environment variable appropriate to
             your system (``LD_LIBRARY_PATH`` on Linux, ``DYLD_LIBRARY_PATH``
             on Darwin, ``PATH`` on Windows, etc.).

.. _section-adapter-data-types:

Register MQTT Data Types
========================

|RSMQTT| exposes data to |RS| using two possible |DDS_TYPES|:
``RTI::MQTT::Message`` and ``RTI::MQTT:KeyedMessage``.

The IDL definition of these types is included with |RSMQTT|, and built in
into the library, but it is necessary to register them in |RS|'s XML
configuration to make it available outside of the adapter:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <types>
            <module name="RTI">
                <module name="MQTT">
                    <enum name="QosLevelKind">
                        <enumerator name="RTI_MQTT_QosLevel_UNKNOWN"/>
                        <enumerator name="RTI_MQTT_QosLevel_ZERO"/>
                        <enumerator name="RTI_MQTT_QosLevel_ONE"/>
                        <enumerator name="RTI_MQTT_QosLevel_TWO"/>
                    </enum>
                    <struct name= "MessageInfo" nested="true">
                        <member name="id" type="int32"/>
                        <member name="qos_level" type="nonBasic"
                                nonBasicTypeName="RTI::MQTT::QosLevelKind"/>
                        <member name="retained" type="boolean"/>
                        <member name="duplicate" type="boolean"/>
                    </struct>
                    <struct name= "MessagePayload" nested="true">
                        <member name="data" sequenceMaxLength="-1" type="byte"/>
                    </struct>
                    <struct name= "Message">
                        <member name="topic" stringMaxLength="-1"
                                type="string" optional="true"/>
                        <member name="info" type="nonBasic"
                                nonBasicTypeName="RTI::MQTT::MessageInfo"
                                optional="true"/>
                        <member name="payload" type="nonBasic"
                                nonBasicTypeName="RTI::MQTT::MessagePayload"/>
                    </struct>
                    <struct name= "KeyedMessage">
                        <member name="topic" stringMaxLength="-1"
                                type="string" key="true"/>
                        <member name="info" type="nonBasic"
                                nonBasicTypeName="RTI::MQTT::MessageInfo"
                                optional="true"/>
                        <member name="payload" type="nonBasic"
                                nonBasicTypeName= "RTI::MQTT::MessagePayload"/>
                    </struct>
                </module>
            </module>
        </types>
    </dds>


.. _section-adapter-qos:

Configure QoS
=============

The types used by |RSMQTT| make used of "unbounded" strings and sequences.

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



.. _section-how-to-config-connection:

Connect to an |MQTT_BROKER|
===========================

Once the plugin has been registered with |RS|, it can be used to create
``<connection>`` elements within a ``<domain_route>``.

The ``<connection>``'s configuration must include properties ``client.id``,
and ``client.servers`` to configure the associated |MQTT_CLIENT|.

The following snippet shows an example ``<connection>`` that connects to
a local |MQTT_BROKER|:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <domain_route>
                <connection name="mqtt" plugin_name="MyPlugins::Mqtt">
                    <property>
                        <value>
                            <element>
                                <name>client.id</name>
                                <value>mqtt_router</value>
                            </element>
                            <element>
                                <name>client.servers</name>
                                <value>tcp://127.0.0.1:1883</value>
                            </element>
                        </value>
                    </property>
                    <registered_type name="RTI::MQTT::Message"
                                     type_name="RTI::MQTT::Message"/>
                    <registered_type name="RTI::MQTT::KeyedMessage"
                                     type_name="RTI::MQTT::KeyedMessage"/>
                </connection>
                <!-- Create a DDS DomainParticipant to connect other
                     applications in the Domain to the MQTT Clients
                     connected to the Broker -->
                <participant name="dds">
                    <domain_id>0</domain_id>
                    <registered_type name="RTI::MQTT::Message"
                                     type_name="RTI::MQTT::Message"/>
                    <registered_type name="RTI::MQTT::KeyedMessage"
                                     type_name="RTI::MQTT::KeyedMessage"/>
                </participant>
            </domain_route>
        </routing_service>
    </dds>



.. _section-how-to-route-mqtt-to-dds:

Route Data from MQTT to DDS
===========================

A ``<connection>`` created by |RSMQTT| can be used to define ``<input>``
elements which will subscribe to |MQTT_MESSAGES| from the |MQTT_BROKER|, and
expose them to the enclosing ``<route>`` as |DDS_SAMPLES| of either
``RTI::MQTT::Message``, or ``RTI::MQTT::KeyedMessage``.

The specific |DDS_TYPE| to use is determined from the value of the
``<registered_type_name>`` element.

The |MQTT_CLIENT| associated with the ``<connection>`` will be used to create
subscriptions on the |MQTT_BROKER| for every topic filter specified in
property ``subscription.topics``.

The value of this property is a list of semicolon-separated
|MQTT_TOPIC_FILTERS|.

The following snippet demonstrate how to create an ``<input>`` which will
subscribe to all topics available on the |MQTT_BROKER|, and route them
to |DDS_TOPIC| ``"dds_mqtt"`` using |DDS_TYPE| ``RTI::MQTT:KeyedMessage``:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <session>
                <route>
                    <input name="mqtt_reader" connection="mqtt">
                        <registered_type_name>
                          RTI::MQTT::KeyedMessage
                        </registered_type_name>
                        <property>
                            <value>
                                <element>
                                    <name>subscription.topics</name>
                                    <value>#</value>
                                </element>
                            </value>
                        </property>
                    </input>
                    <dds_output name="dds_mqtt" participant="dds">
                        <registered_type_name>
                            RTI::MQTT::KeyedMessage
                        </registered_type_name>
                    </dds_output>
                </route>
            </session>
        </routing_service>
    </dds>


.. warning:: While possible, it is discouraged to specify the same topic
             filter on multiple ``<input>`` elements belonging to the same
             ``<connection>``. Each duplicated topic filter will result in a
             single subscription on the |MQTT_BROKER| with settings from the
             last ``<input>`` to be created.

.. _section-how-to-route-dds-to_mqtt:

Route Data from DDS to MQTT
===========================

A ``<connection>`` created by |RSMQTT| can be used to define ``<output>``
elements that will convert |DDS_SAMPLES| into |MQTT_MESSAGES|, and publish
them to a topic on the ``<connection>``'s |MQTT_BROKER|.

|DDS_SAMPLES| must be of type ``RTI::MQTT::Message``, or
``RTI::MQTT::KeyedMessage``.

The following snippet demonstrate how to configure an ``<output>`` to
publish the payload of ``RTI::MQTT::KeyedMessage`` |DDS_SAMPLES| published
on topic ``"dds_mqtt"`` to |MQTT_TOPIC| ``"foo/bar"``, with |MQTT_QOS| 1:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <session>
                <route>
                    <dds_input name="dds_mqtt" participant="dds">
                        <registered_type_name>
                            RTI::MQTT::KeyedMessage
                        </registered_type_name>
                    </dds_input>
                    <output name="mqtt_writer" connection="mqtt">
                        <registered_type_name>
                            RTI::MQTT::KeyedMessage
                        </registered_type_name>
                        <property>
                            <value>
                                <element>
                                    <name>publication.topic</name>
                                    <value>foo/bar</value>
                                </element>
                                <element>
                                    <name>publication.qos</name>
                                    <value>1</value>
                                </element>
                            </value>
                        </property>
                    </input>
                </route>
            </session>
        </routing_service>
    </dds>

Alternatively, property ``publication.use_message_info`` be used to have
the ``<output>`` use the metadata contained in published |DDS_SAMPLES| to
determine other publication setttings (e.g. topic, qos, retained flags, etc.):

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <session>
                <route>
                    <dds_input name="dds_mqtt" participant="dds">
                        <registered_type_name>
                            RTI::MQTT::KeyedMessage
                        </registered_type_name>
                    </dds_input>
                    <output name="mqtt_writer" connection="mqtt">
                        <registered_type_name>
                            RTI::MQTT::KeyedMessage
                        </registered_type_name>
                        <property>
                            <value>
                                <element>
                                    <name>publication.use_message_info</name>
                                    <value>true</value>
                                </element>
                            </value>
                        </property>
                    </input>
                </route>
            </session>
        </routing_service>
    </dds>

.. _section-adapter-xml-properties:

XML Configuration properties
============================

This section describes the properties that can be used to configure |RSMQTT|.

.. _section-adapter-xml-properties-client:

:litrep:`<connection>` Properties
---------------------------------

Each ``<connection>`` created by the |RSMQTT| plugin is associated with an
|MQTT_CLIENT| which must be configured using the ``<properties>`` tag.

Most configuration properties have "reasonable" default values, except for
``client.id``, and ``client.servers`` which must be always specified.

.. list-table:: :litrep:`<connection>` Properties
    :widths: 90 10
    :header-rows: 1

    * - Property
      - Required
    * - :ref:`section-adapter-xml-properties-client-id`
      - Yes
    * - :ref:`section-adapter-xml-properties-client-servers`
      - Yes
    * - :ref:`section-adapter-xml-properties-client-protocol`
      - No
    * - :ref:`section-adapter-xml-properties-client-conntimeout-sec`
      - No
    * - :ref:`section-adapter-xml-properties-client-conntimeout-nsec`
      - No
    * - :ref:`section-adapter-xml-properties-client-maxconnretries`
      - No
    * - :ref:`section-adapter-xml-properties-client-keepalive-sec`
      - No
    * - :ref:`section-adapter-xml-properties-client-keepalive-nsec`
      - No
    * - :ref:`section-adapter-xml-properties-client-clean`
      - No
    * - :ref:`section-adapter-xml-properties-client-unsub`
      - No
    * - :ref:`section-adapter-xml-properties-client-maxreply-sec`
      - No
    * - :ref:`section-adapter-xml-properties-client-maxreply-nsec`
      - No
    * - :ref:`section-adapter-xml-properties-client-reconnect`
      - No
    * - :ref:`section-adapter-xml-properties-client-maxunack`
      - No
    * - :ref:`section-adapter-xml-properties-client-persistence`
      - No
    * - :ref:`section-adapter-xml-properties-client-persistencestorage`
      - No
    * - :ref:`section-adapter-xml-properties-client-username`
      - No
    * - :ref:`section-adapter-xml-properties-client-password`
      - No
    * - :ref:`section-adapter-xml-properties-client-ssl-ca`
      - No
    * - :ref:`section-adapter-xml-properties-client-ssl-id`
      - No
    * - :ref:`section-adapter-xml-properties-client-ssl-key`
      - No
    * - :ref:`section-adapter-xml-properties-client-ssl-key-passwd`
      - No
    * - :ref:`section-adapter-xml-properties-client-ssl-proto`
      - No
    * - :ref:`section-adapter-xml-properties-client-ssl-verifycert`
      - No
    * - :ref:`section-adapter-xml-properties-client-ssl-cyphers`
      - No

.. _section-adapter-xml-properties-client-id:

client.id
^^^^^^^^^

:Required: Yes
:Default: None
:Description: The "client id" that will be sent to the |MQTT_BROKER| during
              connection. The value must be unique within all clients
              connected to the same |MQTT_BROKER|.
:Accepted Values: Any non-empty alphanumeric string

.. _section-adapter-xml-properties-client-servers:

client.servers
^^^^^^^^^^^^^^

:Required: Yes
:Default: None
:Description: A list of |MQTT_BROKER| URIs to which the |MQTT_CLIENT| will try
              to connect, in the specified order, until a connection can be
              successfully established.
:Accepted Values: A list of |MQTT_BROKER| URIs, separated by semicolon. Each
                  URI takes the form ``<protocol>://<address>:<port>``, where
                  ``<protocol>`` can be one of:

                  - ``tcp``: connect to |MQTT_BROKER| using a TCP connection.

                  - ``ssl``: connect to |MQTT_BROKER| using an SSL/TLS
                    connection.

.. note:: ``tcp://127.0.0.1:1883`` is usually the default URI to connect to an
          |MQTT_BROKER| deployed on your local machine (e.g. if you are using
          |MOSQUITTO|).

.. _section-adapter-xml-properties-client-protocol:

client.protocol_version
^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``default``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-conntimeout-sec:

client.connection_timeout.sec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``10``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-conntimeout-nsec:

client.connection_timeout.nanosec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``0``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-maxconnretries:

client.max_connection_retries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``10``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-keepalive-sec:

client.keep_alive_period.sec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``10``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-keepalive-nsec:

client.keep_alive_period.nanosec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``0``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-clean:

client.clean_session
^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``false``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-unsub:

client.unsubscribe_on_disconnect
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``true``
:Description:
:Accepted values:


.. _section-adapter-xml-properties-client-maxreply-sec:

client.max_reply_timeout.sec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``10``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-maxreply-nsec:

client.max_reply_timeout.nanosec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``10``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-reconnect:

client.reconnect
^^^^^^^^^^^^^^^^

:Required: No
:Default: ``false``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-maxunack:

client.max_unack_messages
^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``10``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-persistence:

client.persistence
^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``none``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-persistencestorage:

client.persistence_storage
^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-username:

client.username
^^^^^^^^^^^^^^^

:Required: No
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-password:

client.password
^^^^^^^^^^^^^^^

:Required: No
:Default: None
:Description:
:Accepted values:


.. _section-adapter-xml-properties-client-ssl-ca:

client.ssl.ca
^^^^^^^^^^^^^

:Required: No
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-ssl-id:

client.ssl.id
^^^^^^^^^^^^^

:Required: No
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-ssl-key:

client.ssl.key
^^^^^^^^^^^^^^

:Required: No
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-ssl-key-passwd:

client.ssl.key_password
^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-ssl-proto:

client.ssl.protocol_version
^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``default``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-ssl-verifycert:

client.ssl.verify_server_certificate
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``true``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-client-ssl-cyphers:

client.ssl.cypher_suites
^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``"ALL"``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-sub:

:litrep:`<input>` Properties
----------------------------

.. list-table:: :litrep:`<input>` Properties
    :widths: 90 10
    :header-rows: 1

    * - Property
      - Required
    * - :ref:`section-adapter-xml-properties-sub-topics`
      - Yes
    * - :ref:`section-adapter-xml-properties-sub-maxqos`
      - No
    * - :ref:`section-adapter-xml-properties-sub-queuesize`
      - No

.. _section-adapter-xml-properties-sub-topics:

subscription.topics
^^^^^^^^^^^^^^^^^^^

:Required: Yes
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-sub-maxqos:

subscription.max_qos
^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``2``
:Description:
:Accepted values:


.. _section-adapter-xml-properties-sub-queuesize:

subscription.queue_size
^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``0``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-pub:

:litrep:`<output>` Properties
-----------------------------

.. list-table:: :litrep:`<input>` Properties
    :widths: 90 10
    :header-rows: 1

    * - Property
      - Required
    * - :ref:`section-adapter-xml-properties-pub-topic`
      - Yes*
    * - :ref:`section-adapter-xml-properties-pub-qos`
      - No
    * - :ref:`section-adapter-xml-properties-pub-retained`
      - No
    * - :ref:`section-adapter-xml-properties-pub-usemsginfo`
      - Yes*
    * - :ref:`section-adapter-xml-properties-pub-maxwait-sec`
      - No
    * - :ref:`section-adapter-xml-properties-pub-maxwait-nsec`
      - No

.. _section-adapter-xml-properties-pub-topic:

publication.topic
^^^^^^^^^^^^^^^^^

:Required: Yes (if :ref:`section-adapter-xml-properties-pub-usemsginfo` is
           not enabled).
:Default: None
:Description:
:Accepted values:

.. _section-adapter-xml-properties-pub-qos:

publication.qos
^^^^^^^^^^^^^^^

:Required: No
:Default: ``0``
:Description:
:Accepted values:


.. _section-adapter-xml-properties-pub-retained:

publication.retained
^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``false``
:Description:
:Accepted values:


.. _section-adapter-xml-properties-pub-usemsginfo:

publication.use_message_info
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: Yes (if :ref:`section-adapter-xml-properties-pub-topic` is empty).
:Default: ``false``
:Description:
:Accepted values:


.. _section-adapter-xml-properties-pub-maxwait-sec:

publication.max_wait_time.sec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``10``
:Description:
:Accepted values:

.. _section-adapter-xml-properties-pub-maxwait-nsec:

publication.max_wait_time.nanosec
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``0``
:Description:
:Accepted values:
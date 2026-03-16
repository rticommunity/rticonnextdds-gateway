.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |DIA|, which is based on |RS|. 
This adapter is configured like any other |RS|-based component. 

All configuration options are specified in the |RS| XML configuration file.

.. _section-how-to-load-plugin:

Load |DIA_Heading|
==========================

|DIA| must be registered as a |RS| adapter by using the ``<adapter_plugin>``
tag in the ``<plugin_library>`` section of the |RS| XML configuration.

Once the dynamic library and constructor function have been
registered, |RS| creates an instance of the adapter during startup.
You can use the adapter to create one or more connections, each of 
which stores information to a different database manager.

The following snippet demonstrates how to register the adapter:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="plugin_library">
                <adapter_plugin name="dia_adapter">
                        <dll>build/plugins/adapters/dia/rtiDatabaseIntegrationAdapter</dll>
                        <create_function>DiaAdapter_create_adapter_plugin</create_function>
                </adapter_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |DIA| dynamic library
             (|DIA_ADAPTER_LIB_NAME_LINUX| on Linux® systems). Make
             sure to include the library's directory in the library search
             path environment variable for your system
             (``LD_LIBRARY_PATH`` on Linux systems).

.. _section-register-dis-datatypes:

Register |DIA_Heading| Connections
==================================

Create a |DIA| connection for each target database.  

.. _section-configure-dis-connection-properties:

Configure connection properties
-------------------------------

Each ``<connection>`` is configured using 
the ``<property>`` tag at the connection level. When establishing the 
connection, you must provide a Data Source Name (DSN) that is already 
registered in your ``odbc.ini`` file. You must also provide the SQL
dialect using ``query_converter``. Optionally, you can specify a JSON file
containing *Topic*-level properties.

.. list-table:: :litrep:`<connection>` Properties
    :widths: 20 10 20 20 30
    :header-rows: 1

    * - Property
      - Required
      - Default
      - Type
      - Description
    * - ``DSN``
      - Yes
      - None
      - string
      - Data Source Name registered in the ODBC configuration file (odbc.ini).
        Must correspond to a valid database connection configuration.
    * - ``query_converter``
      - Yes
      - None
      - string
      - Database model. Current options are: ``mariadb``, ``postgresql``.
    * - ``topic_properties``
      - No
      - None
      - string
      - Path to a JSON configuration file containing *Topic*-level properties.
        Properties specified in this file will be applied to all *Topics*/streams
        in this connection. See
        :ref:`section-topic-properties-json` for the JSON file format.

Connection configuration examples
---------------------------------

Suppose the corresponding MariaDB ``odbc.ini`` file is:

.. code-block::

        [mariadb]
        DRIVER       = MariaDB ODBC 3.1 Driver
        SERVER       = 172.20.0.3
        DATABASE     = mariadb_test_database
        USER         = root
        PASSWORD     = mypassword

The correct |DIA| configuration would be:

.. code-block:: xml

        <connection name="mariadb_dia_connection" plugin_name="plugin_library::dia_adapter">
                <property>
                        <value>
                                <element>
                                        <name>DSN</name>
                                        <value>mariadb</value>
                                </element>
                                <element>
                                        <name>query_converter</name>
                                        <value>mariadb</value>
                                </element>
                                <element>
                                        <name>topic_properties</name>
                                        <value>plugins/adapters/dia/resources/json/topic_properties.json</value>
                                </element>
                        </value>
                </property>
        </connection>

.. _section-topic-properties:

Topic properties
----------------

The ``topic_properties`` connection parameter points to a JSON file that
defines *Topic*-level behavior. Each entry applies defaults to matching
*Topics*, so you can configure common behavior once instead of
repeating it in each ``<output>``.

The following properties are supported for each _Topic_ entry:

.. list-table:: ``topic_properties`` JSON entry properties
    :widths: 20 10 20 20 30
    :header-rows: 1

    * - Property
      - Required
      - Default
      - Type
      - Description
    * - ``topic_name_expression``
      - Yes
      - None
      - string
      - Topic name pattern used to match streams.
    * - ``delete_on_dispose``
      - Yes
      - None
      - boolean
      - When a *DataWriter* disposes an instance, all rows associated with
        that instance are deleted from the database.
    * - ``drop_table_on_dispose``
      - Yes
      - None
      - boolean
      - Removes the sample data table when the service shuts down.
    * - ``delete_type_on_dispose``
      - Yes
      - None
      - boolean
      - Removes the type from the types table when the service shuts down.
    * - ``format``
      - Yes
      - None
      - string
      - Format used to store sample data. Current options are: ``JSON``.
    * - ``instance_history``
      - No
      - 0
      - integer
      - Number of samples per instance to maintain in the database. When set
        to ``0``, only the latest sample per instance is kept. When set to a
        value greater than ``0``, up to that many samples per instance are
        stored, with the oldest being updated when the limit is reached. When
        set to ``-1``, all samples per instance are stored indefinitely.

.. _section-topic-properties-json:

Topic properties JSON file format
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``topic_properties`` parameter allows you to specify a JSON file containing
configuration properties that will be applied to *Topics* based on pattern
matching.

The JSON file should contain a ``topic_properties`` array where each element
specifies a *Topic* name pattern and its associated properties:

.. code-block:: json

    {
        "topic_properties": [
            {
                "topic_name_expression": "*",
                "delete_on_dispose": true,
                "drop_table_on_dispose": true,
                "delete_type_on_dispose": false,
                "instance_history": 0,
                "format": "JSON"
            }
        ]
    }


- The ``topic_name_expression`` field supports wildcard patterns:

  - ``*`` matches all *Topics*
  - ``prefix/*`` matches *Topics* starting with "prefix/"
  - ``*suffix`` matches *Topics* ending with "suffix"
  - Exact *Topic* names without wildcards match exactly

- When multiple patterns match a *Topic*, the first matching configuration
  in the array is used.


.. _section-register-dis-topics:

Register |DIA_Heading| Topics
=============================

The samples from the *Topics* to be recorded in the database are configured
inside a session by providing a filter inside a route configuration object.
For example, the following route configuration will record all *Topics* 
except those starting with ``rti/``:

.. code-block:: xml

        <session name="dis_session">
                <auto_route name="from_dds_to_mariadb">
                        <dds_input participant="dds_participant">
                                <allow_topic_name_filter>*</allow_topic_name_filter>
                                <allow_registered_type_name_filter>*</allow_registered_type_name_filter>
                                <deny_topic_name_filter>rti/*</deny_topic_name_filter>
                        </dds_input>
                        <output connection="mariadb_dia_connection">
                                <allow_stream_name_filter>*</allow_stream_name_filter>
                                <allow_registered_type_name_filter>*</allow_registered_type_name_filter>
                                <deny_stream_name_filter>rti/*</deny_stream_name_filter>
                        </output>
                </auto_route>
        </session>


.. _section-configure-dis-properties:

Configure Output Properties
===========================

Each ``<output>`` created by the |DIA| is associated with a
DDS domain, which can be configured using the ``<property>`` tag.
The ``dds.domain_id`` value must match the ``<domain_id>`` configured in
the participant used by that route.

.. list-table:: :litrep:`<output>` Properties
    :widths: 20 10 20 20 30
    :header-rows: 1

    * - Property
      - Required
      - Default
      - Type
      - Description
    * - ``dds.domain_id``
      - No
      - 0
      - integer
      - DDS Domain ID used by the adapter for this output. It must match
        the domain ID configured in the participant used by that route.

The following example is a complete ``<output>`` configuration including a route with
its filters. Note that *Topic*-specific properties like ``delete_on_dispose``,
``drop_table_on_dispose``, ``delete_type_on_dispose``, ``instance_history``,
and ``format`` are now configured in the JSON file specified by the
``topic_properties`` connection property.

.. code-block:: xml

        <session name="dis_session">
                <auto_route name="from_dds_to_mariadb">
                        <dds_input participant="dds_participant">
                                <allow_topic_name_filter>*</allow_topic_name_filter>
                                <allow_registered_type_name_filter>*</allow_registered_type_name_filter>
                                <deny_topic_name_filter>rti/*</deny_topic_name_filter>
                        </dds_input>
                        <output connection="mariadb_dia_connection">
                                <allow_stream_name_filter>*</allow_stream_name_filter>
                                <allow_registered_type_name_filter>*</allow_registered_type_name_filter>
                                <deny_stream_name_filter>rti/*</deny_stream_name_filter>
                                <property>
                                        <value>
                                                <element>
                                                        <name>dds.domain_id</name>
                                                        <value>0</value>
                                                </element>
                                        </value>
                                </property>
                        </output>
                </auto_route>
        </session>

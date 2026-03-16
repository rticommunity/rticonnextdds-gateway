.. include:: vars.rst

.. _section-tutorial:

********
Tutorial
********

This tutorial demonstrates how to use |DIA| to persist DDS samples from 
*RTI Shapes Demo* into a relational database. You will publish samples from
the *Shapes Demo* application and see how they are automatically stored in
either MariaDB or PostgreSQL databases.

Overview
========

In this tutorial, you will:

1. Start *Shapes Demo* and publish Square and Circle samples
2. Run |RS| with the |DIA| to store samples in a database
3. Query the database to verify the samples were stored correctly

The tutorial uses the ``ShapeType`` DDS type, which contains shape coordinates
(x, y), color (as a key field), and size information.

Before You Begin
================

Before starting this tutorial, ensure you have completed the installation and 
setup steps described in :ref:`section-installation-instructions`. Requirements
include:

* *RTI Connext DDS Professional* and *RTI Routing Service* are installed
* |DIA| is built and installed
* Database server (MariaDB or PostgreSQL) is running and accessible
* ODBC drivers are configured and data sources are set up
* *RTI Shapes Demo* is available

.. _section-tutorial-step1:

Step 1: Publish DDS Samples with Shapes Demo
=============================================

First, start *Shapes Demo* and begin publishing samples:

.. note::
    If you are using the provided Dockerfile, set the environment variable
    ``NDDS_DISCOVERY_PEERS=builtin.udpv4://172.20.0.2`` before running
    ``rtishapesdemo`` in order to communicate with the Docker network.

1. Launch *Shapes Demo*:

   .. code-block:: bash

       rtishapesdemo

2. Publish Square samples:

   - Click **Publish** in the *Shapes Demo* window
   - Select **Square** from the shape dropdown
   - Choose a color (e.g., **BLUE**)
   - Click **OK**

3. Publish Circle samples:

   - Click **Publish** again
   - Select **Circle** from the shape dropdown
   - Choose a different color (e.g., **RED**)
   - Click **OK**

You should now see the shapes moving around the window. These movements
represent DDS samples being published to the ``Square`` and ``Circle`` 
*Topics*.

.. _section-tutorial-step2:

Step 2: Run Routing Service with |DIA_HEADING|
==============================================

Now you will start |RS| with the |DIA| to capture and store
the DDS samples in your database.

Configure Routing Service
-------------------------

|DIA| includes a pre-configured |RS| configuration file at 
``plugins/adapters/dia/resources/xml/dia_routing_configuration.xml``.

The configuration file uses wildcard filters (``*``) to automatically discover 
and route any *Topic* published in DDS domain 0, making it ideal for the 
*Shapes Demo*. The configuration includes:

* The |DIA| registration
* Two pre-configured database connections:
  
  - ``mariadb_dia_connection`` - For MariaDB databases
  - ``postgres_dia_connection`` - For PostgreSQL databases

* Auto-routes that capture all DDS *Topics* (except RTI internal *Topics*) and 
  store them in the configured databases

The following snippet shows the key route-level configuration elements for the
MariaDB output:

.. code-block:: xml

    <auto_route name="from_dds_to_mariadb">
        <dds_input participant="dds_participant">
            <allow_topic_name_filter>*</allow_topic_name_filter>
            <allow_registered_type_name_filter>*</allow_registered_type_name_filter>
            <deny_topic_name_filter>rti/*</deny_topic_name_filter>
        </dds_input>
        <output connection="mariadb_dia_connection">
            <property>
                <value>
                    <element>
                        <name>dds.domain_id</name>
                        <value>0</value>
                    </element>
                </value>
            </property>
            <allow_stream_name_filter>*</allow_stream_name_filter>
            <allow_registered_type_name_filter>*</allow_registered_type_name_filter>
            <deny_stream_name_filter>rti/*</deny_stream_name_filter>
        </output>
    </auto_route>

The PostgreSQL route is similar but with different properties. 

.. note::
    The MariaDB route has ``instance_history`` set to ``3``, which means up to
    3 samples per instance (color) will be stored. The PostgreSQL route uses
    ``instance_history`` of ``0`` for UPSERT behavior (only latest sample).

Configure ODBC Data Source
--------------------------

Before running |RS|, you must configure your ODBC data sources. The configuration
file references DSN names ``mariadb`` for MariaDB and ``postgres`` for PostgreSQL.

For detailed instructions on configuring ODBC data sources, see 
:ref:`section-register-dis-datatypes`. You will need to create an ``odbc.ini`` 
file with the appropriate database connection details.

Run Routing Service
-------------------

Start |RS| with the provided configuration file. From the repository root, 
run:

.. code-block:: bash

    source /opt/rti.com/rti_connext_dds-<connext_version>/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.sh
    rtiroutingservice -cfgFile \
        plugins/adapters/dia/resources/xml/dia_routing_configuration.xml  \
        -cfgName bridge_DDS_DIA

You should see output indicating that |RS| has started successfully and
is connecting to the database. The adapter will automatically create the
necessary tables (``Square@0`` and ``Circle@0``) in your database.

.. _section-tutorial-step3:

Step 3: Query the Database
===========================

Now that samples are being stored in the database, you can query them to
verify the integration is working correctly.

#.  Connect to your MariaDB database using the ``mariadb`` ODBC DSN:

    .. code-block:: bash

        isql -v mariadb

#.  View the tables that |DIA| has created:

    .. code-block:: sql

        SHOW TABLES;

    You should see:

    * ``Square@0`` - Contains Square samples from domain 0
    * ``Circle@0`` - Contains Circle samples from domain 0
    * ``types`` - Contains type registration information

#.  Query the Square samples to extract the x and y coordinates of all 
    Square instances.

    MariaDB example query:

    .. code-block:: sql

        SELECT 
            JSON_UNQUOTE(JSON_EXTRACT(json_sample, '$.color')) AS color,
            JSON_EXTRACT(json_sample, '$.x') AS x,
            JSON_EXTRACT(json_sample, '$.y') AS y,
            FROM_UNIXTIME(sampleInfo_reception_timestamp_seconds) AS timestamp
        FROM `Square@0`
        ORDER BY timestamp DESC;

    MariaDB example output:

    .. code-block:: text

        +-------+-----+-----+---------------------+
        | color | x   | y   | timestamp           |
        +-------+-----+-----+---------------------+
        | BLUE  | 145 | 178 | 2026-01-12 10:30:45 |
        | BLUE  | 132 | 165 | 2026-01-12 10:30:44 |
        | BLUE  | 118 | 153 | 2026-01-12 10:30:43 |
        +-------+-----+-----+---------------------+

    .. note::

        Since the MariaDB route has ``instance_history`` set to ``3``, up to three
        samples per instance (color) are stored in the database. As the shape moves,
        newer samples are added until the limit is reached, at which point the oldest
        sample is updated with the new data.

    PostgreSQL example query:

    .. code-block:: sql

        SELECT 
            (json_sample::jsonb)->>'color' AS color,
            (json_sample::jsonb)->>'x' AS x,
            (json_sample::jsonb)->>'y' AS y,
            to_timestamp(sampleinfo_reception_timestamp_seconds) AS timestamp
        FROM "Square@0"
        ORDER BY sampleinfo_reception_timestamp_seconds DESC;

    PostgreSQL example output:

    .. code-block:: text

        +-------+-----+-----+---------------------+
        | color | x   | y   | timestamp           |
        +-------+-----+-----+---------------------+
        | BLUE  | 145 | 178 | 2026-01-12 10:30:45 |
        +-------+-----+-----+---------------------+

    .. note::
        The PostgreSQL route has ``instance_history`` set to ``0``, which means
        only the latest sample per instance (color) is stored using UPSERT behavior.
        Each new sample for the same color replaces the previous one.

#.  Check the Types Table to view the registered types:

    .. code-block:: sql

        SELECT * FROM types;

    Example output:

    .. code-block:: text

        +------------+-----------+---------------------+
        | topic_name | type      | type_representation |
        +------------+-----------+---------------------+
        | Square     | ShapeType | ...                 |
        | Circle     | ShapeType | ...                 |
        +------------+-----------+---------------------+

Next Steps
==========

Now that you have completed the basic tutorial, you can explore more advanced
features:

* **Add More Shapes**: Publish Triangle or other shapes from *Shapes Demo*
  and add corresponding routes in the configuration

* **Filter by Color**: Use DDS content filters to store only specific colors
  in the database

* **Monitor in Real-time**: Create a database view or use periodic queries
  to monitor shape positions as they update

* **Integrate with Applications**: Use the stored data in web applications,
  analytics tools, or reporting systems

For more information on configuration options, see :ref:`section-configuration`.
For database-specific features, see :ref:`section-database-support`.
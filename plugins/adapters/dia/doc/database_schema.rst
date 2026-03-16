.. include:: vars.rst

.. _section-database-schema:

***************
Database Schema
***************

This chapter describes the database tables and schema 
used for data storage and management.

|DIA| automatically creates two database tables:

* :ref:`section-database-table-structure` store DDS samples and associated 
  metadata for each registered *Topic*. 
* The :ref:`section-types-table-structure` stores type information for 
  registered DDS types.

.. _section-database-table-structure:

Data Tables
===========

For each DDS *Topic*, |DIA| creates a data table with the structure detailed
in the table below:

.. list-table:: Data Table Structure
    :widths: 30 20 50
    :header-rows: 1

    * - Column Name
      - Type
      - Description
    * - ``RTI_HISTORY_ID``
      - INT/SERIAL (auto-increment)
      - Unique identifier for each sample. This column is created only when
        ``instance_history > 1``.
        For more information, see :ref:`section-primary-key-configuration`.
    * - ``RTI_HISTORY_ORDER``
      - INT
      - Per-instance ordering field used to identify the oldest stored
        sample when ``instance_history > 1``.
        This column is only created when ``instance_history > 1``.
    * - ``sampleInfo_reception_timestamp_seconds``
      - INT
      - Seconds portion of the sample reception timestamp.
    * - ``sampleInfo_reception_timestamp_nanoseconds``
      - INT
      - Nanoseconds portion of the sample reception timestamp.
    * - ``sampleInfo_instance_handle``
      - BINARY(16) / BYTEA
      - DDS instance handle. Always part of the primary key.
    * - ``json_sample``
      - VARCHAR
      - Contains the serialized JSON representation of the DDS sample data.

.. _section-primary-key-configuration:

Primary key configuration
-------------------------

The primary key structure depends on the ``instance_history`` option:

* **instance_history = 0 or 1**: Primary key is ``(sampleInfo_instance_handle)``
  
  - Only the latest sample per instance is stored
  - New samples with the same instance handle replace existing ones

* **instance_history > 1 or instance_history = -1** : Primary key is 
  ``(RTI_HISTORY_ID, sampleInfo_instance_handle)``
  
  - Multiple samples per instance can be stored
  - ``RTI_HISTORY_ID`` ensures uniqueness across all samples
  - ``RTI_HISTORY_ORDER`` is used to find the oldest sample for replacement
    when the per-instance limit is reached

Table naming convention
-----------------------

Data tables are named using the pattern ``<topic_name>@<domain_id>``.

For example, a *Topic* named ``ShapeType`` in domain ``0`` would create a table
named ``ShapeType@0``.

.. _section-types-table-structure:

Types Table
===========

|DIA| also creates a types table to track which DDS types have been
registered. This table maintains type information for data processing 
and analysis. The structure of the types table is as follows:

.. list-table:: Types Table Structure
    :widths: 30 20 50
    :header-rows: 1

    * - Column Name
      - Type
      - Description
    * - ``topic_name``
      - VARCHAR
      - Name of the DDS *Topic*.
    * - ``type``
      - VARCHAR
      - Name of the DDS type used by the *Topic*.
    * - ``type_representation``
      - VARCHAR or binary (database dependent)
      - Serialized type representation. Its storage type depends on the
        ``type_representation_format`` connection option:

        - ``XML``: stored as string data
        - ``XCDR``: stored as binary data



.. include:: vars.rst

.. _section-release-notes:

Release Notes
=============

.. note::

    This software is provided for experimental purposes to evaluate 
    potential new features and gather customer feedback. These features 
    are not officially supported and must not be used in production 
    environments.

    For more information about experimental features in *Connext*, see 
    the `Connext What's New <https://community.rti.com/static/documentation/connext-dds/current/doc/manuals/connext_dds_professional/whats_new/index.html>`__.

Supported Platforms
-------------------

|DIA| is expected to build and run on the platforms in
:numref:`TableSupportedPlatforms`.

.. list-table:: Supported Platforms
    :name: TableSupportedPlatforms
    :widths: 20 80
    :header-rows: 1

    * - Platform
      - Operating System

    * - Linux systems
      - All Linux platforms supported by |RTI_RS|. For details, see 
        `Linux Platforms <https://community.rti.com/static/documentation/connext-dds/current/doc/manuals/connext_dds_professional/platform_notes/platform_notes/Linux_Platforms.htm#Chapter_5_Linux_Platforms>`__
        in the *RTI Connext Core Libraries Platform Notes*. 


Tested Database Versions
-------------------------

|DIA| has been tested with the following database systems and ODBC drivers:

.. list-table:: Tested Databases
    :name: TableTestedDatabases
    :widths: 25 35 40
    :header-rows: 1

    * - Database
      - Version
      - ODBC Driver

    * - MariaDB
      - 10.5+
      - MariaDB ODBC 3.1 Driver

    * - PostgreSQL
      - 12.0+
      - PostgreSQL Unicode driver

.. note:: |DIA| uses the ODBC API and should work with other ODBC-compliant
          databases, but support for databases other than MariaDB and PostgreSQL
          requires custom implementation. See :ref:`section-database-support` for details.

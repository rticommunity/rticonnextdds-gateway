.. include:: vars.rst

.. _section-installation-instructions:

***********************
Installation and Setup
***********************

This chapter describes the requirements and installation procedures for the
|DIA|.

.. _section-requirements:

Before You Begin
================

|DIA| uses ODBC to interface with databases. It requires the same 
core components as |RTI_GATEWAY|, as well as additional dependencies 
for database connectivity.

The following components are needed to create a successful connection:

* An ODBC driver manager (unixODBC for Linux systems)

* Database-specific ODBC drivers

   * MariaDB: MariaDB Connector/ODBC 3.1 or later
   * PostgreSQL: PostgreSQL ODBC Driver (psqlODBC)

.. note::
   The Docker-based installation  
   comes pre-configured with all required dependencies.

.. _section-docker-installation:

Docker-Based Compilation
=========================

The easiest way to build |DIA| is using the provided Docker environment.
The container includes a fully pre-configured environment with all 
dependencies.

.. important::

   |DIA| is disabled by default. You must explicitly enable it by 
   setting the CMake variable ``RTIGATEWAY_ENABLE_DIA=ON``. See 
   `Compiling with CMake <https://community.rti.com/static/documentation/gateway/current/building.html#compiling-with-cmake>`__ for more information.

Docker environment components
-----------------------------

The provided Dockerfile (located in ``resource/docker/Dockerfile``) includes:

* |RTI_CONNEXT| |CONNEXT_VERSION|
* Build tools (make, cmake, git, gcc)
* ODBC driver manager (unixODBC)
* MariaDB 10.5+ server
* MariaDB Connector/ODBC 3.1
* PostgreSQL 13+ server
* PostgreSQL ODBC driver
* Fully pre-configured environment variables 

.. _section-build-and-run:

Building and running the environment
------------------------------------

The ``resource/docker/docker-compose.yml`` file defines a complete environment
that includes:

* The |DIA| container 
* MariaDB and PostgreSQL database servers

#. Build and start the environment using ``docker compose``.

   .. note::
      Important License Notice:  Use of RTI software is subject to the terms of the applicable RTI Software License Agreement, which can be reviewed here: `RTI License Terms`_ (Free Use Software License Agreement (#4046)). 
      By setting the RTI_LICENSE_AGREEMENT_ACCEPTED environment variable to accepted and providing your rti_license.dat file, you acknowledge that you have read, understood, and agree to be bound by these terms. 
      Replace <license_path> with the path to your RTI license file directory.

   .. code-block:: console

      bash plugins/adapters/dia/resources/scripts/dia_docker_compose.sh build-run --license-file <license_path>

   This command:

   * builds the |DIA| container image with all dependencies
   * starts a MariaDB database server on 172.20.0.3:3306
   * starts a PostgreSQL database server on 172.20.0.4:5432

#. Build the |RTI_GATEWAY| inside the container.    See `Building from Source <https://community.rti.com/static/documentation/gateway/current/building.html>`__ 
   in the |RTI_GATEWAY| documentation for complete build instructions. 
   (See also ``doc/building.rst``.)

#. Run |RS| directly from the container. For example:

   .. code-block:: console

      source /opt/rti.com/rti_connext_dds-<connext_version>/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.sh
      rtiroutingservice -cfgFile \
         plugins/adapters/dia/resources/xml/dia_routing_configuration.xml  \
         -cfgName bridge_DDS_DIA

The ODBC configuration provided in the |DIA| container connects 
to the MariaDB and PostgreSQL containers at 
172.20.0.3 and 172.20.0.4, respectively.

.. note::
   
      For a complete working example that demonstrates |DIA| with both
      MariaDB and PostgreSQL, see the :ref:`section-tutorial` chapter.

Stopping the environment
------------------------

To stop and remove all containers:

.. code-block:: console

   bash plugins/adapters/dia/resources/scripts/dia_docker_compose.sh remove

.. _section-local-installation:

Local Installation
==================

The Dockerfile (``resource/docker/Dockerfile``) is maintained as the single 
source of truth for all dependency requirements and installation commands. It 
contains the exact package names, versions, and configuration steps needed for 
a complete working environment.

To install |DIA| natively (without Docker), use the 
Dockerfile as the definitive reference for required packages and 
installation steps. 

Follow these general steps to install |DIA| locally (adjusting commands 
for your specific Linux distribution):

#. Install all dependencies listed in the Dockerfile.

   Refer to ``resource/docker/Dockerfile`` for the complete list of 
   required packages. The Dockerfile includes the following components:

   * Build tools (make, cmake, git)
   * ODBC driver manager (unixODBC)
   * Database servers and client libraries (MariaDB, PostgreSQL)
   * Database-specific ODBC drivers

   Use your distribution's package manager (``apt``, ``yum``, etc.) to install
   these packages.

#. Build the gateway.

   See `Building from Source <https://community.rti.com/static/documentation/gateway/current/building.html>`__
   in the |RTI_GATEWAY| *User's Manual* for complete build instructions. 
   (See also ``doc/building.rst``)

.. important::

   |DIA| is disabled by default. You must explicitly enable it by 
   setting the CMake variable ``RTIGATEWAY_ENABLE_DIA=ON``. See 
   `Compiling with CMake <https://community.rti.com/static/documentation/gateway/current/building.html#compiling-with-cmake>`__ for more information.

#. Configure ODBC data sources

   Create ``~/.odbc.ini`` (or ``/etc/odbc.ini`` for system-wide configurations)
   with your database connection details. See example configurations in
   ``plugins/adapters/dia/resources/odbc.ini``.

#. Set ODBC environment variables:

   .. code-block:: console

      export ODBCSYSINI=/etc
      export ODBCINI=$HOME/.odbc.ini

#. Test ODBC connectivity to verify the installation:

   .. code-block:: console

      odbcinst -q -d           # List installed ODBC drivers
      isql -v your_dsn_name    # Test database connection

#. Run |RS| with your |DIA| configuration:

   .. code-block:: console

      source /opt/rti.com/rti_connext_dds-<connext_version>/resource/scripts/rtisetenv_x64Linux4gcc7.3.0.sh
      rtiroutingservice \
         -cfgFile plugins/adapters/dia/resources/xml/dia_routing_configuration.xml \
         -cfgName bridge_DDS_DIA

   .. note::
      For a complete working example that demonstrates |DIA| with both
      MariaDB and PostgreSQL, see :ref:`section-tutorial`.

   ..

For in-depth solutions to common issues, see :ref:`section-troubleshooting`.

.. _RTI License Terms: https://www.rti.com/terms


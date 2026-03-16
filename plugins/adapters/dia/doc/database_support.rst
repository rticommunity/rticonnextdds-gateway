.. include:: vars.rst

.. _section-database-support:

****************
Database Support
****************

|DIA| provides out-of-the-box support for MariaDB and PostgreSQL databases. 
This chapter explains the requirements and process to add support for 
additional database systems.

.. _section-adding-database-support:

Add Support for Other Databases
===============================

The |DIA| uses Open Database Connectivity (ODBC), which is a
database-agnostic API. To add support for a database other than MariaDB 
and PostgreSQL, you must implement specific SQL syntax handlers. While 
ODBC provides a universal connection interface, custom handlers ensure 
compatibility with the varying SQL dialects inherent to different database 
engines.

To add support for a new database: 

1. Implement a custom ``QueryConverter`` class.
2. If advanced SQL features are needed, modify the ``Queries`` class. 
3. Register your custom ``QueryConverter`` in the factory and map it from the ``query_converter`` connection property.


Creating a custom QueryConverter class
--------------------------------------

The ``QueryConverter`` abstract class defines the interface for database-specific
SQL generation. You must implement all pure virtual methods:

**Header Location**: ``plugins/adapters/dia/srcCxx/QueryConverter.hpp``


Extending the Queries Class (optional)
--------------------------------------

The ``Queries`` class (located in ``plugins/adapters/dia/srcCxx/Queries.hpp`` and
``Queries.cxx``) uses your ``QueryConverter`` to build SQL statements. In most cases,
you won't need to modify this class.

However, if your database requires significantly different SQL syntax for advanced
features, you may need to extend or modify the ``Queries`` class methods.


Registering your QueryConverter
-------------------------------

Add your custom ``QueryConverter`` to the enum in
``plugins/adapters/dia/srcCxx/QueryConverter.hpp`` and to the factory in
``plugins/adapters/dia/srcCxx/QueryConverter.cxx``:

.. code-block:: cpp

   enum class QueryConverterFormat {
       mariadb,
       postgresql,
       oracle
   };

   std::unique_ptr<QueryConverter> QueryConverterFactory::create_query_converter(
           const QueryConverterFormat& query_converter) const
   {
       switch (query_converter) {
       case QueryConverterFormat::mariadb:
           return std::unique_ptr<QueryConverter>(new MariaDBQueryConverter());
       case QueryConverterFormat::postgresql:
           return std::unique_ptr<QueryConverter>(new PostgreQueryConverter());
       case QueryConverterFormat::oracle:
           return std::unique_ptr<QueryConverter>(new OracleQueryConverter());
       default:
           throw std::runtime_error("Incorrect database");
       }
   }

Then map the new string value from connection properties in
``plugins/adapters/dia/srcCxx/DiaConnection.cxx`` (for example,
``query_converter = oracle``).


Configuring Routing Service
---------------------------

Once implemented and registered, use your custom database by setting the
``query_converter`` connection property in your |RS| configuration:

.. code-block:: xml

   <connection name="my_custom_db_connection" plugin_name="plugin_library::dia_adapter">
       <property>
           <value>
               <element>
                   <name>query_converter</name>
                   <value>oracle</value>
               </element>
               <!-- other properties -->
           </value>
       </property>
   </connection>

Configuring the ODBC Data Source Name
-------------------------------------

Don't forget to configure your new database's ODBC Data Source Name (DSN)
in your ``odbc.ini`` file. Without this name, the ODBC driver manager will
not be able to locate your database connection.

Add an entry for your custom database:

.. code-block:: ini

   [oracle]
   DRIVER       = Oracle ODBC Driver
   SERVER       = localhost
   DATABASE     = your_database_name
   USER         = your_username
   PASSWORD     = your_password
   PORT         = 1521

The DSN name (e.g., ``[oracle]``) must match the ``DSN`` property in your |DIA| connection configuration:

.. code-block:: xml

   <connection name="my_custom_db_connection" plugin_name="plugin_library::dia_adapter">
       <property>
           <value>
               <element>
                   <name>DSN</name>
                   <value>oracle</value>  <!-- Must match odbc.ini section name -->
               </element>
           </value>
       </property>
   </connection>

See the ``odbc.ini`` file in ``plugins/adapters/dia/resources/odbc/`` for
complete configuration examples.

Common Considerations
=====================

Consider these common architectural variations when adding support for 
a new database:

Data types
----------

* Binary data: MariaDB uses ``BINARY(n)``, PostgreSQL uses ``BYTEA``, SQL Server uses ``VARBINARY``
* Strings: Different maximum lengths and VARCHAR behavior
* Integers: Signed vs unsigned, size limits

Identifier quoting
------------------

* MariaDB: backticks (`````)
* PostgreSQL: double quotes (``"``)
* SQL Server: square brackets (``[]``) or double quotes
* Oracle: double quotes

Auto-increment columns
----------------------

* MariaDB: ``AUTO_INCREMENT``
* PostgreSQL: ``SERIAL`` or ``IDENTITY``
* SQL Server: ``IDENTITY(1,1)``
* Oracle: ``GENERATED AS IDENTITY`` or sequences

UPSERT operations
-----------------

* MariaDB: ``REPLACE INTO`` or ``INSERT ... ON DUPLICATE KEY UPDATE``
* PostgreSQL: ``INSERT ... ON CONFLICT DO UPDATE``
* SQL Server: ``MERGE`` statement
* Oracle: ``MERGE`` statement

Transaction and locking
-----------------------

Different databases have varying default behaviors for:

* Transaction isolation levels
* Lock timeouts
* Concurrent writes

Ensure your ODBC driver and connection settings are appropriate for your use case.

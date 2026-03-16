# Database Integration Adapter

The Database Integration Adapter (DIA) enables RTI Routing Service to persist DDS samples into relational databases through ODBC.

It is designed for DDS-to-database integration workflows where applications need to store and query historical DDS data using SQL. 
DIA can automatically create  topic-based tables and store sample payloads with useful metadata.

## Experimental Status

Database Integration Adapter is currently **experimental** and intended for evaluation, prototyping, feedback and Not recommended for production use.

## Documentation

Refer to complete DIA documentation here: 
https://community.rti.com/static/documentation/gateway/current/adapters/dia/index.html 

Refer to Connext Gateway documentation here: 
https://community.rti.com/static/documentation/gateway/current/index.html 

You can find additional resources  for RTI Connext and all other RTI products on the following sites:
* [RTI Website](https://www.rti.com/en/)
* [RTI Community](https://community.rti.com/)


## Dependencies, Platform Support and Versioning

DIA requires the same core components as [RTI Connext Gateway](https://community.rti.com/static/documentation/gateway/current/installation.html) and additional
database connectivity components:

* ODBC driver manager (unixODBC on Linux)
* Database-specific ODBC drivers
    * MariaDB Connector/ODBC 3.1 or later
    * PostgreSQL ODBC driver (psqlODBC)

Out-of-the-box database support and tested configurations include:

* MariaDB 10.5+ with MariaDB ODBC 3.1 driver
* PostgreSQL 12.0+ with PostgreSQL Unicode driver

DIA is expected to build and run on Linux platforms supported by RTI Routing Service. For full platform details, refer to [RTI Routing Service
Release Notes](https://community.rti.com/static/documentation/connext-dds/7.3.1/doc/manuals/connext_dds_professional/services/routing_service/release_notes/release_main.html) in the Connext documentation.

## Feedback and Support

**Technical Issues:** For bugs, build problems, or API-related issues, please open an issue in this GitHub repository. Providing clear reproduction steps and environment details helps us respond more effectively.

**Product Feedback and General Questions**

For non-technical questions, usability feedback, or input on future direction, please email:product-feedback@rti.com Your feedback helps shape the evolution of Database integration support in RTI products.
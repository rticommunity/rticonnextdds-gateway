.. include:: vars.rst

.. _section-introduction:

Introduction
============

|RSKAFKA_TM| is a plugin for |RTI_RS| that enables communication between
*Kafka* and *DDS* applications.

Using |RSKAFKA|, |RS| can act as a |KAFKA_CLIENT|, connect to a
|KAFKA_BROKER|. Then, it can publish |KAFKA_MESSAGEs|, and subscribe to
|KAFKA_TOPICs| published by other |KAFKA_APPs|.

Users can configure |RS| to route data between multiple |DDS_DOMAINS|
and |KAFKA_BROKERs|, enabling all *DomainParticipants* in a DDS Domain to
exchange data with all |KAFKA_CLIENTs| connected to a |KAFKA_BROKER|.

|RSKAFKA| uses |RS|’s Transformation plugins to convert DDS data
to a |KAFKA_MESSAGE| in a format that a Kafka application can understand
(e.g., JSON) and transform a |KAFKA_MESSAGE| to DDS data as well.

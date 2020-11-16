.. include:: vars.rst

.. _section-introduction:

Introduction
============

|RSMODBUS_TM| is a plugin for |RTI_RS| that enables communication between
*Modbus* and *DDS* applications.

Using |RSMODBUS|, |RS| can act as a |MODBUS_CLIENT|, connect to a
|MODBUS_SERVER|, send requests to write or read holding
registers and coils, or read input registers or discrete inputs.

You can configure |RS| to route data between multiple |DDS_DOMAINS| and
|MODBUS_DEVICEs|.

.. _section-adapter-registers-addresses:

This document assumes you have a basic understanding of the Modbus protocol,
DDS and Routing Service, and their terminology.

Modbus Registers and Addresses
------------------------------

|RSMODBUS| diferentiates between a "register address" and a "register number".
Register addresses use a 0-based index; register numbers use a 1-based index.

Throughout this documentation, only "register address" is used. Therefore,
code snippets and examples will consider the register 0 as a valid register.

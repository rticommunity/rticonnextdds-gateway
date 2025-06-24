.. include:: vars.rst

.. _section-introduction:

Introduction
============

|DATADIODE_NAME| is a plugin for |RTI_RS| that enables communication through a data diode.

To achieve this, |DATADIODE_NAME| serializes DDS data and sends it through a UDP socket.
At the other end, another instance of |DATADIODE_NAME| deserializes the data and
publishes it to a DDS domain.

.. _section-udp-socket:

UDP sockets
------------------------------

|DATADIODE_NAME| requires some socket configuration that is accesible through the |RS| file.




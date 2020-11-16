.. include:: vars.rst

.. _section-install:

************
Installation
************

|RTI_CONNEXT_GATEWAYS| is distributed in source format. You can clone the source repository
using ``git``:

.. code-block:: sh

    git clone --recurse-submodule https://github.com/rticommunity/rticonnextdds-gateways.git


The ``--recurse-submodule`` option is required to clone additional
third-party repositories as submodules.

External Dependencies
=====================

Source Build
------------
The repository requires ``cmake`` to be built from source, plus a toolchain
supported by one of ``cmake``'s generators.

External Librarires
-------------------

|RSMODBUS|
~~~~~~~~~~

:libmodbus: :link_libmodbus:`Libmodus <>` is used as implementation of the
            client and server of the Modbus protocol.

|RSMQTT|
~~~~~~~~

:paho.mqtt.c: The :link_paho_c:`Paho C Client <>` library is used as
              implementation of the client side of the MQTT protocol.

Documentation
-------------
The repository requires ``doxygen`` and ``sphinx``.
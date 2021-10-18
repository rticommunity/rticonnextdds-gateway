.. include:: vars.rst

.. _section-install:

************
Installation
************

|RTI_GATEWAY| is distributed in source format. You can clone the source repository
using ``git``:

.. code-block:: sh

    git clone --recurse-submodule https://github.com/rticommunity/rticonnextdds-gateway.git


The ``--recurse-submodule`` option is required to clone additional
third-party repositories as submodules.

If you forget to clone the repository with ``--recurse-submodule``, simply run the
following command to pull all the dependencies:

.. code-block:: sh

    git submodule update --init --recursive

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

:libmodbus: :link_libmodbus:`Libmodbus <>` is used as implementation of the
            client and server of the MODBUS protocol.
:License: `LGPL v2.1 <https://github.com/stephane/libmodbus/blob/master/COPYING.LESSER>`__

|RSMQTT|
~~~~~~~~

:paho.mqtt.c: The :link_paho_c:`Paho C Client <>` library is used as
              implementation of the client side of the MQTT protocol.
:License: `Eclipse Public License - v 2.0 <https://github.com/eclipse/paho.mqtt.c/blob/master/LICENSE>`__

|RSKAFKA|
~~~~~~~~

:librdkafka: The :link_librdkafka:`Librdkafka <>` library is used as
              implementation of the client side of the Kafka protocol.
:License: `2-clause BSD license <https://github.com/edenhill/librdkafka/blob/master/LICENSE>`__


Other Dependencies
------------------

:json_parser: :link_json_parser:`json_parser <>` is used in order to parse json
              files.
:License: `BSD with no dependencies <https://github.com/udp/json-parser/blob/master/LICENSE>`__

Documentation
-------------
The repository requires ``doxygen`` and ``sphinx``. You may also need to
manually install: ``graphviz``, ``docutils 0.14+``, and ``sphinx_rtd_theme``.
See the `README.md <https://github.com/rticommunity/rticonnextdds-gateway>`__
file for further information.

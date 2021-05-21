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

|RSMQTT|
~~~~~~~~

:paho.mqtt.c: The :link_paho_c:`Paho C Client <>` library is used as
              implementation of the client side of the MQTT protocol.

Documentation
-------------
The repository requires ``doxygen`` and ``sphinx``. You may also need to
manually install: ``graphviz``, ``docutils 0.14+``, and ``sphinx_rtd_theme``.
See the `README.md <https://github.com/rticommunity/rticonnextdds-gateway>`__
file for further information.

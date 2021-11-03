.. include:: vars.rst

.. _section-build:

********************
Building from source
********************

Since |RTI_GATEWAY| is distributed in source format, it must
be compiled into a shared library before it can be used by |RS|.

The code can be built using |CMAKE|, version 3.10.0 or greater.

.. _section-build-env:

Build Setup
===========

|RTI_CONNEXT| (version |CONNEXT_VERSION_601| or greater) must be available
on the build system. The installation must include libraries for the
target build architecture, and ``rtiddsgen``.

.. note:: The location where |RTI_CONNEXT| is installed will be referred to
          as ``NDDSHOME``, while the target build architecture will
          be ``CONNEXTDDS_ARCH``.

Compiling with |CMAKE|
======================

Configuration
-------------

Create a build directory and run ``cmake`` to create build files for the
build system of your preference.

At a minimum, you will need to specify:

  - ``CONNEXTDDS_DIR`` or ``NDDSHOME`` environment variable to select an
    |RTI_CONNEXT| installation.

Optionally, you can use ``CMAKE_BUILD_TYPE`` to select the type of libraries to
build (for example ``Release``, or ``Debug``, see CMake's own documentation for
`CMAKE_BUILD_TYPE <https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html>`_).
This documentation and README files assume that you are using the ``Release``
build configuration. If you build the libraries using ``Debug``
configuration, you must run the debug version of Routing Service located at:

.. code-block:: sh

    $NDDSHOME/resource/app/bin/<architecture>/rtiroutingserviceappd


It is also strongly recommended (albeit not required for building) that you
also change the value of variable ``CMAKE_PREFIX_INSTALL``, to specify a custom
installation directory where all build artifacts will be copied after building.
This directory can be copied, and deployed independently of the source and build
directories.

.. code-block:: sh

    # The build/ and install/ directories can be created inside the git clone
    # (they are ignored via .gitignore)
    mkdir rticonnextdds-gateways/build \
          rticonnextdds-gateways/install

    cd rticonnextdds-gateways/build/

    # CONNEXTDDS_DIR and CONNEXTDDS_ARCH will be read from the
    # shell environment if not passed explicitly to cmake
    cmake .. -DCONNEXTDDS_DIR=/path/to/rti_connext_dds/ \
             -DCMAKE_BUILD_TYPE=Debug|Release           \
             -DCMAKE_INSTALL_PREFIX=../install

Compilation
-----------

Once the project has been configured, libraries can be built using the selected
toolchain, e.g.:

Linux® and macOS® systems:

.. code-block:: sh

    cmake --build .

Windows® systems:

.. code-block:: bat

    cmake --build . --config Debug|Release

Installation
------------

It is recommended that you also run the ``install`` step, e.g.:

Linux and macOS systems:

.. code-block:: sh

    cmake --build . -- install

Windows systems:

.. code-block:: bat

    cmake --build . --config Debug|Release --target install

CMake General Options
---------------------

This section summarizes all available |CMAKE| general options and configuration
variables. Specific adapter options are explained in their corresponding
sections.

CONNEXTDDS_DIR
^^^^^^^^^^^^^^

:Required: Only if ``NDDSHOME`` environment variable is not set.
:Default: None
:Description: The installation location of |RTI_CONNEXT| must be specified
              using this variable or ``NDDSHOME`` environment variable. The
              latter will take precedence.

CONNEXTDDS_ARCH
^^^^^^^^^^^^^^^

:Required: No
:Default: None, if possible, it is taken from ``FindRTIConnextDDS``.
:Description: The identifier for the target build architecture must be
              specified using this variables. Required |RTI_CONNEXT| libraries
              must be available within path
              ``NDDSHOME/lib/CONNEXTDDS_ARCH``.

RTIGATEWAY_ENABLE_ALL
^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If enabled, all components will be enabled unless they are
              disabled specifically. This includes tests, examples, docs,
              plugins...

RTIGATEWAY_ENABLE_TESTS
^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If enabled, all tests of enabled plugins will be built.

RTIGATEWAY_ENABLE_EXAMPLES
^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If this variable is enabled, all the examples of enabled plugins
              will be built.


RTIGATEWAY_ENABLE_DOCS
^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If enabled, the |RTI_GATEWAY| user manual (this document)
              will be built, along with specific documentation for every enabled
              plugin. Make sure to have Sphinx, breathe and doxygen installed.

Plugins CMake Specific Options
------------------------------

RTIGATEWAY_ENABLE_MODBUS
^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If this variable is enabled, the Modbus adapter and all its
              components are enabled unless they are specifically disabled
              (tests, examples, docs...).

RTIGATEWAY_ENABLE_MQTT
^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If this variable is enabled, the MQTT adapter and all its
              components are enabled unless they are specifically disabled
              (tests, examples, docs...).

RTIGATEWAY_ENABLE_KAFKA
^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If this variable is enabled, the Kafka adapter and all its
              components are enabled unless they are specifically disabled
              (tests, examples, docs...).

RTIGATEWAY_ENABLE_FWD
^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If this variable is enabled, the forwarder processor and all its
              components are enabled unless they are specifically disabled
              (tests, examples, docs...).

RTIGATEWAY_ENABLE_TSFM_FIELD
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If this variable is enabled, the field transformation and all its
              components are enabled unless they are specifically disabled
              (tests, examples, docs...).

RTIGATEWAY_ENABLE_TSFM_JSON
^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``ON``
:Description: If this variable is enabled, the JSON transformation and all its
              components are enabled unless they are specifically disabled
              (tests, examples, docs...).

RTIGATEWAY_ENABLE_LOG
^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``OFF`` (for ``Release``), ``ON`` (for ``Debug``)
:Description: By default, plugins will not print any messages to standard
              output when built in ``Release`` mode. When built in ``Debug``
              mode, or if ``RTIGATEWAY_ENABLE_LOG`` is enabled, plugins will
              print informational and error messages to standard output. These
              messages cannot be disabled at run-time.
:Note: This doesn't apply to the Modbus Adapter.

RTIGATEWAY_ENABLE_TRACE
^^^^^^^^^^^^^^^^^^^^^^^

:Required: No
:Default: ``OFF``
:Description: If enabled, this option will cause plugins to produce a much
              more verbose logging output, which can be used to trace all
              function calls within the adapter code.
:Note: This doesn't apply to the Modbus Adapter.

MQTT Adapter CMake Options
--------------------------

RTIGATEWAY_ENABLE_SSL
^^^^^^^^^^^^^^^^^^^^^

|OPENSSL| must be available on the system, if support for SSL/TLS is required.

.. note:: In case that this fails or do not find the correct version, you can
          provide |CMAKE| with the correct path by setting the variable
          ``-DOPENSSL_ROOT_DIR=<path_to_openssl>``. This directory must contain
          header files in an ``include/`` subdirectory, and libraries in a
          ``lib/`` subdirectory. In Windows, there will be dll files in
          the ``bin/`` subdirectory as well.

:Required: No
:Default: ``OFF``
:Description: When this option is enabled, SSL/TLS support will be compiled in
              |RSMQTT|, and |PAHO_ASYNC|. ``OPENSSLHOME`` must also be
              specified to provide the required |OPENSSL| dependencies. You can
              specify ``OPENSSLHOME`` when calling |CMAKE| by adding
              ``-DOPENSSLHOME=/path/to/openssl``.


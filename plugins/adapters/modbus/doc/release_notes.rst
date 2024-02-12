.. include:: vars.rst

.. _section-release-notes:

Release Notes
=============

Bug fixes (January 2023)
--------------------------

#. Unexpected behavior when reading DDS negative values in some platforms.
   This issue has been fixed.

New features (September 2021)
-----------------------------

#. Support for Modbus RTU slaves devices. These devices are identified by the
   JSON configuration attribute |CONF_MODBUS_SLAVE_DEVICE_ID|.
#. Support for new constant datatypes. Primitive types, enums and, arrays of
   the previous types are now allowed values for the JSON configuration
   attribute |CONF_VALUE|.

Bug fixes (September 2021)
--------------------------

#. Unexpected behavior when using negative values in the JSON configuration
   attributes |CONF_MODBUS_MIN_VALUE| and |CONF_MODBUS_MAX_VALUE| if the DDS
   field was unsigned. Now an error is thrown when this happens.

Supported Platforms
-------------------

|RTI_RSMODBUS| is expected to build and run on the platforms in
:numref:`TableSupportedPlatforms`.

.. list-table:: Supported Platforms
    :name: TableSupportedPlatforms
    :widths: 20 80
    :header-rows: 1

    * - Platform
      - Operating System

    * - Linux systems
      - All Linux platforms supported by |RTI_RS|.
    * - macOS systems
      - All macOS platforms supported by |RTI_RS|.
    * - Windows systems
      - All Windows platforms supported by |RTI_RS|.

.. include:: vars.rst

.. _section-additional_resources:

********************
Additional Resources
********************

This section contains a list of additional resources for |RSMODBUS|.
The following resources are not detailed in this documentation because
they contain their own README file.

Examples
^^^^^^^^

Examples are located under ``<RTI Gateway main folder>/examples/modbus``.
If you have provided CMake with an installation directory, they will be under
``<CMake Installation directory>/examples/modbus``.


Tests
^^^^^

Tests are located under ``<RTI Gateway main folder>/plugins/adapters/modbus/test``
If you have provided CMake with an installation directory, they will be under
``<CMake Installation directory>/test/modbus``.


Utilities
^^^^^^^^^

Utilities are located under ``<RTI Gateway main folder>/plugins/adapters/modbus/utilities``
If you have provided CMake with an installation directory, they may be under
different categories, for example binaries will be located under
``<CMake Installation directory>/bin``.

Utilities may contain different stuff that are used by other artifacts or users
directly, for example a modbus server, modbus client, etc... They may not
contain a README file if they are used and mentioned in other artifacts's
README files.

Resources
^^^^^^^^^

Resources are located under ``<RTI Gateway main folder>/resource/modbus``
If you have provided CMake with an installation directory, they will be under
``<CMake Installation directory>/resource/modbus``.
Resources may include a different set of useful things like JSON schemas,
and more.

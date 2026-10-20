.. include:: vars.rst

.. _section-release-notes:

Release Notes
=============

Pending Changes
---------------

#. Added the per-output ``payload_format`` property. The default ``text`` mode
   removes one trailing NUL byte. Set ``payload_format`` to ``binary`` for
   Protobuf and other binary payloads to preserve every byte, including zeros.
#. Fixed binary payload truncation, empty-value handling, partial write error
   reporting, and reader resource cleanup and notification ordering.
#. Replaced the reader's shared stop flag with a semaphore signal. A failed
   stop signal, wakeup or worker join reports an error and retains reader
   resources, avoiding cleanup while the worker may still access them. A
   successful deletion retry releases the retained resources.

New Features (May 2024)
-----------------------

#. Updated librdkafka to version 2.10.0

Supported Platforms
-------------------

|RTI_RSKAFKA| is expected to build and run on the platforms in
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

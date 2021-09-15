.. include:: vars.rst

.. _section-release-notes:

Release Notes
=============

New features (July 2021)
------------------------

#. Introduced support for a wider range of input/output members when converting
   between JSON and DynamicData representations.
#. The type of the member specified by property |PROP_BUFFER_MEMBER| can
   now be any of the following "string-like" types:

   * ``string``
   * ``sequence<octet>``
   * ``sequence<char>``
   * ``octet[N]``
   * ``char[N]``

#. Improved resource management. This implies the deletion of the following
   properties:

   * *serialized_size_max*
   * *serialized_size_incr*

   Deprecated:

   * *serialized_size_min*

   And added a new property:

   * |PROP_UNBOUNDED_MEMBER_SERIALIZED_SIZE_INITIAL|

   This new property represents the initial allocated size when using unbounded
   sequences and strings.

Supported Platforms
--------------------

|RTI_RS_JSON_TSFM| is expected to build and run on the platforms in
:numref:`TableSupportedPlatforms`.

It can also be deployed as a C library linked into your application.

This is true for all platforms in :numref:`TableSupportedPlatforms`.

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

.. include:: vars.rst

.. _section-release-notes:

Release Notes
=============

New features
------------

#. Added support for all DynamicData datatypes when creating a JSON
   representation of the DynamicData (aka deserialize).
#. The datatype pointed by the property |PROP_BUFFER_MEMBER| supports the
   following string-like datatypes:

   * Strings
   * Sequences of DDS_Octets (``DDS_OctetSeq``)
   * Sequences of DDS_Chars (``DDS_CharSeq``)
   * Arrays of ``DDS_Octet``
   * Arrays of ``DDS_Char``

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

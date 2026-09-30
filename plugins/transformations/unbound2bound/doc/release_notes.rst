.. include:: vars.rst

.. _section-release-notes:

Release Notes
=============

Initial Unbounded2Bounded implementation
--------------------------------------------------

* Strict one-sequence input validation and same-kind bounded output validation.
* One-to-many chunking, including empty-input completion markers.
* Exception-safe sample/info ownership and explicit lifecycle filtering.
* XML-defined types, forward routes and burst-capable test QoS.
* Direct numeric/schema tests and real DDS integration/rejection tests.

Migration
---------

This plugin replaces the copied Sequence2Array implementation in this directory.
The old Sequence2Array and Array2Sequence factory symbols are not exported here.
Use the new library/factory and bounded sequence plus ``more_data`` schema.
The separate original plugin is unchanged and may be built alongside this one.

Validation scope
----------------

Local validation uses Connext 7.7.0, Linux x64 and GNU C++ 13. The repository's
Connext 7.3 minimum is unchanged. Other SDKs, Windows and macOS require their
own build and runtime validation; no cross-platform certification is implied.
IDL long double preserves platform-specific SDK data rather than guaranteeing
native IEEE binary128 arithmetic.

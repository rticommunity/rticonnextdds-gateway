.. include:: vars.rst

.. _section-introduction:

************
Introduction
************

|RS_UNBOUNDED2BOUNDED_TSFM| splits each input's unbounded numeric sequence into
distinct bounded-sequence samples, preserving numeric type, values and order.
Conversion is forward-only.

The input structure must contain exactly one required, non-key unbounded
sequence. The output must contain exactly two required, non-key fields: a
bounded sequence with the same member name and numeric element kind, and a
boolean named ``more_data``. Type aliases and reordered output fields are
accepted. Inheritance, extra fields, arrays, unions, nested collections,
characters, enums, strings and boolean sequence elements are rejected.

Signed/unsigned 16-, 32- and 64-bit integers, octet, float, double and IDL long
double are supported. Explicit int8/uint8 support follows the repository's SDK
capability check. Long double is copied using ``rti::core::LongDouble`` scalar
access, preserving the SDK representation without arithmetic conversion.

The output sequence bound is the chunk size. Each chunk except the final one
has ``more_data=true``. An empty input produces one empty sample with false.
Exact multiples of the bound have no extra terminator. Inputs are never merged
and incomplete chunks are not retained across callbacks.

For a bound of 1000, an input of length 2001 produces lengths 1000, 1000 and 1,
with flags true, true and false. The last flag is false even if another input
sample is already queued.

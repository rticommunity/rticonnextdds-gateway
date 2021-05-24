.. include:: vars.rst

.. _section-introduction:

************
Introduction
************

|RS_JSON_TSFM_TM| provides a way of transforming DDS types into a JSON string
(using standard DDS-JSON mapping), and store the result in an octet
sequence field of the output.

It also allows to parse the contents of an octet sequence field in
the input sample as a JSON object, and set the values of the corresponding
fields in the output sample (only "flat" types, with only fields of a
primitive type).

.. include:: vars.rst

.. _section-introduction:

************
Introduction
************

|RS_JSON_TSFM_TM| provides a way of transforming DDS types into a JSON string
(using standard DDS-JSON mapping), and store the result in a member capable
of storing a string-like value, e.g. a string or a sequence of characters (see section
:ref:`section-how-to-load-plugin` for a complete list of supported data types).

The plugin also supports parsing the contents of a member in the input sample
as a JSON object, and setting the resulting values in the corresponding fields of
the output sample.

Similarly to the conversion from DDS to JSON, the input member must be of a
"string-like" type, e.g. a string or a sequence of characters.
fields in the output sample.

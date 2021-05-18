.. include:: vars.rst

.. _section-introduction:

************
Introduction
************

|RS_JSON_TSFM_TM| provides a way of transforming DDS types into a JSON string
(using standard DDS-JSON mapping), and store the result in a member capable
of storing a string-like value, e.g. a string or a sequence of characters (see section
:ref:`section-how-to-load-plugin` for a complete list of supported data types).

It also allows to parse the contents of the supported datatypes fields in
the input sample as a JSON object, and set the values of the corresponding
fields in the output sample.

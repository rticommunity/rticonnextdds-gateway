.. include:: vars.rst

.. _section-introduction:

************
Introduction
************

|RS_SEQ2ARRAY_TSFM_TM| provides a way of transforming DDS samples that contain
sequences into DDS samples that uses arrays.

The plugin supports any input data type, and it checks that the output datatype
is compatible (just replacing sequences by arrays). It also checks that the size
of the corresponding sequences fit into the array, otherwise it will throw an
error.

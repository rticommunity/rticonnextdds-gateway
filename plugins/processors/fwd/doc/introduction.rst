.. include:: vars.rst

.. _section-introduction:

Introduction
============

|RSFWD_TM| is a processor plugin for |RTI_RS| that allows you to forward *DDS*
data within a Route. This processor contains two different ways of forwarding
data:

* Forwarding Engine (By Input Name): selects the Output to which a sample
  should be forwarded within a Route based on the name of the Input that produced
  it.
* Forwarding Engine (By Input Value): selects the Output to which a sample
  should be forwarded within a Route based on the value of one of the fields of
  the sample.


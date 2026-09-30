.. include:: vars.rst

Additional Resources
====================

Build integration
-----------------

Enable ``RTIGATEWAY_ENABLE_TSFM_UNBOUNDED2BOUNDED`` in the root project.
``RTIGATEWAY_ENABLE_TESTS`` enables the generated types and four tests:

* ``tsfm_unbounded2bounded_unit``: numeric boundaries, schemas, metadata and loans.
* ``tsfm_unbounded2bounded_integration``: real integer and floating-point routes.
* ``tsfm_unbounded2bounded_reject_extra``: rejects additional input fields.
* ``tsfm_unbounded2bounded_reject_mismatch``: rejects different numeric kinds.

Sources and runnable configuration are under
``plugins/transformations/unbound2bound/test``. The harness accepts
``--test-dir``, ``--domain-id``, ``--timeout`` and ``--config``. Negative tests
terminate the entire Routing Service process tree after capturing the expected
diagnostic. Per-message discovery/receive deadlines and outer test timeouts
prevent malformed configurations from hanging indefinitely.

Enable ``RTIGATEWAY_ENABLE_DOCS`` and build
``unbounded2bounded-transformation-doc`` for this manual. The original array
transformation remains independently buildable; no original tests are removed.

API references
--------------

* `Routing Service Transformation API <https://community.rti.com/static/documentation/connext-dds/7.7.0/doc/api/routing_service/api_cpp/classrti_1_1routing_1_1transf_1_1Transformation.html>`_
* `DynamicData API <https://community.rti.com/static/documentation/connext-dds/7.7.0/doc/api/connext_dds/api_cpp2/classdds_1_1core_1_1xtypes_1_1DynamicData.html>`_
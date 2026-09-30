.. include:: vars.rst

Configuration
=============

Plugin registration
-------------------

Load ``rtiunbounded2boundedtransf`` with creation function
``Unbounded2BoundedTransformationPlugin_create_transformation_plugin``.
Debug builds append ``d`` to the library base name. The plugin accepts no
configuration properties; the output sequence bound is the chunk size.

Define and register both types explicitly. XML ``sequenceMaxLength="-1"``
declares an unbounded sequence; omitting the attribute declares a scalar.
Use ``type="boolean"`` for the output's required ``more_data`` member.

The following tested configuration contains independent integer and
floating-point forward routes. ``input_type_name`` and
``input_connection_name`` identify the transformation input type and participant.
Immediate endpoint creation validates types without waiting for discovery.

.. literalinclude:: ../test/xml/tsfm_unbounded2bounded.xml
   :language: xml

Local execution
---------------

Set ``U2B_DOMAIN`` to an unused DDS domain. Set ``NDDS_QOS_PROFILES`` to
``tsfm_unbounded2bounded_test_qos.xml`` and add the plugin build directory to
``LD_LIBRARY_PATH`` on Linux, ``DYLD_LIBRARY_PATH`` on macOS, or ``PATH`` on
Windows. Start ``rtiroutingservice`` with ``-cfgFile`` pointing to the generated
route XML and ``-cfgName TestService``. Use generated XML for Debug library-name
substitution. The test script supplies these settings automatically.

Test QoS
--------

The local test uses reliable delivery, volatile durability, shared-memory
transport and KEEP_ALL history with capacity for 256 samples per endpoint.
This is a test workload limit, not a deployment guarantee. Increase limits for
larger bursts and account for slow consumers. Initial allocation must not exceed
maximum limits. Depth-1 KEEP_LAST history can discard all but the final chunk.

.. literalinclude:: ../test/xml/tsfm_unbounded2bounded_test_qos.xml
   :language: xml

Failure and lifecycle policy
----------------------------

Schema violations log ``Unbounded2Bounded schema:`` and prevent transformation
creation. Routing Service may continue running with the affected output disabled;
monitor route errors rather than treating process survival as success.

Invalid-data disposal/no-writers notifications are skipped without payload access.
Valid data produces distinct DynamicData and SampleInfo objects for every chunk.
If conversion fails, staged callback outputs are destroyed and the error is
logged and propagated. This is not atomic DDS publication or a retry guarantee.

Framing and resource limits
---------------------------

``more_data`` is true while values remain in the current input, not while future
input samples are expected. Consumers need an intact, ordered, noninterleaved
stream and a known starting boundary. The flag cannot detect missing chunks or
identify messages. Late joining, restart recovery, multiple output writers and
partial durable replay require an external synchronization protocol or additional
schema fields.

Chunking bounds individual output samples, not aggregate memory or transport
packet sizes. One input vector, all staged outputs, metadata and DDS queues may
coexist. Establish an upstream operational input-size limit; the plugin does not
silently truncate. Lifespan expiry and reader filtering must not discard chunks.
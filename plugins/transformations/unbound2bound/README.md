# RTI Routing Service Unbounded2Bounded Transformation

Splits one unbounded numeric sequence into distinct bounded-sequence samples.
The output sequence bound determines the chunk size; no plugin properties are
required. Only forward conversion is supported.

## Type Contract

The input must be an unkeyed structure with exactly one required unbounded
sequence. The output must contain a required bounded sequence with the same
member name and numeric element kind, plus a required boolean `more_data`.
Member order and type aliases are supported. Extra fields, optional/key members,
inheritance, arrays, unions, nested collections, enums, characters, strings and
boolean sequence elements are rejected at creation with a logged error.

```idl
module processor {
		struct unbounddata { sequence<long> data; };
		struct BoundData {
				sequence<long, 1000> data;
				boolean more_data;
		};
};
```

Numeric support includes signed/unsigned 16-, 32-, 64-bit integers, octet,
float, double, and IDL long double. Explicit int8/uint8 use the repository's
Connext capability check. Numeric kinds must match; no precision/signedness
conversion occurs. Long double uses `rti::core::LongDouble` scalar access and
preserves the SDK representation, without promising native binary128 arithmetic.

Each input's chunks are emitted consecutively, without padding or merging.
`more_data` is true except on that input's final chunk. Empty input produces
one empty chunk with false. Exact multiples produce no extra terminator.
Invalid-data lifecycle notifications are skipped without reading the payload.
Distinct data and SampleInfo objects remain owned until `return_loan()`.

## Build and Run

Enable `RTIGATEWAY_ENABLE_TSFM_UNBOUNDED2BOUNDED` in the root CMake project.
Enable `RTIGATEWAY_ENABLE_TESTS` to build the direct and integration tests;
their names begin with `tsfm_unbounded2bounded_`.

The library is `rtiunbounded2boundedtransf` (with `d` appended for Debug), and
the factory is
`Unbounded2BoundedTransformationPlugin_create_transformation_plugin`.

[test/xml/tsfm_unbounded2bounded.xml](test/xml/tsfm_unbounded2bounded.xml) defines
and registers both integer and floating-point types and forward-only routes.
[test/xml/tsfm_unbounded2bounded_test_qos.xml](test/xml/tsfm_unbounded2bounded_test_qos.xml)
provides reliable, volatile, shared-memory QoS for local testing. Set
`U2B_DOMAIN` to the desired domain, `NDDS_QOS_PROFILES` to that QoS file, and
add the built library directory to the platform's library search path. Start
Routing Service with `-cfgFile` pointing to the generated route XML and
`-cfgName TestService`. Use the generated XML for Debug DLL substitution.

The Python test runner supplies these settings, starts the service and typed
tester, and enforces deadlines. `--reject extra` and `--reject mismatch` verify
schema diagnostics and failed transformation creation, not just missing output.

## Deployment Limits

- `more_data` is a boundary flag, not a message ID or loss detector. Consumers
	require an intact, ordered, noninterleaved stream starting at a known boundary.
	Late joins, restarts, multiple output writers and partial replay require an
	external synchronization protocol or a richer schema.
- Reliable delivery alone is insufficient: size history/resource limits for
	bursts and slow readers. Depth-1 history, filtering and lifespan expiry can
	discard chunks. The supplied limits cover the tests, not arbitrary workloads.
- Chunking bounds individual output samples, not peak memory or network packets.
	The input, one temporary numeric vector and all output chunks coexist until
	the output loan returns. Apply operational input-size limits upstream.
- Runtime failures roll back staged callback outputs, but DDS writes are not
	atomic or exactly-once. Monitor Routing Service errors; a failed route may
	leave the service process running.

Old Sequence2Array/Array2Sequence factories and recursive array schemas are not
supported by this plugin. The separate original plugin is unchanged.

Validated locally with Connext 7.7.0 on Linux x64. The repository's 7.3 minimum
is retained; other SDK/platform combinations require their own validation.

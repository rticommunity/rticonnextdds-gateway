# Kafka adapter unit tests

The suite runs 93 deterministic CTest cases against all four production adapter
modules. It uses real Connext DynamicData and semaphores, scripted Kafka I/O,
and a thread lifecycle double, plus three real-worker shutdown tests. Neither a
broker nor librdkafka is needed for the standalone build. The Connext SDK, a C compiler, and CMake are required.

## Run

From the repository root:

```sh
cmake -S plugins/adapters/kafka/test -B build/kafka-unit \
    -DCONNEXTDDS_DIR=/path/to/rti_connext_dds \
    -DCONNEXTDDS_ARCH=your_connext_architecture \
    -DCMAKE_BUILD_TYPE=Debug
cmake --build build/kafka-unit
ctest --test-dir build/kafka-unit --output-on-failure
```

The tests are also included in the normal gateway build when
`RTIGATEWAY_ENABLE_KAFKA` and `RTIGATEWAY_ENABLE_TESTS` are enabled. That build
still requires librdkafka to build the adapter. Select only Kafka tests with
`ctest --test-dir build -R '^kafka_' --output-on-failure`.

For Clang or GCC, add `-DKAFKA_TEST_SANITIZERS=ON` to enable AddressSanitizer and
UndefinedBehaviorSanitizer. Add `-DKAFKA_TEST_COVERAGE=ON` for gcov instrumentation.
Use a fresh build directory to collect coverage for a single run. On macOS,
`xcrun llvm-cov gcov -b -c <object-file>.gcno` generates line and branch reports;
on GCC, use `gcov`. Adapter/connection coverage is included in the
`KafkaLifecycleTests.c.gcno` report because that translation unit includes their
production sources with local dependency wrappers.

## Coverage

| Area | Cases | Behaviors checked |
| --- | ---: | --- |
| Stream writer | 27 | Text mode with optional, single and repeated terminators, embedded zeros, empty values, retries and batches; binary mode with trailing NUL; embedded, leading and trailing zero bytes; every octet value; non-terminated and empty payloads; batches; mixed sizes; repeated calls; queue-full retries; permanent failures before and after retries; failures at different batch positions; missing payloads; zero-sample writes. |
| Stream reader | 16 | Binary, empty and null Kafka values; idle and stopped workers; repeated timeouts and errors; repeated messages; invalid output samples; synchronous notification/read; repeated reads; loan and message ownership; real-thread stop before polling, during a blocked poll, and with a pending loan. |
| Adapter | 2 | Callback registration and allocation failure. |
| Connection | 3 | Bootstrap configuration, missing bootstrap property and allocation failure. |
| Writer lifecycle | 16 | Creation/deletion, required fields, allocation/configuration/client failures, default text mode, explicit text/binary mode, invalid/empty format rejection, property forwarding and filtering, credential log suppression, flush with pending messages, delivery callbacks. |
| Reader lifecycle | 29 | Creation/deletion, every direct adapter allocation failure, DDS sample/type-support failure, configuration failures, queue redirection, subscription allocation/add/subscribe failures, all three semaphore allocations, initial signaling failure, thread creation failure, group override, credential log suppression, shutdown with an outstanding loan, consumer close failure; failed stop signaling, wakeup and join retain resources, followed by successful deletion retry. |

All 93 cases pass in Release and in a fresh Debug build with AddressSanitizer
and UndefinedBehaviorSanitizer, using Connext Professional 7.7.0.1. These
sanitizers do not detect data races. The earlier 73-case coverage run measured
100% of executable lines in `KafkaAdapter.c`, 98.4% in `KafkaConnection.c`,
96.7% in `KafkaStreamWriter.c`, and 82.7% in `KafkaStreamReader.c`.
These figures describe instrumented line execution, not exhaustive branch or
concurrency coverage.

## Test design and contracts

`KafkaAdapterTests.c` checks stream behavior using real DDS types and semaphores.
`FakeKafka.c` captures payloads, copies producer values, injects Kafka failures,
and accounts for configuration, client and subscription ownership.
`KafkaLifecycleTests.c` compiles the production adapter and connection sources
with wrappers for their allocation, DDS-object, semaphore, thread, error and
logging dependencies. Wrappers account for resources and check balanced cleanup
on both successful and failed construction; they do not replace the process
allocator or DDS internals.

Most reader cases run synchronously with a scripted final message that stops
the loop. Three additional cases run the actual worker on an SDK thread, using
semaphores to coordinate shutdown without sleeps or shared flags. The lifecycle
thread double checks initialization and shutdown order, including waking a
worker waiting for a loan before joining it. Failure injection verifies that
failed stop signaling, wakeup or join leaves the reader, outstanding payload,
DDS objects and Kafka client intact until a successful deletion retry. CTest
gives each case a ten-second deadline so a blocked read fails instead of hanging the
suite. Unknown case names are rejected. Checks remain enabled in Release builds;
no cases are disabled or marked `WILL_FAIL`.

Write success means Kafka accepted a message into its producer queue. A failed
write returns the successfully enqueued prefix length. Final broker delivery is
asynchronous. An empty outgoing value uses a non-null pointer and zero length;
incoming tombstones map to an empty sequence because the adapter's message type
does not distinguish the two.

The binary fixture is a Protobuf wire payload for a string containing an embedded
zero followed by an integer field. No Protobuf dependency is needed to verify
byte preservation; transformation correctness belongs to the separate Protobuf
plugin suite.

## Remaining integration coverage

An opt-in [DDS/Kafka integration suite](integration/README.md) now checks actual
broker delivery and native Routing Service plugin loading in both directions.
The unit suite itself does not validate authentication, rebalances,
offset commits, network failures or librdkafka ABI compatibility. The real-worker
cases cover specific shutdown interleavings; they do not provide exhaustive
concurrency or data-race coverage. Join failure is injected through the lifecycle
double. Some stream-level DDS/semaphore failure branches remain unexercised. The precompiled
Connext SDK itself is outside the sanitizer instrumentation; resource accounting
covers adapter-owned objects, not every internal SDK allocation.

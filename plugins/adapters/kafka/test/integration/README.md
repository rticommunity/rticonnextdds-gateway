# Local DDS/Kafka regression test

This opt-in test uses real DDS applications, the installed Routing Service
executable, dynamically loaded adapter/transformation plugins, and a local Kafka
3.9.1 broker. Kafka calls and thread scheduling are not mocked.

## Build and run

Install the Connext host and native target packages, Docker Compose, Python 3,
and librdkafka. The repository pins librdkafka revision
`c024ac13daf98667de2b8724986e97f489644c15` (2.10.0); use that revision for a
comparison representative of this checkout. Build it with its examples and
tests disabled and install it into a separate prefix.

From the repository root:

```sh
cmake -S plugins/adapters/kafka/test -B build/kafka-integration \
    -DCONNEXTDDS_DIR=/path/to/rti_connext_dds \
    -DCONNEXTDDS_ARCH=your_connext_architecture \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
    -DKAFKA_TEST_INTEGRATION=ON \
    -DKAFKA_RDKAFKA_ROOT=/path/to/librdkafka/install
cmake --build build/kafka-integration

docker compose -p piddock-kafka-integration \
    -f plugins/adapters/kafka/test/integration/compose.yaml up -d --wait
ctest --test-dir build/kafka-integration \
    -R '^kafka_broker_regression$' --output-on-failure
docker compose -p piddock-kafka-integration \
    -f plugins/adapters/kafka/test/integration/compose.yaml down
```

The broker binds only to `127.0.0.1:19092`. The test creates unique topics and
deletes them afterward. DDS uses domain 173 and UDP loopback; shared memory and
external observability profiles are excluded from the test. Use a fresh build
directory after replacing the SDK. `BUILD_SHARED_LIBS=ON` ensures the loaded
plugins use the same DDS runtime as Routing Service.

`KAFKA_TEST_BASELINE_REF` defaults to the original adapter revision
`d8250299084b2150fa1b53103b1f0d020a3560e7`, so committing the fixes does not
change the comparison baseline. CMake snapshots that revision's
adapter sources into the build directory and compiles them against the same SDK
and librdkafka as the modified adapter. Set it to the desired original revision,
or set it empty to run only the modified adapter.
`KAFKA_TEST_ROUTING_SERVICE` can override the native executable. An embedded API
host is also built as `kafka_integration_router` for diagnostic use.

## Scenarios and evidence

Each adapter version receives the same four scenarios, with twenty numbered
samples per scenario:

| Scenario | Path | Check |
| --- | --- | --- |
| `json_out` | DDS ShapeType -> JSON transformation -> Kafka | Exact field values and counts; strict JSON parsing; trailing NUL detection. |
| `json_in` | Kafka JSON -> JSON transformation -> DDS ShapeType | Exact field values, order and counts. |
| `bytes_out` | DDS byte envelope -> Kafka | Exact bytes, including an empty value, embedded and trailing zeros, and high-bit bytes. |
| `bytes_in` | Kafka bytes -> DDS byte envelope | Exact bytes, including an empty value, embedded and trailing zeros, and high-bit bytes. |

JSON checks compare all twenty samples in order with `color="BLUE"`, `x=i`,
`y=2*i`, and `shapesize=30`, for `i` from 0 through 19. Outbound Kafka values
are parsed as strict JSON; inbound DDS samples are checked for the same field
values. Counts and order must match as well.

Nonempty binary fixtures are valid Protobuf wire messages with a bytes field.
They test binary transport; this suite does not load or validate the Protobuf
transformation plugin itself, and does not decode field values with a Protobuf
library. The comparison checks the complete expected wire bytes, including the
sample ID. The modified binary output sets
`payload_format=binary`; the baseline omits this new adapter property. JSON
output omits it for both versions, exercising the modified writer's default
`text` mode. Both versions use the unchanged JSON transformation. An empty
value is included to exercise the adapter's zero-length handling.

Results are written to `<build>/integration/results/summary.json`, with XML,
process logs and `result.json` files for each version/scenario. Clean Routing
Service shutdown is checked for every case. A mismatch or failure in the
modified adapter makes CTest fail. Original-adapter failures are recorded as
negative controls and do not by themselves make CTest fail.

The JSON check is deliberately strict: diagnostic parsing may remove a trailing
NUL to compare field values, but the scenario still fails if strict consumers
would reject the original wire value. No test is disabled or marked `WILL_FAIL`.

For diagnosis, invoke `run_integration.py` with the paths printed by
`ctest --test-dir build/kafka-integration -N -V`. It supports `--scenario`,
`--variant`, `--count`, `--domain`, and a separate artifacts directory.
`--start-broker` creates the configured Compose stack and removes it on exit;
omit it when using a broker stack you already started.

These scenarios do not cover broker restart, sustained load, authentication,
partition reassignment, offset recovery, or an exhaustive duplicate-delivery
window. Subscribers check the requested sample count and content, then exit.

## Observed results

On 2026-10-20, using Connext Professional 7.7.0.1, Kafka 3.9.1, librdkafka
2.10.0 and baseline revision `d825029`, the comparison produced:

| Scenario | Original adapter | Updated adapter |
| --- | --- | --- |
| JSON DDS -> Kafka | Pass, 20/20 | Pass, 20/20, strict JSON, no trailing NUL |
| JSON Kafka -> DDS | Pass, 20/20 | Pass, 20/20 |
| Binary DDS -> Kafka | Fail: 19/20, bytes corrupted | Pass, 20/20, exact bytes |
| Binary Kafka -> DDS | Pass, 20/20 | Pass, 20/20, exact bytes |

All eight Routing Service processes stopped normally. All 93 unit cases also
passed in the same Release build.

An earlier byte-exact writer change exposed a JSON compatibility regression:
the unchanged JSON transformation deliberately includes a string terminator in
its output sequence. The writer now defaults to `payload_format=text`, omitting
exactly one final NUL. Binary outputs explicitly select `payload_format=binary`
to preserve every byte. The binary fixtures include legitimate trailing zeros
as well as embedded zeros, high-bit bytes and an empty value. The JSON and
Protobuf transformation implementations remain unchanged.

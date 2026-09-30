# Example Unbounded2Bounded

## Description of the Example

This example uses the Unbounded2Bounded transformation to split an unbounded
sequence of integers into samples containing bounded sequences. The subscriber
reassembles the original sequence and prints its length, number of chunks and
checksum for comparison with the publisher.

The publisher uses `processor::UnboundData` on topic `unbound_data`. On each
iteration, it publishes between 1,500 and 5,000 elements containing pseudorandom
integers from 0 through 1,000,000, and prints the sequence length and checksum.

The subscriber uses `processor::BoundData` on topic `bound_data`. Each sample
contains at most 500 elements. The `more_data` flag is true until the final
chunk of the current input sequence. When the subscriber receives false, it
reports the completed sequence and clears its reassembly buffer.

The relevant definitions in [types.idl](types.idl) are:

```idl
module processor {
    @appendable
    struct UnboundData {
        sequence<int32> data;
    };

    @appendable
    struct BoundData {
        sequence<int32,500> data;
        boolean more_data;
    };
};
```

Both applications calculate the same simple checksum: the sum of the element
values accumulated in a 64-bit unsigned integer. Compare the lengths and
checksums in the two consoles; the checksum is not transmitted as another field.

### Routing Service Configuration

[RsUnbounded2Bounded.xml](RsUnbounded2Bounded.xml) provides the
`Unbounded2BoundedExample` configuration. It defines and registers both types,
reads `unbound_data`, applies the transformation, and writes `bound_data`.
All endpoints use domain **0**, reliable delivery and KEEP_ALL history.

The transformation is forward-only. Its chunk size comes from the output
sequence bound, not from a plugin property.

## Running the Example

Build the repository with `RTIGATEWAY_ENABLE_TSFM_UNBOUNDED2BOUNDED=ON` and
`RTIGATEWAY_ENABLE_EXAMPLES=ON`. CMake generates the type support using
`connextdds_rtiddsgen_run` with `-unboundedSupport`. Use a **Release** build for
the library name in the supplied configuration.

Three processes are required: Routing Service, the subscriber and the publisher.
Run them in separate terminals from
`<install dir>/examples/unbounded2bound/`. On Linux, set the following in
each terminal, replacing the installation paths as needed:

```sh
export NDDSHOME=/opt/rti_connext_dds-7.7.0
export GATEWAY_INSTALL=/path/to/rticonnextdds-gateway/install
export PATH="$NDDSHOME/bin:$PATH"
export LD_LIBRARY_PATH="$GATEWAY_INSTALL/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$GATEWAY_INSTALL/examples/unbounded2bound"
```

> **NOTE**: `librtiunbounded2boundedtransf.so` must be reachable by the operating
> system. Add the directory containing that file to `LD_LIBRARY_PATH`, not the
> filename itself. The configuration loads it using the base name
> `rtiunbounded2boundedtransf`.

Start Routing Service:

```sh
rtiroutingservice -cfgFile RsUnbounded2Bounded.xml -cfgName Unbounded2BoundedExample
```

Wait for Routing Service to report that it is executing, then start the
subscriber in the second terminal:

```sh
./unbound2bound_subscriber --domain 0
```

Start the publisher in the third terminal:

```sh
./unbound2bound_publisher --domain 0
```

The publisher sends one sequence per second. For example, the corresponding
notifications might be:

```text
Published sequence 1: length=2348, checksum=1183892276
Reconstituted sequence 1: length=2348 (5 chunks), checksum=1183892276
```

Lengths and checksums vary with the generated data. Matching values on both
sides indicate successful reassembly for this demonstration. Start the
subscriber before publishing and use a single publisher so the sequence groups
remain ordered. The simple checksum is not a cryptographic integrity check.

Press Ctrl+C to stop each process. To send and receive three complete sequences,
add `--sample-count 3` to both application commands; the subscriber counts
reconstituted sequences, not individual chunks.

### Running From the Build Directory

Installation is optional. Run all three commands from
`<build dir>/examples/unbound2bound/`, which contains the executables and a copy
of the Routing Service XML. Set `LD_LIBRARY_PATH` to include
`<build dir>/plugins/transformations/unbound2bound/` instead of the installed
library directory. Use the same Routing Service command shown above.
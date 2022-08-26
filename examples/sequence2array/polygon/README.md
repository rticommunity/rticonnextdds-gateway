# Example Sequence2Array Polygon

## Description of the Example

This example shows how to translate Dynamic Data objects with sequences of type
`polygonSeq` and `polygonSetSeq` to a compatible type with arrays instead of
sequences.

The IDL definitions for `polygonSeq` and `polygonSetSeq` are:

```idl
struct point {
  int32 x;
  int32 y;
};

struct polygonSeq {
  sequence<point> polygon;
};

struct polygonSetSeq {
  sequence<polygonSeq> severalPolygons;
};

```

A compatible type means that the type is the same but the sequences are replaced
by arrays. The names may be different, but the index should match.

Therefore, their compatible types are `polygonArray` and `polygonSetArray`:

```idl
struct point {
  int32 x;
  int32 y;
};

struct polygonArray {
  point polygon[4];
};

struct polygonSetArray {
  polygonArray severalPolygons[4];
};

```

### Routing Service Configuration

The example provides a Routing Service configurations in file
[RsSequence2ArrayTransf.xml](RsSequence2AraryTransf.xml) which demonstrate
the use of the Sequence2Array transformation.

- `RsSequence2ArrayTransf`
  - Read `polygonSeq` and `polygonSetSeq` samples from topic `PolygonTopicSeq`
    and `PolygonSetTopicSeq` respectively. Then, it converts these samples to
    `polygonArray` and `polygonSetArray` in topics `PolygonTopicArray` and
    `PolygonSetTopicArray`.

## Running the Example

In order to run the integration test, three elements are required (all of them
must be run from directory `<install dir>/examples/tsfm_sequence2array/polygon/`):

- Publisher:

```sh
./publisher/PolygonSeq_publisher
```

- Subscriber:

```sh
./subscriber/PolygonArray_subscriber
```

Start a Routing Service instance:

```sh
rtiroutingservice -cfgFile RsSequence2ArrayTransf.xml -cfgName RsSequence2ArrayTransf
```

> **NOTE**: the `rtisequence2arraytransf` library should be reachable by the OS. Add
> the corresponding folders to your `LD_LIBRARY_PATH`, `PATH` or
> `RTI_LD_LIBRARY_PATH` environment variable (depending on your OS). If you
> followed the default building instructions, the folder to include is:
> `<path to rticonnextdds-gateway>/install/lib`.

At this time, you should be receiving `polygonArray` and `polygonSetArray` in
the subscriber application.

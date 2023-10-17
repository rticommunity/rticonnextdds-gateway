# Example Array2Sequence Polygon

## Description of the Example

This example shows how to translate Dynamic Data objects with arrays of type
`polygonArray` and `polygonSetArray` to a compatible type with sequences instead
of arrays.

The IDL definitions for `polygonArray` and `polygonSetArray` are:

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

A compatible type means that the type is the same but the arrays are replaced
by sequences. The names may be different, but the index should match.

Therefore, their compatible types are `polygonSeq` and `polygonSetSeq`:

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



### Routing Service Configuration

The example provides a Routing Service configurations in file
[RsArray2SequenceTransf.xml](RsArray2SequenceTransf.xml) which demonstrate
the use of the Sequence2Array transformation.

- `RsArray2SequenceTransf`
  - Read `polygonArray` and `polygonSetArray` samples from topic `PolygonTopicArray`
    and `PolygonSetTopicArray` respectively. Then, it converts these samples to
    `polygonSeq` and `polygonSetSeq` in topics `PolygonTopicSeq` and
    `PolygonSetTopicSeq`.

## Running the Example

In order to run the integration test, three elements are required (all of them
must be run from directory `<install dir>/examples/tsfm_sequence2array/polygon2/`):

- Publisher:

```sh
./publisher/PolygonArray_publisher
```

- Subscriber:

```sh
./subscriber/PolygonSeq_subscriber
```

Start a Routing Service instance:

```sh
rtiroutingservice -cfgFile RsArray2SequenceTransf.xml -cfgName RsArray2SequenceTransf
```

> **NOTE**: the `rtisequence2arraytransf` library should be reachable by the OS. Add
> the corresponding folders to your `LD_LIBRARY_PATH`, `PATH` or
> `RTI_LD_LIBRARY_PATH` environment variable (depending on your OS). If you
> followed the default building instructions, the folder to include is:
> `<path to rticonnextdds-gateway>/install/lib`.

At this time, you should be receiving `polygonSeq` and `polygonSetSeq` in
the subscriber application.

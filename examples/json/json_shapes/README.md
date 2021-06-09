# Example JSON Shapes

## Description of the Example

This example shows how to translate a Dynamic Data object of type `ShapeTypeExtended` to
a string containing its DDS-JSON representation and vice versa.

The IDL definition for `ShapeTypeExtended` is:

```idl
enum ShapeFillKind {
    SOLID_FILL,
    TRANSPARENT_FILL,
    HORIZONTAL_HATCH_FILL,
    VERTICAL_HATCH_FILL
};

struct ShapeType
{
    string<128> color; //@key
    long x;
    long y;
    long shapesize;
};//@Extensibility EXTENSIBLE_EXTENSIBILITY

struct ShapeTypeExtended : ShapeType {
    ShapeFillKind fillKind;
    float angle;
};//@Extensibility EXTENSIBLE_EXTENSIBILITY

```

The JSON plugin can use any _string-like_ field of a Dynamic Data object to store and read a DDS-JSON representation. The target field can be any of the following types:

* `string`
* `sequence<octet>`
* `sequence<char>`
* `octet[N]`
* `char[N]`

The example uses type `MessagePayload` to store DDS-JSON representations:

```idl
struct MessagePayload {
    string data;    //@ID 0
};
//@Extensibility EXTENSIBLE_EXTENSIBILITY
```

### Routing Service Configuration

The example provides two Routing Service configurations in file [json_shapes.xml](examples/json/json_shapes/json_shapes.xml) which demonstrate the use of the JSON plugin in both directions:

- `ShapeToJson`
  - Read `ShapeTypeExtended` samples from topic `Square` on domain `0`, convert them to DDS-JSON, store the result in `MessagePayload::data`, and publish `MessagePayload` to topic `SquareJson` on domain `1`.
- `JsonToShape`
  - Read `MessagePayload` samples from topic `SquareJson` on domain `1`, convert `MessagePayload::data` to `ShapeTypeExtended`, publish `ShapeTypeExtended` to topic `Circle` on domain `2`.


## Running the Example

Start two instances of RTI Shapes Demo:
- Shapes Demo 1:
  - Go to *Controls > Configuration*, and make sure the domain ID is `0`.
  - Start publishing a `Square` with default options ("BLUE" color).
- Shapes Demo 2:
  - Go to *Controls > Configuration*, and change the domain ID to `2`.
  - Subscribe to topic `Circle` with default options.

In two separate command prompt windows for Routing Service instances that will
run both scenarios described above, we run the following commands from the
example directory (this is necessary to ensure the application find the
configuration files):

* ShapeTypeExtended to Content Container

```sh
rtiroutingservice -cfgFile shapes_to_json_transf.xml -cfgName JsonTransformationExample
```

* Content Container to ShapeTypeExtended

```sh
rtiroutingservice -cfgFile json_to_shapes_transf.xml -cfgName JsonTransformationExample
```

> **NOTE**: the `rtijsontransf` library should be reachable by the OS. Add the
> corresponding folders to your `LD_LIBRARY_PATH`, `PATH` or
> `RTI_LD_LIBRARY_PATH` environment variable (depending on your OS). If you
> follow these instructions, the folder is: `.../rticonnextdds-gateway/install/lib`.

At this time, you should see a BLUE circle in the _Shape Demo 2_.

Additionally you may want to see what is sent to the topic `SquareJSON` in the
domain ID 1 by using RTI Admin Console or RTI DDS Spy. For example, using
RTI DDS Spy you will see an output similar to:

> 1623083351.947987  d +M  0A3200CA    SquareJSON          MessagePayload
> data: "{ "color":"BLUE", "x":92, "y":25, "shapesize":30, "fillKind":"SOLID_FILL", "angle":0 } "

The `json_shapes.xml` configuration file contains alternative data types which show the use of other _string-like_ types as containers for DDS-JSON representations.

If you want to test these different data types, you can comment the
`<types>` tag in the XML file, and enable one of the alternative definitions which are commented out by default.

# Example JSON Shapes

## Description of the Example

This example shows to translate a Dynamic Data object (`ShapeTypeExtended`) to a
DDS-JSON representation of it and viceversa.

The IDL representation of the `ShapeTypeExtended` is:

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

The DDS-JSON representation is contained in a Dynamic Data object with a
_string-like_ field:

* `string`
* `sequence<octet>`
* `sequence<char>`
* `octet[N]`
* `char[N]`

From now on, the Dynamic Data which contains the DDS-JSON representation is
called `Content Container` since it contains the DDS-JSON content.

This example uses the following datatype as `Content Container` to store
DDS-JSON samples:

```idl
struct MessagePayload {
    string data;    //@ID 0
};
//@Extensibility EXTENSIBLE_EXTENSIBILITY
```

### ShapeTypeExtended to Content Container

This example reads samples in the topic `Square` of type `ShapeTypeExtended` in
the domain ID 0 and publishes a DDS-JSON representation of them in the topic
`SquareJSON` in the domain ID 1.

### Content Container to ShapeTypeExtended

In order to show how to transform a DDS-JSON sample to Dynamic Data, this
example will read `Content Container` samples from the domain ID 1 and topic
`SquareJSON` and will publish them in the domain ID 2 in the topic `Circle`.


## Running the Example

Open two separate RTI Shape Demo applications and:
* Shape Demo 1:
  * Change domain ID to 0 in Controls/Configuration.
  * Publish a `Square` with the default options (BLUE color).
* Shape Demo 2
  * Change domain ID to 2 in Controls/Configuration.
  * Subscribe to the `Circle` topic with the default options.

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

If you want to test different _string-like_ datatypes, you can comment the
`<types>` tag in both XML files (`shapes_to_json_transf.xml` and
`json_to_shapes_transf.xml`) and uncomment a different datatype definition.
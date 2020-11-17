# Integration test

## Integration test 1

### Description

The integration_test1 is an application that writes data from DDS to a Modbus
server, and vice versa, it also reads data from a Modbus server and publishes
it as a DDS Sample.

After that, the application tests that the data written/read to/from the Modbus
server is the same that the DDS application has published.

The configuration of the adapter is described in
rticonnextgateways/plugins/adapters/modbus/test/integration_test1/RSConfigTest.xml.

The RSConfigTest.xml file configures the following elements:

* Connection
  * TestConnection: connection that includes the IP and port
* Types
  * TestType_pub: writes data from DDS to Modbus
  * TestType_sub: reads data from Modbus to DDS
* Routes
  * dds_input and Modbus output: this route publishes data from DDS and reads
    it in Modbus according to the configuration under
    integration_test1/config/test_pub_config.json
  * dds_output and Modbus input: this route reads data from Modbus and
    publishes it in DDS according to the configuration under
    integration_test1/config/test_sub_config.json

> **_NOTE:_**  the QoS of the DDS entities is set to use RELIABILITY, KEEP_LAST
> with depth 1 and TRANSIENT_LOCAL. These QoS settings are required to run integration_test1.

The JSON configuration of these routes indicates the matching between DDS
fields and Modbus registers, as well as other parameters that the Modbus
Adapter allows you to use it. The full list of parameters can be found in the
documentation.

These configurations use different Modbus datatypes (defined by this adapter),
as well as it matching to DDS datatypes (including arrays, sequences, strings,
enums, and optional members). This also includes at least one use of the
optional configuration attributes:

* modbus_register_count
* modbus_min_value
* modbus_max_value
* modbus_valid_values
* ouput_data_factor or input_data_factor
* output_data_offset or input_data_offset
* value

### Behavior

The integration_test1 will write one sample of the type *TestType_pub* using the
topic *Modbus Adapter Test Pub*. As this test only publishes one sample, it has
to be reliable to ensure that the Modbus Adapter has received it.

The Modbus Adapter will read the data from the DDS sample and forward it to
a Modbus server according to the configuration *test_pub_config.json*.

Later, the integration_test1 will subscribe to the Modbus Adapter and will read
the data that comes from the Modbus server, including a set of input registers.
As the DataReader is created after publishing the data, TRANSIENT_LOCAL is
the mandatory durability policy.

Finally, integration_test1 checks whether the received data is the expected one
and shows an "OK" message or an error otherwise.

### How to run Integration Test 1

In order to run the integration_test1 three elements are required:

1. Add the folders which contains libmodbus and rtimodbusadapter libraries to
your LD_LIBRARY_PATH, PATH or RTI_LD_LIBRARY_PATH environment variable (depending
of your OS).

1. Modbus server (bin directory)

    ```sh
    ./modbusserver
    ```

1. Routing Service with the configuration RSConfigTest.xml (test/modbus/integration_test1/ directory)

    ```sh
    rtiroutingservice -cfgFile RSConfigTest.xml -cfgName RSModbusAdapterTest
    ```

1. The integration test 1 application (test/modbus/integration_test1/ directory)

    ```sh
    ./integration_test1
    ```

If the test has passed correctly, the following message will be printed:
**PASSED TEST OK**

> **_NOTE:_** after running Routing Service, you may see the following message:
> ```Error: value <0> of element <test_type.enum_field> not in the modbus_valid_values list.```
> This happens because the modbusserver application initializes everything
> to 0 and it is not part of the enum_field *modbus_valid_values*.

# Example MBus_WTH_CO2_LCD_ETH

## Description of the Example

This example shows to connect to the DataNab device [MBus_WTH_CO2_LCD_ETH](https://cdn.shopify.com/s/files/1/0274/6020/6726/files/MBus_xTH_LCD_ETH_Manual.pdf).
The configuration showed in this example is a subset of the one shown in the
documentation. This configuration contains the following fields:

* Modbus Output - MBus_WTH_CO2_LCD_ETH_WRITE
  * alarm_sound_on_sec_for_prealarm
* Modbus Input - MBus_WTH_CO2_LCD_ETH_INPUT.idl
  * device_name
  * ip_address
  * temperatureC
  * temperatureF
  * humidity
  * co2_value
  * analog_output_config
  * alarm_sound_on_sec_for_prealarm

You can find the IDL files that are used by this example under:

* *example/modbus/MBus_WTH_CO2_LCD_ETH/idl*

This example will read the data from the device every 2 seconds, and will
modify the *alarm_sound_on_sec_for_prealarm* every second. The configuration
of this parameter has a maximum of 20, therefore, the example will throw
errors when the value reaches 20.

If you don't have the DataNab MBus_WTH_CO2_LCD_ETH device, you can simulate
a Modbus server using the `bin/modbusserver` utility.

> **NOTE**: you need to modify the modbus_server_ip and modbus_server_port to
> point to the correct device IP.

## Running the Example

In three separate command prompt windows for the Routing Service, DDS publisher
and DDS subscriber. Run the following commands from the example directory (this
is necessary to ensure the application loads the QoS defined in
*USER_QOS_PROFILES.xml*):

```sh
rtiroutingservice -cfgFile RSConfigAdapter.xml -cfgName RSModbusAdapterExample
./MBus_WTH_CO2_LCD_ETH_INPUT
```

> **NOTE**: the `libmodbus` and `rtimodbusadapter` libraries should be reachable
> by the OS. Add the corresponding folders to your `LD_LIBRARY_PATH`, `PATH` or
> `RTI_LD_LIBRARY_PATH` environment variable (depending on your OS). If you
> follow these instructions, the folder is:
> `<path to rticonnextdds-gateway>/install/lib`.

After running the Routing Service and MBus_WTH_CO2_LCD_ETH_INPUT, you will read
messages from the modbus device:

> Received data
> &nbsp; device_name: "device_37"
> &nbsp; ip_address:
> &nbsp; &nbsp; &nbsp; ip_address[0]: <7F>
> &nbsp; &nbsp; &nbsp; ip_address[1]: <00>
> &nbsp; &nbsp; &nbsp; ip_address[2]: <00>
> &nbsp; &nbsp; &nbsp; ip_address[3]: <01>
> &nbsp; temperatureC: 39.6000023
> &nbsp; temperatureF: 103.200005
> &nbsp; humidity: 54.6000023
> &nbsp; co2_value: 436
> &nbsp; analog_output_config: <03>
> &nbsp; alarm_sound_on_sec_for_prealarm: 20

After that, run the DDS Writer

```sh
./MBus_WTH_CO2_LCD_ETH_WRITE
```

You will see how this modifies the value of *alarm_sound_on_sec_for_prealarm*.
As the RS configuration sets a polling time of 2 seconds, the changes in this
value appear every 2 seconds.

The *alarm_sound_on_sec_for_prealarm* will be updated in the range [0,21].
The value 21 will be out of the ranged specified in the JSON configuration.
Therefore the window that runs the Routing Service will show the following
message indicating the error:

> Error: value <21> of element <alarm_sound_on_sec_for_prealarm> out of range.

.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RSMODBUS|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the Modbus Adapter Plugin
==============================

|RSMODBUS| must be registered as a |RS| plugin by using the ``<adapter_plugin>``
tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPlugins">
            <adapter_plugin name="ModbusAdapter">
                <dll>rtimodbusadapter</dll>
                <create_function>
                    ModbusAdapter_create_adapter_plugin
                </create_function>
            </adapter_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |RSMODBUS| dynamic library
             (|MODBUS_ADAPTER_LIB_NAME_LINUX| on Linux® systems,
             |MODBUS_ADAPTER_LIB_NAME_MAC| on macOS® systems,
             or |MODBUS_ADAPTER_LIB_NAME_WIN| on Windows® systems). Make
             sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on
             macOS systems, or ``PATH`` on Windows systems, etc.).

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up, and
you can use the plugin to create one or more connections to |MODBUS_DEVICEs|.

.. _section-how-to-config-connection:

Modbus Connection
=================

Configuration
-------------

Once the plugin has been registered with |RS|, you can use it to create
``<connection>`` elements within a ``<domain_route>``.

A Connection identifies a |MODBUS_DEVICE|, therefore you have to specify the
|MODBUS_DEVICE|'s IP and port. In order to do that the ``<connection>``'s
configuration must include the properties ``modbus_server_ip`` and
``modbus_server_port`` to configure the associated |MODBUS_DEVICE|.

Optionally, you can also set a response timeout for a specific Modbus connection
by setting the property ``modbus_response_timeout_msec``. If not set, it will use
the default value of libmodbus.

The following snippet shows an example ``<connection>`` that connects
the adapter to a local |MODBUS_DEVICE|:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <routing_service>
            <domain_route>
                <connection name="CO2_Device_1" plugin_name="AdapterLib::ModbusAdapter">
                <property>
                    <value>
                        <element>
                            <name>modbus_server_ip</name>
                            <value>127.0.0.1</value>
                        </element>
                        <element>
                            <name>modbus_server_port</name>
                            <value>1502</value>
                        </element>
                        <element>
                            <name>modbus_response_timeout_msec</name>
                            <value>5000</value>
                        </element>
                    </value>
                </property>
                <registered_type name="MBus_WTH_CO2_LCD_ETH_WRITE" type_name="MBus_WTH_CO2_LCD_ETH_WRITE" />
                <registered_type name="MBus_WTH_CO2_LCD_ETH_INPUT" type_name="MBus_WTH_CO2_LCD_ETH_INPUT" />
            </connection>
            </domain_route>
        </routing_service>
    </dds>

The Modbus connection maps to a (TCP) connection to the Modbus service.
It maps to what typical Modbus client libraries do when they connect to
a Modbus server. For example, see `modbus_connect <https://libmodbus.org/docs/v3.1.6/modbus_connect.html>`__
and `modbus_new_tcp <https://libmodbus.org/docs/v3.1.6/modbus_new_tcp.html>`__.


.. _section-input-output:

Modbus Input/Output
===================

This adapter allows you to write or read multiple Modbus registers
or coils using the same input/output.

.. _section-modbus-output:

Modbus Output (Routing Data from DDS to Modbus)
-----------------------------------------------

Concept
~~~~~~~

A single Modbus output can write multiple Modbus holding registers
and/or coils within the same |MODBUS_DEVICE|. Therefore, the configuration
must provide a way to associate each of the DynamicData fields with
the Modbus register/coil where it should be written. This is basically
an array, where each element is a tuple consisting of:

- The |CONF_MODBUS_DATATYPE| (COIL, INT16, etc.)
- The |CONF_MODBUS_REGISTER_ADDRESS|

  - Optionally, a |CONF_MODBUS_REGISTER_COUNT| starting from that address

- The DynamicData member that is copied into the Modbus register/coil
- Optionally, in order to support linear transformations:

  - |CONF_OUTPUT_DATA_FACTOR|
  - |CONF_OUTPUT_DATA_OFFSET|

- Optionally, in order to ensure written values are within range for the Modbus server:

  - |CONF_MODBUS_MIN_VALUE|, |CONF_MODBUS_MAX_VALUE|
  - |CONF_MODBUS_VALID_VALUES|

The |RSMODBUS| allows you to write/read multiple registers by using the
JSON syntax, either in-line or in a separate file.

Example
~~~~~~~

Consider the following custom data-type, which will hold the information
to be written to each |MODBUS_DEVICE|.

This example shows the configuration for DataNab
`MBus_WTH_CO2_LCD_ETH <https://github.com/rticommunity/rticonnextdds-gateway/tree/master/examples/modbus/MBus_WTH_CO2_LCD_ETH/MBus_xTH_CO2_LCD_ETH_datasheet.pdf>`__
|MODBUS_DEVICE|. The data-type is defined as follows:

.. code-block:: idl

    enum AlarmControlSetting {
        @value(0)   AUTO,
        @value(128) OFF,
        @value(129) FORCE_PRE_ALARM,
        @value(130) FORCE_CONTINUOUS_ALARM
    };

    enum LCDTemperatureUnit {
        @value(0)   CELSIUS,
        @value(1)   FAHRENHEIT
    };

    @mutable
    struct MBus_WTH_CO2_LCD_ETH_WRITE {
        @optional LCDTemperatureUnit  lcd_units_to_display;
        @optional int16 co2_calibration_offset;
        @optional int16 setpoint_for_co2_pre_alarm;
        @optional int16 setpoint_for_co2_continuous_alarm;
        @optional AlarmControlSetting alarm_control;
        @optional uint16 alarm_sound_on_sec_for_prealarm;
        @optional uint16 alarm_sound_off_sec_for_prealarm;
        uint8  gateway_ip[4];
        uint8  subnet_mask[4];
        uint8  ip_address[4];
    };

Then you can use this data-type to write one or more registers in the
same Modbus server with a single sample.

To configure this output, use the following JSON configuration:

.. code-block:: json

    [
        {
            "field": "lcd_units_to_display",
            "modbus_register_address": 201,
            "modbus_datatype": "HOLDING_REGISTER_INT8",
            "modbus_valid_values": [
                0,
                1
            ]
        },
        {
            "field": "co2_calibration_offset",
            "modbus_register_address": 212,
            "modbus_datatype": "HOLDING_REGISTER_INT16",
            "modbus_min_value": -1000,
            "modbus_max_value": 1000
        },
        {
            "field": "setpoint_for_co2_pre_alarm",
            "modbus_register_address": 213,
            "modbus_datatype": "HOLDING_REGISTER_INT16",
            "modbus_min_value": 0,
            "modbus_max_value": 2000
        },
        {
            "field": "setpoint_for_co2_continuous_alarm",
            "modbus_register_address": 214,
            "modbus_datatype": "HOLDING_REGISTER_INT16",
            "modbus_min_value": 0,
            "modbus_max_value": 2000
        },
        {
            "field": "alarm_control",
            "modbus_register_address": 1246,
            "modbus_datatype": "HOLDING_REGISTER_INT8",
            "modbus_valid_values": [
                0,
                128,
                129,
                130
            ]
        },
        {
            "field": "alarm_sound_on_sec_for_prealarm",
            "modbus_register_address": 1247,
            "modbus_datatype": "HOLDING_REGISTER_INT16",
            "modbus_min_value": 0,
            "modbus_max_value": 20
        },
        {
            "field": "alarm_sound_off_sec_for_prealarm",
            "modbus_register_address": 1248,
            "modbus_datatype": "HOLDING_REGISTER_INT16",
            "modbus_min_value": 0,
            "modbus_max_value": 20
        },
        {
            "field": "ip_address",
            "modbus_register_address": 107,
            "modbus_register_count": 4,
            "modbus_datatype": "HOLDING_REGISTER_INT8",
            "modbus_min_value": 0,
            "modbus_max_value": 255
        },
        {
            "field": "subnet_mask",
            "modbus_register_address": 111,
            "modbus_register_count": 4,
            "modbus_datatype": "HOLDING_REGISTER_INT8",
            "modbus_min_value": 0,
            "modbus_max_value": 255
        },
        {
            "field": "gateway_ip",
            "modbus_register_address": 115,
            "modbus_register_count": 4,
            "modbus_datatype": "HOLDING_REGISTER_INT8",
            "modbus_min_value": 0,
            "modbus_max_value": 255
        }
    ]

The output will be configured as follows:

.. code-block:: xml

    <session name="session">
        <route>
            <route_types>true</route_types>
            <dds_input participant="ModbusParticipant">
                <datareader_qos base_name="AdapterLibrary::ModbusProfile" />
                <topic_name>MBus_WTH_CO2_LCD_ETH_WRITE StreamWriter</topic_name>
                <registered_type_name>MBus_WTH_CO2_LCD_ETH_WRITE</registered_type_name>
            </dds_input>
            <output connection="CO2_Device_1">
                <registered_type_name>MBus_WTH_CO2_LCD_ETH_WRITE</registered_type_name>
                <stream_name>ModbusDevice1_configuration</stream_name>
                <property>
                    <value>
                        <element>
                            <name>configuration_file_json</name>
                            <value>output_config.json</value>
                        </element>
                    </value>
                </property>
            </output>
        </route>
    </session>

Or you can provide the JSON configuration in-line, like this:

.. code-block:: xml

    <session name="session">
        <route>
            <route_types>true</route_types>
            <dds_input participant="ModbusParticipant">
                <datareader_qos base_name="AdapterLibrary::ModbusProfile" />
                <topic_name>MBus_WTH_CO2_LCD_ETH_WRITE StreamWriter</topic_name>
                <registered_type_name>MBus_WTH_CO2_LCD_ETH_WRITE</registered_type_name>
            </dds_input>
            <output connection="CO2_Device_1">
                <registered_type_name>MBus_WTH_CO2_LCD_ETH_WRITE</registered_type_name>
                <stream_name>ModbusDevice1_configuration</stream_name>
                <property>
                    <value>
                        <element>
                            <name>configuration_string_json</name>
                            <value>
                                [
                                    {
                                    "field": "lcd_units_to_display",
                                    "modbus_register_address": 201,
                                    "modbus_datatype": "HOLDING_REGISTER_8",
                                    "modbus_valid_values": [0, 1]
                                    }
                                ]
                            </value>
                        </element>
                    </value>
                </property>
            </output>
        </route>
    </session>


Output Configuration Attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The configuration is a JSON array. Each element contains the attributes
described in the table below:

.. csv-table:: Output Configuration Attributes
   :file: _static/csv/output_configuration_attributes.csv
   :widths: 15, 10, 15, 60
   :header-rows: 1

Behavior
~~~~~~~~

The information provided with the above parameters will be used when
creating the DynamicData and when writing to the |MODBUS_DEVICE|.
If any of the configuration parameters provided are wrong or the data that
they define do not accomplish these restrictions (e.g., |CONF_MODBUS_MIN_VALUE|),
an error will be thrown.

For each element defined in the JSON configuration array, the |RSMODBUS| will check that:

- The member exists.
- The associated type is compatible with the Modbus data-type.

Each instruction results in a synchronous call to the Modbus client API to
write the corresponding data into the |MODBUS_DEVICE|. The following
checks are performed:

- The value is between |CONF_MODBUS_MIN_VALUE| and |CONF_MODBUS_MAX_VALUE| (if provided).
- The value belongs to the values in |CONF_MODBUS_VALID_VALUES| (if provided).

If these checks pass, |CONF_MODBUS_REGISTER_COUNT| registers will be written to
the |MODBUS_DEVICE| with the value or values (if it is an array or sequence) applying
linear transformation as follows:

- |CONF_INPUT_DATA_OFFSET| + |CONF_INPUT_DATA_FACTOR| * <value>

.. note:: If the DynamicData type is an array or a sequence, the linear
          transformation applies to all of them.

.. _section-access-modbus-server:

Access to the Modbus Server
^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are four different functions that can be used to write Modbus coils
and registers. They are distinguished by the **Modbus function
code**. The function code used is controlled by the setting of
|CONF_MODBUS_DATATYPE| as seen in the table below:

.. csv-table:: Access to the Modbus Server
   :file: _static/csv/access_to_modbus_server.csv
   :header-rows: 1

Conversion from DynamicData Fields
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Data is written to Modbus according to the type of the DynamicData field
and the |CONF_MODBUS_DATATYPE|.

**Conversion from DynamicData fields to Modbus registers**

.. csv-table:: Conversion from DynamicData Fields
   :file: _static/csv/conversion_from_dynamic_data_fields.csv
   :widths: 60 40 40
   :header-rows: 1

Floating point values can be stored to pairs of Modbus registers
according to the following formats:

-  “ABCD”: IEEE big endian (usual Modbus format)

-  “DCBA”: IEEE little endian

-  “BADC”: Big endian, byte swapped

-  “CDAB”: Little endian, byte swapped


.. _section-modbus-input:

Modbus Input (Routing Data from Modbus to DDS)
----------------------------------------------

Concept
~~~~~~~

A single Modbus input can be used to read multiple Modbus
registers/coils within the same |MODBUS_DEVICE| and place them into
the same DynamicData object. Therefore the configuration must provide
a way to associate each Modbus register/coil with the corresponding
DyamicData member where the value will be copied into. This is an array
of tuples, similar to the :ref:`section-modbus-output`, using JSON.

Example
~~~~~~~

Consider the following custom data-type, which will hold the information
to be read from each Modbus device.

This example shows the configuration for a DataNab *MBus_WTH_CO2_LCD_ETH*
|MODBUS_DEVICE|. The data-type defined is the following:

.. code-block:: idl

  @mutable
  struct MBus_WTH_CO2_LCD_ETH_INPUT {
      @key      string<64>  device_name; // Configured in the input
      @optional string<64>  device_type;
      uint8  mac_address[6];
      uint8  ip_address[4];
      @optional @unit("celsius") float temperature;
      @optional float humidity;
      @optional float co2_value;
      @optional int8  analog_output_config;
  };


.. code-block:: json

    [
        {
            "field": "device_name",
            "value": "device_37"
        },
        {
            "field": "device_type",
            "value": "MBus_WTH_CO2_LCD_ETH"
        },
        {
            "field": "mac_address",
            "modbus_register_address": 100,
            "modbus_register_count": 6,
            "modbus_datatype": "HOLDING_REGISTER_INT8"
        },
        {
            "field": "ip_address",
            "modbus_register_address": 107,
            "modbus_register_count": 4,
            "modbus_datatype": "HOLDING_REGISTER_INT8",
            "modbus_min_value": 0,
            "modbus_max_value": 255
        },
        {
            "field": "temperature",
            "modbus_register_address": 204,
            "modbus_datatype": "HOLDING_REGISTER_INT16",
            "input_data_factor" : 0.1
        },
        {
            "field": "humidity",
            "modbus_register_address": 207,
            "modbus_datatype": "HOLDING_REGISTER_INT16",
            "input_data_factor" : 0.1
        },
        {
            "field": "co2_value",
            "modbus_register_address": 211,
            "modbus_datatype": "HOLDING_REGISTER_INT16"
        },
        {
            "field": "analog_output_config",
            "modbus_register_address": 1254,
            "modbus_datatype": "HOLDING_REGISTER_INT8"
        }
    ]


The input is configured as follows:

.. code-block:: xml

  <input connection="ModbusDevice1">
      <registered_type_name>
        MBus_WTH_CO2_LCD_ETH_INPUT
      </registered_type_name>
      <stream_name>ModbusDevice1_input</stream_name>
      <property>
          <value>
              <element>
                  <name>polling_period_msec</name>
                  <value>2000</value>
              </element>
              <element>
                  <name>configuration_file_json</name>
                  <value>input_config.json</value>
              </element>
          </value>
      </property>
  </input>

Similar to the output, the JSON configuration can be provided
in-line as in:

.. code-block:: xml

  <input connection="ModbusDevice1">
      <registered_type_name>test</registered_type_name>
      <stream_name>ModbusDevice1_input</stream_name>
      <property>
          <value>
              <element>
                  <name>polling_period_msec</name>
                  <value>2000</value>
              </element>
              <element>
                  <name>configuration_string_json</name>
                  <value>
                  [
                    {
                      "field": "device_type",
                      "value": "MBus_WTH_CO2_LCD_ETH",
                    },
                    {
                      "field": "mac_address",
                      "modbus_register_address": 100,
                      "modbus_register_count": 6,
                      "modbus_datatype": "INPUT_REGISTER_INT8"
                    },
                    …
                  ]
                  </value>
              </element>
          </value>
      </property>
  </input>

Configuration Attributes
~~~~~~~~~~~~~~~~~~~~~~~~

The configuration is a JSON array. Each element contains the attributes
described in the table below:

.. csv-table:: Input Configuration Attributes
   :file: _static/csv/input_configuration_attributes.csv
   :widths: 15, 10, 15, 60
   :header-rows: 1

Behavior
~~~~~~~~

Accessing the Modbus Server
^^^^^^^^^^^^^^^^^^^^^^^^^^^

To read data from a |MODBUS_DEVICE|, the adapter can either use an internal
thread or the Route's session thread.
Which thread to use depends on whether |CONF_POLLING_PERIOD_MSEC| is set:

- If it is set: each period, the adapter will actively read from the
  |MODBUS_SERVER|, save the value, and notify the |RS|.

   - The new data read from the server replaces any previous value
     stored in the input. If |CONF_POLLING_PERIOD_MSEC| is set, the Input
     Adapter read/take operations are **non-blocking**. They just
     return data that has already been read from the Modbus server and
     is kept in the Adapter input, if any.

- If it is not set: the adapter will only read in the context of a
  read/take operation.

   - The read/take operation is **blocking** and synchronously calls
     the |MODBUS_CLIENT| API to read data from the |MODBUS_SERVER| and
     stores the value with the input prior to returning it as the
     output of the read/take call.

There are four different functions that can be used to access
registers, discrete inputs, coils in a |MODBUS_SERVER|. They are distinguished
by the **Modbus function code**. The function code used is
controlled by the setting of |CONF_MODBUS_DATATYPE|, as seen in the table
below:

.. csv-table:: Read Data From Modbus Server
   :file: _static/csv/read_from_modbus_server.csv
   :widths: 20, 20, 60
   :header-rows: 1

Data caching
^^^^^^^^^^^^

The Modbus input keeps at most one data value from the Server. In other
words, it's semantically as if it was storing data samples from a single
instance and had History QoS set to KEEP_LAST with depth=1.

The reason is that, semantically, Modbus looks like memory registers and
those have “KEEP_LAST 1” semantics. So the Modbus input simply caches
the most current value accessed from the corresponding Modbus server
registers.

Read and take behavior
^^^^^^^^^^^^^^^^^^^^^^

The read/take operations return a sequence with one DynamicData sample.

The semantics of read vs. take are the usual ones:

-  With a “read”, the data is not removed from the Adapter input.

-  With a “take”, the data is removed from the Adapter input.

Conversion to DynamicData
^^^^^^^^^^^^^^^^^^^^^^^^^

The configuration of the input dictates where (in the DynamicData
object) to store the registers read from Modbus.

|CONF_MODBUS_REGISTER_COUNT| determines the number of registers that
are read from a |MODBUS_DEVICE|. Then, depending on this setting and the
|CONF_MODBUS_DATATYPE|, the DynamicData object interprets the data that
the read registers contain. The following sections show examples about
how this data is interpreted.

Reading Primitive Values
''''''''''''''''''''''''

The simplest case is when a reading a primitive value is read from Modbus.
Depending on the type of value, this requires reading a single bit coil,
a 16-bit register, two or four consecutive 16-bit registers. The first
register address is specified by the setting |CONF_MODBUS_REGISTER_ADDRESS|.
In this case, the |CONF_MODBUS_REGISTER_COUNT| shall not be specified.

The following table shows how Modbus registers are accessed and
converted to primitive values:

.. _section-conversion-modbus-dynamicdata:
.. csv-table:: Conversion from Modbus Registers to Primitive Value fields
   :file: _static/csv/conversion_modbus_to_dynamicdata.csv
   :widths: 20, 20, 60
   :header-rows: 1

Reading Array/Sequence Values
'''''''''''''''''''''''''''''

It is also possible to read a list of consecutive registers as long as
they are all the same type and are interpreted the same way. This is
achieved by setting |CONF_MODBUS_REGISTER_COUNT|.

If |CONF_MODBUS_REGISTER_COUNT| is specified, then the corresponding
DynamicData field must be an array or sequence of a type compatible
with the |CONF_MODBUS_DATATYPE|. The compatibility rules are the same as
for accessing a primitive field.

For example, consider this mapping:

.. code-block:: xml

    {
        "field": "mac_address",
        "modbus_register_address": 100,
        "modbus_register_count": 6,
        "modbus_datatype": "INPUT_REGISTER_INT8"
    }

In the corresponding definition of MBus_WTH_CO2_LCD_ETH_INPUT,
the field *mac_address* is defined as an int 8 array:

.. code-block:: idl

    struct MBus_WTH_CO2_LCD_ETH_INPUT {
        ...
        @optional int8 mac_address[6];
        ...
    };

In Table :ref:`section-conversion-modbus-dynamicdata`
we see that the |CONF_MODBUS_DATATYPE| INPUT_REGISTER_INT8 is indeed
compatible with all signed integer fields.

If the field being assigned is an array, then, the size must be exactly
what is needed to read the specified number of primitive values. Note that
the size of the array may not match the number of registers, because in some
cases it is necessary to read two registers for each primitive value.

In the case of the *mac_address* we can see from
:ref:`section-conversion-modbus-dynamicdata` that the
|CONF_MODBUS_DATATYPE| INPUT_REGISTER_INT8 only uses one register to hold
the value. Therefore, the size of the array must match
|CONF_MODBUS_REGISTER_COUNT|.

However if we had |CONF_MODBUS_DATATYPE| INPUT_REGISTER_INT32, or
INPUT_REGISTER_FLOAT_ABCD, then each primitive value read (INT32 or
FLOAT_ABCD) requires two Modbus registers. So to read an array of six
elements, we would need to specify 12 Modbus registers. For example,
the following IDL type:

.. code-block:: idl

    struct MBus_WTH_CO2_LCD_ETH_INPUT {
        ...
        @optional float float_array[6];
        ...
    };

Should be configured by using:

.. code-block:: json

    {
        "field": "float_array",
        "modbus_register_address": 2000,
        "modbus_register_count": 12,
        "modbus_datatype": "INPUT_REGISTER_FLOAT_ABCD"
    }

The DynamicData field may also be of a sequence type rather than an
array type. This situation is handled the same way as the array except
that the size of the sequence will be adjusted to match the number of
primitive elements read. So the length will end up being the number of
registers read, except in the case where |CONF_MODBUS_DATATYPE| requires two
or more registers per primitive value (i.e., the 32-bit integers and the
floating-point types). For the configuration to be valid, the maximum length
of the sequence must accommodate this.


.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RS_SEQ2ARRAY_TSFM|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the Sequence2Array Transformation Plugin
=============================================

|RS_SEQ2ARRAY_TSFM| must be registered as a |RS| plugin by using the
``<transformation_plugin>`` tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPluginLib">
            <transformation_plugin name="Sequence2ArrayTransformation">
                <dll>rtisequence2arraytransf</dll>
                <create_function>
                    Sequence2ArrayTransformationPlugin_create_transformation_plugin
                </create_function>
            </transformation_plugin>
        </plugin_library>
    </dds>

.. warning:: |RS| must be able to find the |RS_SEQ2ARRAY_TSFM| dynamic library
             (|SEQ2ARRAY_TRANSFORMATION_LIB_NAME_LINUX| on Linux® systems,
             |SEQ2ARRAY_TRANSFORMATION_LIB_NAME_MAC| on macOS® systems,
             or |SEQ2ARRAY_TRANSFORMATION_LIB_NAME_WIN| on Windows® systems).
             Make sure to include the library's directory in the library search
             path environment variable appropriate for your system
             (``LD_LIBRARY_PATH`` on Linux systems, ``RTI_LD_LIBRARY_PATH`` on
             macOS systems, or ``PATH`` on Windows systems, etc.).

Once the dynamic library and constructor function have been
registered, |RS| will create an instance of the plugin during start-up.

Supported Data Types
====================

|RS_SEQ2ARRAY_TSFM| does not require users to specify the type that the
transformation is using. However, it does require to specify the topic name
as well as the type name. Then, |RS| will handle the input and output Types
automatically. For example:

.. code-block:: xml

     <topic_route name="Sequence2ArrayTestRoute">
        <input participant="myDomain">
            <registered_type_name>MyTypeWithSequences</registered_type_name>
            <topic_name>Sequence2ArrayTopic</topic_name>
        </input>
        <output participant="myDomain">
            <registered_type_name>MyTypeWithArrays</registered_type_name>
            <topic_name>Sequence2ArrayTopicArray</topic_name>
            <transformation plugin_name="MyPluginLib::Sequence2ArrayTransformation"/>
        </output>
    </topic_route>

The snippet above will cause Routing Service to automatically establish a route
whenever it detects a DataWriter using topic ``Sequence2ArrayTopic`` with
registered type name``MyTypeWithSequences``.
The received data will be transformed using the Sequence2Array transformation,
and published to topic ``Sequence2ArrayTopicArray``, which uses type
``MyTypeWithArray``.

When a new route is established, the Sequence2Array transformation will check
compatibility between the input and output types, to make sure that samples may
be converted from one to the other.
In order for types to be compatible, the types must have compatible nested
members, based on the following rules:

- The members are of the same primitive type.
- The members are of two compatible structured types.
- The input member is a collection (array or sequence), the output member is an
  array, and satisfy the following conditions:
  - The elements of both members have compatible types.
  - The input member is a sequence, or the size of the input array is less than
    or equal to the size of the output array.

The type validation is performed recursively until all "leaf" nested members
have been validated, or until at least one member is found to be incompatible.
If the types are found to be incompatible, the Sequence2Array transformation
will throw an error and prevent Routing Service from establishing the new route.

Members are matched based on their names and the value assignation is done by
the position of the member. Therefore, both, the name and position of the
members must be the same.

For example, if we have the following type:

.. code-block:: idl

    enum NewEnum1 {
        ENUM_VALUE_1,
        ENUM_VALUE_2,
        ENUM_VALUE_3
    };

    struct NewStruct1 {
        string member1;
        double member2;
        int8 member3;
        NewEnum1 member4;
    };

        union NewUnion1 switch(NewEnum1) {
        case ENUM_VALUE_1:   uint16 NewMember1;
        case ENUM_VALUE_2:   boolean NewMember2;
        default:   sequence<NewStruct1, 5> NewMember3;
    };

    struct MyTypeWithSequences {
        sequence<NewStruct1> member1;
        sequence<NewEnum1> member2;
        sequence<NewUnion1> member3;
        NewStruct1 member4[2];
        NewStruct1 member5;
        uint64 member6;
        NewEnum1 member7;
    };

A compatible matching type might be:

.. code-block:: idl

    enum NewEnum1 {
        ENUM_VALUE_1,
        ENUM_VALUE_2,
        ENUM_VALUE_3
    };

    struct NewStruct1 {
        string member1;
        double member2;
        int8 member3;
        NewEnum1 member4;
    };

    union NewUnion1Array switch(NewEnum1) {
        case ENUM_VALUE_1:   uint16 NewMember1;
        case ENUM_VALUE_2:   boolean NewMember2;
        default:   NewStruct1 NewMember3[5];
    };

    struct MyTypeWithArrays {
        NewStruct1 member1[5];
        NewEnum1 member2[5];
        NewUnion1Array member3[5];
        NewStruct1 member4[2];
        NewStruct1 member5;
        uint64 member6;
        NewEnum1 member7;
    };

Since sequences are dynamically-sized containers whose actual size might change
at runtime, the Sequence2Array transformation will check, at runtime, that the
size of an array member in the output type is compatible with the current size
of the sequence member in the input.

This allows the input type to make use of both "unbounded sequences"
(e.g. ``sequence<int32> my_int32_sequence``) and sequences whose boundaries
exceed those of the corresponding output array
(e.g. ``sequence<int32, 100> my_bounded_int32_sequence``).

It is the user's responsibility to guarantee that the actual values of any
sequence member will fit in the corresponding output array. If this is not the
case, Sequence2Array will log an error and drop the incompatible sample.

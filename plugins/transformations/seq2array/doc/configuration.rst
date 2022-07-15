.. include:: vars.rst

.. _section-configuration:

*************
Configuration
*************

This section describes how to configure |RS_SEQ2ARRAY_TSFM|.

All configuration is specified in |RS|'s XML configuration file.

.. _section-how-to-load-plugin:

Load the JSON Transformation Plugin
===================================

|RS_SEQ2ARRAY_TSFM| must be registered as a |RS| plugin by using the
``<transformation_plugin>`` tag.

The following snippet demonstrates how to register the plugin in the
``<plugin_library>`` section of |RS|'s XML configuration:

.. code-block:: xml

    <?xml version="1.0"?>
    <dds>
        <plugin_library name="MyPluginLib">
            <transformation_plugin name="Seq2ArrayTransformation">
                <dll>rtiseq2arraytransf</dll>
                <create_function>
                    Seq2ArrayTransformationPlugin_create_transformation_plugin
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

     <topic_route name="Seq2ArrayTestRoute">
        <input participant="myDomain">
            <registered_type_name>NewStruct2</registered_type_name>
            <topic_name>Seq2ArrayTopic</topic_name>
        </input>
        <output participant="myDomain">
            <registered_type_name>NewStruct2Array</registered_type_name>
            <topic_name>Seq2ArrayTopicArray</topic_name>
            <transformation plugin_name="MyPluginLib::Seq2ArrayTransformation"/>
        </output>
    </topic_route>

The snippet above automatically applies the Seq2Array Transformation to all the
types in the topic ``Seq2ArrayTopic`` whose registered name is ``NewStruct2``,
and publish them in the topic ``Seq2ArrayTopicArray`` whose type is
``NewStruct2Array``.

The Seq2Array transformation will check that those types are compatible, meaning
that the types are the same ones but just changing sequences by arrays.

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

    struct NewStruct2 {
        sequence<NewStruct1> member1;
        sequence<NewEnum1> member2;
        sequence<NewUnion1> member3;
        NewStruct1 member4[2];
        NewStruct1 member5;
        uint64 member6;
        NewEnum1 member7;
    };

The compatible matching type will be:

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

    struct NewStruct2Array {
        NewStruct1 member1[5];
        NewEnum1 member2[5];
        NewUnion1Array member3[5];
        NewStruct1 member4[2];
        NewStruct1 member5;
        uint64 member6;
        NewEnum1 member7;
    };

As you can see, when replacing sequences by arrays, an array size must be added.
It is responsibility of the user to assign enough size for storing data that
comes from the sequence. Unbounded sequences are supported, but the actual
size sent should be less than or equal to the array size. If using bounded
sequences, using the bound as array size is recommended.

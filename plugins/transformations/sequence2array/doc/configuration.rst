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

The snippet above automatically applies the Sequence2Array Transformation to all
the types in the topic ``Sequence2ArrayTopic`` whose registered name is
``MyTypeWithSequences``, and publish them in the topic ``Sequence2ArrayTopicArray``
whose type is ``MyTypeWithArrays``.

The Sequence2Array transformation will check that those types are compatible,
meaning that the types are the same ones but just changing sequences by arrays.
This means that the name of inner members and the order of them within the whole
type must be the same (DynamicData ``index``) in the input and the output.
Besides this, these types must be compatible (i.e. they have the same type, or
one is a sequence and the other is an array with the same type of elements).

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

As you can see, when replacing sequences by arrays, an array size must be added.
It is responsibility of the user to assign enough size for storing data that
comes from the sequence. Unbounded sequences are supported, but the actual
size sent should be less than or equal to the array size. If using bounded
sequences, using the bound as array size is recommended. In case that the
number of sequence elements are greater than the array size, an error is logged
and the sample is dropped.

# Integration test

## Sequence2Array Integration test 1

### Description

This integration test consists of 2 different applications, a publisher, and a
subscriber, plus a Routing Service configuration to use the Sequence2Array
transformation.

These applications test that the transformation of the data is done correctly
(replacing sequences by arrays) and the samples are received.

The configuration of the RS transformation is done in
`<RTI Gateway main folder>/plugins/transformations/sequence2array/test/integration_test1/RsSequence2ArrayTransfTest.xml`.
It enables an output transformation for the data (with sequences) sent.

### Behavior

The publisher application will send the following type:

```
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
```

That data will be transformed by the sequence2array transformation into the
following type:

```
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
```

The sequences of the publisher IDL have been modified to arrays with a fixed
size (5).

Both applications won't be compatible because they use different datatypes,
however, when the Routing Service is running with the sequence2array
transformation, the data will be received in the subscriber side.

### How to run Sequence2Array Integration Test 1

In order to run the integration test, three elements are required (all of them
are supposed to be running from `<install dir>/test/tsfm_sequence2array/sequence2array_integration_test1/`
directory):

1. Add the folders which contains `librtisequence2arraytransf` library to
your `LD_LIBRARY_PATH`, `PATH` or `RTI_LD_LIBRARY_PATH` environment variable
(depending on your OS). If you follow these instructions, the folder is:
`<path to rticonnextdds-gateway>/install/lib`.

1. DDS Publisher

    ```sh
    ./publisher/sequence2array_integration_test1_publisher
    ```

1. DDS Subscriber

    ```sh
    ./subscriber/sequence2array_integration_test1_subscriber
    ```

1. Routing Service with the specific configuration

    ```sh
    rtiroutingservice -cfgFile RsSequence2ArrayTransfTest.xml -cfgName RsSequence2ArrayTransfTest
    ```


If you can see data on the subscriber side, the test has passed correctly.

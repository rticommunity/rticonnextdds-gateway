Example Code: AWS IoT Connector
===============================

This example demonstrates how to use *Routing Service* in combination with
the Processor API to integrate *Connext DDS* (or any other technology) with
the Amazon AWS IoT Core platform.

It is assumed that you're familiar with the AWS IoT platform and you have already
an account created and properly set up. For further information, you can refer
to t `Explore AWS IoT Core services in hands-on tutorial
<https://docs.aws.amazon.com/iot/latest/developerguide/iot-gs-first-thing.html>`_

Below there are the instructions to run this example. All the commands
and syntax used assume a Linux/MacOs-based system. If you run this example in a
different architecture, please adapt the commands accordingly.

This example requires building the Python binding plug-in, as explained in the
top-level documentation.

Concept
-------

The integration pattern shown here is a simple as to perform a processing
pipeline to the input streams to produce MQTT messages. The *RoutingService*
configuration simply defines an *AutoRoute* to accept any user input stream
to generate a *Route* that instantiates the *Processor* that will perform
the integration.

The *Processor* is implemented using the Python binding to facilitate the
sample conversion into JSON and to forward the samples to AWS IoT. The
implementation replies on the `AWS IoT SDK
<https://docs.aws.amazon.com/iot/latest/developerguide/iot-sdks.html>`_ to
establish the MQTT connection and publish messages.

As stated above, you will need an account properly configured with a registered
*Thing* and with an associated *Policy* that accepts any possible topic. For
example, for a client ID ``rti_connect_gw``:

.. code-block:: json

    {
        "Version": "2020-10-17",
        "Statement": [
          {
            "Effect": "Allow",
            "Action": [
              "iot:Publish",
              "iot:Receive"
            ],
            "Resource": [
              "arn:aws:iot:us-west-2:2842734449256:topic/*"
            ]
          },
          {
            "Effect": "Allow",
            "Action": [
              "iot:Subscribe"
            ],
            "Resource": [
              "arn:aws:iot:us-west-2:2842734449256:topicfilter/*"
            ]
          },
          {
            "Effect": "Allow",
            "Action": [
              "iot:Connect"
            ],
            "Resource": [
              "arn:aws:iot:us-west-2:2842734449256:client/rti_connext_gw"
            ]
          }
        ]
    }

You will need to create a similar policy and associate it to the registered
*Thing*.


Integration Architecture
^^^^^^^^^^^^^^^^^^^^^^^^

The integration with AWS IoT in this example suggests a 1:1 mapping between
DDS topics and MQTT topics. Indeed, the topic names from both sides are exactly
the same.

The output MQTT messages payload contains a Unicode encoded JSON string. Each
MQTT message maps to one DDS sample, which is converted to generate the following
JSON payload:



Processor properties
^^^^^^^^^^^^^^^^^^^^

The example processor requires input information to successfully establish
communication with an AWS IoT Core. You will need to provide the proper
values for the following properties:

.. list-table:: Plugin Configuration Properties
    :name: TablePluginProperties
    :widths: 30 10 60
    :header-rows: 1

    * - Name (<base_name> = **rti.aws_iot_connector**)
      - Value
      - Description
    * - **<base_name>.endpoint_url**
      - ``<string>``
      - Endpoint URL of the registered *Thing*. You can find URLs in the AWS
        IoT console, under ``Things -> (your thing) -> interact``.
        Required.
        Default: ``a28hekebdxkvd3-ats.iot.us-west-2.amazonaws.com``
    * - **<base_name>.client_id**
      - ``<string>``
      - Client identifier. This is an arbitrary
        value you choose and that shall appear in the policy to allow
        connections.
        Required.
        Default: ``rti_connext_gw``
    * - **<base_name>.cert**
      - ``<string>``
      - Client identifier. This is an arbitrary
        Path to the *Thing* certificate file,
        Required.
        Default: ``rti_connext_gw.cert.pem``
    * - **<base_name>.cert**
      - ``<string>``
      - Client identifier. This is an arbitrary
        Path to the *Thing* private key file,
        Required.
        Default: ``rti_connext_gw.private_key``
    * - **<base_name>.cert**
      - ``<string>``
      - Client identifier. This is an arbitrary
        Path to the *Thing* root CA file,
        Required.
        Default: ``rti_connext_gw.cert.root_ca``

The *Thing* certificates are generated when you register the device and can
download them at time. You can also retrieve them from  the AWS IoT console
under ``Secure -> Certificates``.


Running the Example
-------------------

To run this example you will need an instance of *RTI Shapes Demo* and a
an instance of *RoutingService*.

To run *RoutingService*, you will need first to set up your environment as
follows:

.. code-block:: bash

    export RTI_LD_LIBRARY_PATH=$NDDSHOME/lib/<ARCH>:<RTI_GW_INSTALL_DIR>/lib


where ``<ARCH>`` shall be replaced with the target architecture you built the
the binding plug-in, and ``<RTI_GW_INSTALL_DIR>`` is the directory where you
install the *RTI Connext Gateway*.

Additionally, the Python processor script requires knowing the location of
the Python binding modules for the Processor API. By default it assumes that
these modules are under ``<RTI_GW_INSTALL_DIR>/resource/pyprocessor/modules``.
To specify a different location, you can set the environment variable
``RTI_PROC_MODULE_PATH`` to contain the location of the Python binding modules.
For example:

.. code-block:: bash

    export RTI_LD_LIBRARY_PATH=~/rticonnextdds-gateway/dist/modules


1.  Run one instance of *ShapesDemo* on domain 0. This will be the publisher
    application. Publish any combinations of squares, circles, and triangles,
    and select multiple colors.

3.  Now run *RoutingService* to cause the establish the integration from the
    publisher application to the subscriber application.

    .. code-block:: bash

        $NDDSHOME/bin/rtiroutingservice \
                -cfgFile ../rti_connext_gw.xml \
                -cfgName AwsIotGateway

4. To verify the communication is flowing, go to your AWS IoT console and select
   ``Test``. This will take you to the MQTT console where you can subscribe
   to the topics to monitor. For this example, subscribe to any MQTT topic
   providing ``#``, select ``0`` as QoS, and JSON as payload format. Once you
   subscribe you should the messages arriving to the console:


    .. code-block::

        Circle               <TIMESTAMP>                           Export  Hide
        {
            "data": {
                "color": "BLUE",
                "x": 153,
                "y": 24,
                "shapesize": 30,
                "fillKind": 0,
                "angle": 0
            },
            "info:": {
                "instance_handle": {
                  "valid": 1,
                  "key_hash": "cac217c318363f8ef1160eeedef9e886"
                },
                "publication_handle": {
                  "valid": 1,
                  "key_hash": "0101a2b69a46581a35cc849180000002"
                },
                "sample_state": 2,
                "view_state": 2,
                "instance_state": 1,
                "valid_data": 1,
                "flag": 0,
                "original_publication_virtual_sequence_number": {
                  "low": 44356,
                  "high": 0
                },
                "original_publication_virtual_guid": "0101a2b69a46581a35cc849180000002",
                "related_original_publication_virtual_sequence_number": {
                  "low": 4294967295,
                  "high": -1
                },
                "related_original_publication_virtual_guid": "00000000000000000000000000000000",
                "reception_sequence_number": {
                  "low": 6,
                  "high": 0
                },
                "publication_sequence_number": {
                  "low": 44356,
                  "high": 0
                },
                "reception_timestamp": {
                  "low": 1608086393,
                  "high": 476256999
                },
                "source_timestamp": {
                  "low": 1608086393,
                  "high": 475839999
                }
              }
          }

        ...

   As you can see, a sample is represented in JSON with two main components:
   ``data`` and ``info``, which represent the actual user data and metadata
   portions of the *Topic* samples, respectively.

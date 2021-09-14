# Kafka Shapes - An example integration of DDS and Kafka
This directory contains an example that demonstrates how DDS and Kafka can be integrated to allow DDS applications to access Kafka data and vice versa.

The integration is achieved by deploying RTI Routing Service with the Kafka adapter plugin. The plugin establishes client connetions to Kafka brokers, and to access Kafka data which encapsulates Kafka payload in JSON. 

## Data Types
The example uses the following `ShapeType` as a data type:
```idl
struct ShapeType
{
    string<128> color; //@key
    long x;
    long y;
    long shapesize;
};
```

Kafka applications use JSON to encode data, fololowing the defined data type. An example encoded data looks like:
```json
{
    "color": "BLUE",
    "x": 1,
    "y": 2,
    "shapesize": 10
}
```

The following figure presents the overall architecture of the example scenario. 

![Example Scenario Architecture](example_arch.png "Example Scenario Architecture")

- **DDS DataWriter** publishes DDS data over Connext DDS Databus.
- **DDS DataReader** subscribes to DDS data over Connext DDS Databus. 
- **DDS Input** subscribers to DDS data over Connext DDS Databus and passes the data to Kafka Output.
- **DDS Output** transforms JSON data to DDS from Kafka input and then publishes the data over Connext DDS databus 
- **Kafka Input** subscribes to Kafka data from a Kafka broker and passes the data to DDS Output. 
- **Kafka Output** receives DDS data from DDS Input and converts the data from DDS to JSON data. Then, it will publish the data to a Kafka Broker. 
- **Kafka Broker** exchanges Kafka data between producers (e.g., Kafka Output) and consumers (e.g., Kafka Input). 

## Run Kafka infrastructure components
To run this example, the following Kafka components are required to run. 
- ZooKeeper
- Kafka Broker
- Confluent Control Center (only with Docker Compose option)

## Running RTI Routing Service with the Kafka adapter plugin
We would like to provide instructions with two options to set up Kafka infrastructure components: 1) Script option 2) Docker Compose option 

### Option 1: Running with scripts
1. Get Kafka.
   [Download](https://kafka.apache.org/quickstart) the latest Kafka release and extract it:
    ```sh
    $ tar -xzf kafka_2.13-2.8.0.tgz
    $ cd kafka_2.13-2.8.0

    ```

2. Run Kafka infrastructure components:
   Run the following commands in order to start all services in the correct order:
    ```sh
    # Start the ZooKeeper service
    # Note: Soon, ZooKeeper will no longer be required by Apache Kafka.
    $ bin/zookeeper-server-start.sh config/zookeeper.properties
    ```

   Open another terminal session and run:
    ```sh
    # Start the Kafka broker service
    $ bin/kafka-server-start.sh config/server.properties
    ```
    Once all services have successfully launched, you will have a basic Kafka environment running and ready to use.

3. Create a `Square` topic
   ```sh
   $ bin/kafka-topics.sh --create --topic Square --bootstrap-server localhost:9092
   ```

4. Start an *RTI Shapes Demo* instance:

    ```sh
    $NDDSHOME/bin/rtishapesdemo
    ```

5. Publish a `Square` topic.

    You can create a shapes publisher by clicking in the following
    menu items in the *RTI Shapes Demo* application:

    - "Publish/Square..." and click 'OK' to use the default parameters.

6. In a separate terminal, start an *RTI Routing Service* instance with the example XML configuration:

    ```sh
    $NDDSHOME/bin/rtiroutingservice -cfgFile  kafka_adapter_simple.xml -cfgName dds_kafka_bridge
    ```

7. In a separate terminal, run the console consumer client to see the `Square` topic data from the RTI Gateway.
    ```sh
    $ bin/kafka-console-consumer.sh --topic Square --from-beginning --bootstrap-server localhost:9092
    { "color":"BLUE", "x":120, "y":180, "shapesize":30 } 
    { "color":"BLUE", "x":120, "y":178, "shapesize":30 } 
    { "color":"BLUE", "x":120, "y":176, "shapesize":30 } 

    ```

### Option 2: Running with Docker Compose

1. Get the Docker Compose file. 
   You can get the Docker Compose file provided by Confluent at [this link](https://github.com/confluentinc/cp-all-in-one/tree/6.2.0-post/cp-all-in-one).  

2. Run Kafka infrastructure components:
    Please install [Docker Engine](https://docs.docker.com/engine/install) and [Docker Compose](https://docs.docker.com/compose/install) to run the containers for Kafka. 

    Download the Docker Compose file provided by Confluent.
    ```sh
    $ git clone https://github.com/confluentinc/cp-all-in-one.git
    ```

    Start up the Docker containers for Kafka.

    ```sh
    $ cd cp-all-in-one/cp-all-in-one
    $ docker-compose up -d
    ```
3. After the Kafka services are running, you can acess the Web-based management UI called `Confluent Control Center` at `localhost:9021`.

4. Start an *RTI Shapes Demo* instance:

    ```sh
    $NDDSHOME/bin/rtishapesdemo
    ```

5. Publish a `Square` topic.

    You can create a shapes publisher by clicking in the following
    menu items in the *RTI Shapes Demo* application:

    - "Publish/Square..." and click 'OK' to use the default parameters.

6. In a separate terminal, start an *RTI Routing Service* instance with the example XML configuration:

    ```sh
    $NDDSHOME/bin/rtiroutingservice -cfgFile  kafka_adapter_simple.xml -cfgName dds_kafka_bridge
    ```

7. In Confluent Control Center(localhost:9021), you can see the `Square` topic under the `Topics` tab. After clicking the `Square` topic, You can see the `Square` topic data coming from the RTI Gateway.


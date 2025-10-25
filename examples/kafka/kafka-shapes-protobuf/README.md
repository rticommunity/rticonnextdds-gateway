# Kafka Shapes with Protobuf - An example integration of DDS, Kafka, and Protobuf

This directory contains an example that demonstrates how DDS and Kafka can be
integrated to allow DDS applications to access Kafka data and vice versa.

The integration is achieved by deploying RTI Routing Service with the Kafka
adapter plugin. The plugin establishes client connections to Kafka brokers, and
to access Kafka data which encapsulates Kafka payload using the Protocol Buffers
serialization format.

## Data Types

The example uses the common `ShapeType` defined by `rtishapesdemo`:

```idl
struct ShapeType
{
    string<128> color; //@key
    long x;
    long y;
    long shapesize;
};
```

Kafka applications use Protocol Buffers to encode data, using a data type
equivalent to `ShapeType`:

```protobuf
syntax = "proto3";

message ShapeType {
    string color = 1;
    int32 x = 2;
    int32 y = 3;
    int32 shapesize = 4;
}
```

The following figure presents the overall architecture of the example scenario.

![Example Scenario Architecture](./demo_scenario.png "Example Scenario Architecture")

- **Kafka Publisher**: publishes a random green square every second.
  Shapes are encoded using the Protobuf serialization format, and published through
  the *Kafka Server* on Kafka topic `Square`.

- **Kafka Subscriber**: subscribes to Kafka topic `Square` via the *Kafka Server*.
  Shapes are read as instances of the `ShapeType` Protobuf type, deserialized from
  the Kafka message payload.

- **Kafka Server**: provides the necessary infrastructure to distribute data between
  various Kafka clients.

- **RTI Shapes Demo**: publishes and subscribes to squares on DDS topic `Square`.
  Shapes are encoded as DDS sample of type `ShapeType`.

- **RTI Routing Service**: bridges the *DDS Databus* and the *Kafka Server*, allowing
  data to flow bi-directionaly between applications on each side. The service uses
  three plugins:

  - The built-in **DDS Adapter** plugin, which creates a *DDS DomainParticipant*, and
    other DDS entities required to exchanged data on the *Databus*.

  - The **Kafka Adapter** plugin, which creates a client connection to the *Kafka Server*
    to allow the service to interact with other Kafka applications.

  - The **Protobuf Transformation** plugin, which is configured on the output of each
    route to transform the data between the DDS and (serialized) Protobuf representations.

## Run Kafka infrastructure components

To run this example, the following Kafka components are required to run.

- ZooKeeper (or Kafka using KRaft)
- Kafka Broker
- Confluent Control Center (only with Docker Compose option)

## Running RTI Routing Service with the Kafka adapter plugin

We would like to provide instructions with two options to set up Kafka
infrastructure components: 1) Script option 2) Docker Compose option

In both cases, steps 4 through 7 may be performed automatically inside a `tmux` session
with the help of a script. After starting the Kafka Server, from a terminal:

```sh
cd install/examples/kafka/kafka-shapes-protobuf

./scripts/tmux_session.sh
```

### Option 1: Running with scripts

1. Get Kafka.
   [Download](https://kafka.apache.org/quickstart) the latest Kafka release and
   extract it:

   ```sh
   tar -xzf kafka_2.13-4.0.0.tgz
   cd kafka_2.13-4.0.0
   ```

2. Run Kafka infrastructure components:
   Run the following commands in order to start all services in the correct
   order:

   (Optional, only if using ZooKeeper)

   ```sh
   # Start the ZooKeeper service
   # Note: Soon, ZooKeeper will no longer be required by Apache Kafka.
   bin/zookeeper-server-start.sh config/zookeeper.properties
   ```

   (Optional, if using KRaft)
   Modify `<Kafka installation path>/config/server.properties` by adding
   `controller.quorum.voters=1@localhost:9093` at the end of the file.

   ```sh
   # Initialize metadata log and creates meta.properties
   ./bin/kafka-storage.sh format -t $(./bin/kafka-storage.sh random-uuid) -c config/kraft/server.properties
   ```

   Open another terminal session and run:

   ```sh
   # Start the Kafka broker service
   bin/kafka-server-start.sh config/server.properties
   ```

   Once all services have successfully launched, you will have a basic Kafka
   environment running and ready to use.

3. Create a `Square` topic

   ```sh
   bin/kafka-topics.sh --create --topic Square --bootstrap-server localhost:9092 --partitions 1 --replication-factor 1
   ```

4. Start an *RTI Shapes Demo* instance:

    ```sh
    rtishapesdemo -dataType Shape -pubInterval 1000
    ```

   4.1. Publish a `Square` topic with default parameters (menu "Publish" > "Square" > "OK").

   4.2. Subscribe to `Square` topic with default parameters (menu: "Subscribe" > "Square" > "OK").

5. In a separate terminal, start an *RTI Routing Service* instance with the
    example XML configuration:

    ```sh
    rtiroutingservice -cfgFile shapesdemo_kafka_protobuf.xml -cfgName shapesdemo_bridge
    ```

6. In a separate terminal, run the shapes Kafka subscriber to see the `Square`
   topic data from the RTI Gateway.

    ```sh
    bin/shapes_kafka_subscriber localhost:9092 Square
    ```
  
7. In a separate terminal, run the shapes Kafka publisher to produce data to the `Square`
   Kafka topic.

    ```sh
    bin/shapes_kafka_publisher localhost:9092 GREEN Square
    ```

### Option 2: Running with Docker Compose

1. Get the Docker Compose file.
   You can get the Docker Compose file provided by Confluent at
   [this link](https://github.com/confluentinc/cp-all-in-one/tree/7.9.0-post/cp-all-in-one).

2. Run Kafka infrastructure components:
   Please install [Docker Engine](https://docs.docker.com/engine/install) and
   [Docker Compose](https://docs.docker.com/compose/install) to run the
   containers for Kafka.

    Download the Docker Compose file provided by Confluent.

    ```sh
    git clone https://github.com/confluentinc/cp-all-in-one.git
    ```

    Start up the Docker containers for Kafka.

    ```sh
    cd cp-all-in-one/cp-all-in-one
    docker-compose up -d
    ```

3. After the Kafka services are running, you can access the Web-based management
   UI called `Confluent Control Center` at `localhost:9021`.

4. Start an *RTI Shapes Demo* instance:

    ```sh
    rtishapesdemo -dataType Shape -pubInterval 1000
    ```

   4.1. Publish a `Square` topic with default parameters (menu "Publish" > "Square" > "OK").

   4.2. Subscribe to `Square` topic with default parameters (menu: "Subscribe" > "Square" > "OK").

5. In a separate terminal, start an *RTI Routing Service* instance with the
    example XML configuration:

    ```sh
    rtiroutingservice -cfgFile shapesdemo_kafka_protobuf.xml -cfgName shapesdemo_bridge
    ```

6. In a separate terminal, run the shapes Kafka subscriber to see the `Square`
   topic data from the RTI Gateway.

    ```sh
    bin/shapes_kafka_subscriber localhost:9092 Square
    ```
  
7. In a separate terminal, run the shapes Kafka publisher to produce data to the `Square`
   Kafka topic.

    ```sh
    bin/shapes_kafka_publisher localhost:9092 GREEN Square
    ```

8. In Confluent Control Center(localhost:9021), you can see the `Square` topic
   under the `Topics` tab. After clicking the `Square` topic, You can see the
   `Square` topic data coming from the RTI Gateway.

import os
import sys
import time
import json
from enum import Enum
from typing import Dict
import AWSIoTPythonSDK.MQTTLib as AWSIoTPyMQTT

# Obtain the location of the rti.routing module so can configure the
# path for Python to find it. This is useful if rti.routing is not
# installed as Python package
PROC_MODULE_PATH = os.getenv("RTI_PROC_MODULE_PATH")
if PROC_MODULE_PATH:
    sys.path.append(os.path.abspath(PROC_MODULE_PATH))
else:
    # consider default location wthin the install dir <root_dir>/dist
    sys.path.append(os.path.abspath("../../../dist/modules"))

from rti.routing import proc
from rti.routing import service

class ConfigProperty(Enum):
    ENDPOINT = "rti.aws_iot_connector.endpoint_url"
    CLIENT_ID = "rti.aws_iot_connector.client_id"
    PATH_TO_CERT = "rti.aws_iot_connector.cert"
    PATH_TO_KEY = "rti.aws_iot_connector.private_key"
    PATH_TO_ROOT = "rti.aws_iot_connector.root_ca"

class AwsIotConnector(proc.Processor):
    def __init__(self, route: proc.Route, properties : Dict):
        # get configuration values for MQTT connection

        self.mqtt_connection = AWSIoTPyMQTT.AWSIoTMQTTClient(properties[ConfigProperty.CLIENT_ID.value])
        self.mqtt_connection.configureEndpoint(properties[ConfigProperty.ENDPOINT.value], 8883)
        self.mqtt_connection.configureCredentials(
                properties[ConfigProperty.PATH_TO_ROOT.value],
                properties[ConfigProperty.PATH_TO_KEY.value],
                properties[ConfigProperty.PATH_TO_CERT.value])
        self.mqtt_connection.connect()

    def on_data_available(self, route) -> None:
        for input in route.inputs:
            topic_name = input.info["stream_info"]["stream_name"]
            with input.take() as samples:
                for sample in samples:
                    sample_out = {"data" : sample.data, "info:" : sample.info}
                    self.mqtt_connection.publish(topic_name, json.dumps(sample_out), 0)


def create_processor(route: proc.Route, properties: Dict) -> proc.Processor:
    return AwsIotConnector(route, properties)

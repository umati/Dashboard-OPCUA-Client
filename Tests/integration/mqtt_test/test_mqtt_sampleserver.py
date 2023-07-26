"""
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright 2023 (c) Sebastian Friedl, FVA GmbH / interop4X(for umati and VDW e.V.)
"""

import json
import docker
import time
import unittest
from typing import Any, Optional

import jsonschema
import paho.mqtt.client as mqtt
from parameterized import parameterized


class TestMqttSampleServer(unittest.TestCase):
    client = mqtt.Client()

    # Define the topics and corresponding JSON schema files
    topic_schema_pairs = [
        (
            "BaseMachineTool",
            "umati/v2/umati/mqtt_test/MachineToolType/nsu=http:_2F_2Fexample.com_2FBasicMachineTool_2F;i=66382",
            "schemas/SampleServer/BaseMachineTool.json",
        ),
        (
            "FullWoodworking",
            "umati/v2/umati/mqtt_test/WwMachineType/nsu=http:_2F_2Fexample.com_2FFullWoodworking_2F;i=66382",
            "schemas/SampleServer/FullWoodworking.json",
        ),
        (
            "FullMachineTool",
            "umati/v2/umati/mqtt_test/MachineToolType/nsu=http:_2F_2Fexample.com_2FFullMachineTool_2F;i=66382",
            "schemas/SampleServer/FullMachineTool.json",
        ),
        (
            "BasicGMS",
            "umati/v2/umati/mqtt_test/GMSType/nsu=http:_2F_2Fwww.isw.uni-stuttgart.de_2FBasicGMS_2F;i=66382",
            "schemas/SampleServer/BasicGMS.json",
        ),
        # Add more entries here for more test cases.
    ]


    @classmethod
    def setUpClass(cls):
        """
        Sets up the MQTT client and ensures the Docker container for each topic is ready.
        """
        cls.setup_mqtt_client()

    @classmethod
    def setup_mqtt_client(cls):
        """
        Set up the MQTT client.
        """
        cls.client = mqtt.Client()
        ret = cls.client.connect("localhost", 1883, 60)
        assert ret == 0


    @classmethod
    def wait_for_topic(cls, container_name, topic, wait_time_limit_sec=10, wait_step_sec=5):
        """
        Wait for a single topic to be ready in the Docker container's logs.
        """
        docker_client = docker.from_env()

        container = docker_client.containers.get(container_name)
        next_wait_time =0
        print(topic)
        print (container.logs().decode('utf-8').count(topic))
        while (container.logs().decode('utf-8').count(topic) < 2) and (next_wait_time < wait_time_limit_sec):
            print(f"Waiting for test container to become ready since {next_wait_time}s...")
            time.sleep(wait_step_sec)
            next_wait_time += wait_step_sec
        time.sleep(wait_step_sec)
        
    @classmethod
    def tearDownClass(cls) -> None:
        """
        A class method called after all tests in an individual class have run.
        Here we disconnect the MQTT client connection.
        """
        ret = cls.client.disconnect()
        assert ret == 0

    def test_clientOnline_status(self):
        """Tests if the client online status message is as expected."""
        received_msg = self.receive_message("umati/v2/umati/mqtt_test/clientOnline")
        self.assertEqual(received_msg, b"1")

    @parameterized.expand(
        topic_schema_pairs,
        name_func=lambda f, n, p: f"{f.__name__}_{p.args[0]}",
    )
    def test_message_correctness(self, name: str, topic: str, schema_file: str) -> None:
        """
        This test function tests if the messages sent to the mqtt broker are correct.
        Add more tests:
        - start an valid server, dashboardclient, mqtt Broker
        - copy the json message of the new test machine
        - create json schem from the copied json (you can use https://codebeautify.org/json-to-json-schema-generator)
        - save json schema in schemas folder
        - add a new parameterized (on top of the method) with name, the mqtt topic and json schema path
        """
        
        self.wait_for_topic('mqtt_test-gateway-1', topic)
        
        # Use the helper method to receive the message as JSON
        json_msg = self.receive_message_as_json(topic)

        # Load the JSON schema from a file.
        with open(schema_file, "r") as f:
            schema = json.load(f)

        # Validate the received message against the JSON schema.
        fail = False
        fail_message = ""
        try:
            jsonschema.validate(instance=json_msg, schema=schema)
        except jsonschema.exceptions.ValidationError as e:
            fail = True
            fail_message = str(e)
        if fail == True:
            self.fail(
                f"Message on topic '{topic}' does not match schema: {fail_message}"
            )

    def receive_message(self, topic: str, timeout: int = 10) -> Any:
        """
        This helper function subscribes to a topic and waits for a message or until the timeout.
        It then returns the received message as bytes.

        Parameters:
        - topic: the MQTT topic to subscribe to.
        - timeout: the amount of time in seconds to wait for a message. Default is 10 seconds.

        Returns:
        - The payload of the received MQTT message as bytes.
        """
        received_msg = None

        def on_message(client, userdata, msg):
            nonlocal received_msg
            if msg.topic == topic:
                received_msg = msg.payload

        self.client.on_message = on_message
        self.client.subscribe(topic)
        timeout_time = time.time() + timeout
        while received_msg is None and time.time() < timeout_time:
            self.client.loop()
        return received_msg

    def receive_message_as_json(self, topic: str, timeout: int = 10) -> dict:
        """
        This helper function uses receive_message() to get a message,
        then it decodes and deserializes it from JSON to a Python dictionary and returns it.

        Parameters:
        - topic: the MQTT topic to subscribe to.
        - timeout: the amount of time in seconds to wait for a message. Default is 10 seconds.

        Returns:
        - The payload of the received MQTT message as a Python dictionary.
        """
        received_msg: Optional[bytes] = self.receive_message(topic, timeout)
        if received_msg is not None:
            return json.loads(received_msg.decode("utf-8"))
        else:
            return {}


if __name__ == "__main__":
    unittest.main()

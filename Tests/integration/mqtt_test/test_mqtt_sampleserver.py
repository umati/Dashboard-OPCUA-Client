"""
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright 2023 (c) Sebastian Friedl, FVA GmbH / interop4X(for umati and VDW e.V.)
"""

import json
import time
import unittest
from typing import Any, Optional

import jsonschema
import paho.mqtt.client as mqtt


class TestMqttSampleServer(unittest.TestCase):
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)

    @classmethod
    def setUpClass(cls) -> None:
        """
        A class method called before tests in an individual class are run.
        Here we establish the MQTT client connection.
        """
        ret = None
        for i in range(0, 120):
            try:
                ret = cls.client.connect("localhost", 1883, 60)
            except ConnectionRefusedError:
                print(f"Try to  connect to mqtt broker {i} times")
                time.sleep(1)
        assert ret == 0

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

    def test_BaseMachineTool(self) -> None:
        """
        This test function test if the BaseMachineTool is send correct to the mqtt broker
        """
        # Use the helper method to receive the message as JSON
        topic = "umati/v2/umati/mqtt_test/MachineToolType/nsu=http:_2F_2Fexample.com_2FBasicMachineTool_2F;i=66382"
        json_msg = self.receive_message_as_json(topic)

        # Load the JSON schema from a file.
        with open("schemas/SampleServer/BaseMachineTool.json", "r") as f:
            schema = json.load(f)

        # Validate the received message against the JSON schema.
        # In case of validation errors, the jsonschema.validate() function will raise an exception.
        try:
            jsonschema.validate(instance=json_msg, schema=schema)
        except jsonschema.exceptions.ValidationError as e:
            print(e)
            self.fail(f"Message is of {topic} is not correct!")

    def test_FullMachineTool(self) -> None:
        """
        This test function test if the FullMachineTool is send correct to the mqtt broker
        """
        # Use the helper method to receive the message as JSON
        topic = "umati/v2/umati/mqtt_test/MachineToolType/nsu=http:_2F_2Fexample.com_2FFullMachineTool_2F;i=66382"
        json_msg = self.receive_message_as_json(topic)

        # Load the JSON schema from a file.
        # you can use https://codebeautify.org/json-to-json-schema-generator to generate a schema from a example json
        with open("schemas/SampleServer/FullMachineTool.json", "r") as f:
            schema = json.load(f)

        # Validate the received message against the JSON schema.
        # In case of validation errors, the jsonschema.validate() function will raise an exception.
        try:
            jsonschema.validate(instance=json_msg, schema=schema)
        except jsonschema.exceptions.ValidationError as e:
            print(e)
            self.fail(f"Message is of {topic} is not correct!")

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

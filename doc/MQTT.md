# Local MQTT requirements

## Installing a MQTT Broker

Deploying a MQTT Broker locally. Example with Eclipse Mosquitto:

1. [Download Mosquitto](https://mosquitto.org/download/).
2. Install Mosquitto.
3. Change the content of "mosquitto.conf" (Use any text editor) to:

    1. [https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2](https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2)
    2. "{{MQTT_WS_PORT}}" change for "1884"

4. Start "mosquitto.exe".

5. A console windows appears without any output.

## Installing a MQTT Client

The MQTT Client is used to read the values on the MQTT Broker. This Example uses MQTTX:

1. [Download MQTTX](https://mqttx.app/).
2. Install and start MQTTX.
3. Create new connection via + button with these parameters.
    1. Name: "OPCUA Client"
    2. Host: "mqtt://" "127.0.0.1"
    3. Port: 1883
4. Press Button "Connect".
5. Press Button "New Subscription".
    1. Topic: "umati/#"
    2. Press "Confirm"

# Local MQTT requirements

## Installing a MQTT Client on Windows

The MQTT Client is used to read the values on the MQTT Broker. This Example uses MQTTX:

1. [Download MQTTX](https://mqttx.app/) alternatively [MQTT-Explorer](https://github.com/thomasnordquist/MQTT-Explorer) or [mqttMultimeter](https://github.com/chkr1011/mqttMultimeter)
2. Install and start MQTTX.
3. Create new connection via + button with these parameters.
    1. Name: "OPCUA Client"
    2. Host: "mqtt://" "127.0.0.1"
    3. Port: 1883
4. Press Button "Connect".
5. Press Button "New Subscription".
    1. Topic: "umati/#"
    2. Press "Confirm"

## Installing a MQTT Broker on Windows

Deploying a MQTT Broker locally. Example with Eclipse Mosquitto:

1. [Download Mosquitto](https://mosquitto.org/download/).
2. Check for a running Mosquitto version, if you installed previously
3. Install Mosquitto, with local admin privileges
4. Change the content of `mosquitto.conf` (Use any text editor) to:

    1. [https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2](https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2)
    2. "{{MQTT_WS_PORT}}" change for "1884"

5. Start `mosquitto.exe`.

6. A console windows appears without any output.

## Running a MQTT Broker as a container image

If you want to run a MQTT broker as container image, please make sure you have a container runtime link [Docker Desktop](https://www.docker.com/products/docker-desktop/) installed.

1. Execute `docker run -it -p 1883:1883 eclipse-mosquitto:2.0.15 mosquitto -c /mosquitto-no-auth.conf`

This starts a unauthenticated MQTT broker on your localhost.

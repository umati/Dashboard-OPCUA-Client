# Usage as a testing tool for correct information model instantiation according to a companion specification

Content:
The following documentation describes a use case in which a local server with a companion specification shall be tested
by using a locally deployed getaway and a MQTT Broker.

Requirements:
- A local OPC UA Server with an instance model according to a companion specification, which is supported by the Dashboard-OPCUA-Client. You can find an overview of all supported specification here: [README](https://github.com/umati/Dashboard-OPCUA-Client/blob/development/README.md)
- Admin privileges
- Open port between OPC UA Server, Getaway and MQTT Broker.

## Installing a MQTT Broker

Deploying a MQTT Broker locally. Example with Eclipse Mosquitto:

1. Download Mosquitto ([https://mosquitto.org/download/](https://mosquitto.org/download/)).
2. Install Mosquitto.
3. Change the content of "mosquitto.conf" (Use any text editor) to:

    1. [https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2](https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2)
    2. "{{MQTT_WS_PORT}}" change for "1884"

4. Start "mosquitto.exe".

5. A console windows appears without any output.

## Installing a MQTT Client

The MQTT Client is used to read the values on the MQTT Broker. This Example uses MQTTX:

1. Download MQTTX ([https://mqttx.app/](https://mqttx.app/)).
2. Install and start MQTTX.
3. Create new connection via + button with these parameters.
    1. Name: "OPCUA Client"
    2. Host: "mqtt://" "127.0.0.1"
    3. Port: 1883
4. Press Button "Connect".
5. Press Button "New Subscription".
    1. Topic: "umati/#"
    2. Press "Confirm"

## Executing OPCUA Dashboard-OPCUA-Client

1. Get OPC UA Client Binaries:

    1. Two Possibilities:

        1. Stable version ([https://github.com/umati/Dashboard-OPCUA-Client/releases](https://github.com/umati/Dashboard-OPCUA-Client/releases))

    1. Select newest release.
    2. Download artefact for your operating system.
    3. If older than 4 weeks use alternative option.

    2. Development version ([https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml](https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml))

        1. Select newest workflow run.
        2. Download artifact for your operating system.

3. Unzip Folder.
4. Create configuration for Client.

    1. Within folder "bin" create a file called "configuration.json"
    2. Open "configuration.json" (use any text editor) and insert content from here

        1. [https://github.com/umati/Dashboard-OPCUA-Client/blob/development/configuration.json.example](https://github.com/umati/Dashboard-OPCUA-Client/blob/development/configuration.json.example)
        2. Change constant of OPCUA to your specific local OPC UA implementation

5. Start "DashboardOpcUaClient.exe".

    1. The start-up phase can last for up to 10 minutes, depending of the size of the server. While the start-up phase a '1' is transmitted to the MQTT Broker. The output of the console should look like this:
![Client_Output](https://user-images.githubusercontent.com/105195460/178679686-8a3fc388-ef05-45cd-aeaf-da880036e526.png)

6. After the start up a JSON containing all machine values is transmitted to the MQTT Broker.

<details>
    <summary>Troubleshooting</summary>

Common errors:
1. Missing DLLs
    In case DLLs are missing, those are most likely from the Visual C++ Redistributable package. Those can be downloaded [here](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).

</details>


## Compare errors by hand

1. Open your OPC UA server with a generic client and show all values that shall be transmitted
2. Compare the values from the JSON and the values from the server:

    1. If all values are available and correct within the JSON, the OPC UA Server implementation and the OPC UA instances are correct.
    2. If some values are not correct or are shown at different places, please check the modelling and the naming of the values within the server.

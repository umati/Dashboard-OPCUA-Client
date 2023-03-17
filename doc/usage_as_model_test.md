# Usage as a testing tool for correct information model instantiation according to a companion specification

The following documentation describes a use case in which a local server with a companion specification shall be tested
by using a locally deployed gateway and a MQTT Broker.

## Requirements

- A local OPC UA Server with an instance model according to a companion specification, which is supported by the Dashboard-OPCUA-Client. You can find an overview of all supported specification here: [README](../README.md#tested-companion-specifications)
- Admin privileges
- Open port between OPC UA Server, Getaway and MQTT Broker.

## Deployment

1. Deploy local [MQTT broker](MQTT.md#installing-a-mqtt-broker-on-windows).
2. Prepare the `configuration.json` and configure MQTT according to your local deployment.
3. Deploy the client in [Standalone](./Standalone.md) or [Container](./Configuration.md) mode.
4. Subcribe to your MQTT broker with a [MQTT client](MQTT.md#installing-a-mqtt-client-on-windows)

## Compare errors by hand

1. Open your OPC UA server with a generic client and show all values that shall be transmitted
2. Compare the values from the JSON and the values from the server:

    1. If all values are available and correct within the JSON, the OPC UA Server implementation and the OPC UA instances are correct.
    2. If some values are not correct or are shown at different places, please check the modelling and the naming of the values within the server.
    3. ExtensionObjects might create issues and the parsing logic needs to be adapted. Please open an [issue](https://github.com/umati/Dashboard-OPCUA-Client/issues) pointing to your companion specification, if you run into this.

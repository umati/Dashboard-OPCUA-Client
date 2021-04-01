# Umati Dashboard OPC UA Client
This is the OPC UA client for the [Umati Dashboard](http://umati.app). The client subscribes to the values of one or multiple machine instances in one OPC UA Server and publish them via MQTT in a JSON encoding. All instances are read based on OPC UA ObjectType-definitions. Invalid instances (e.g. missing mandatory nodes) are skipped. Additional nodes aside from the specified nodes are also ignored to ensure a uniform output via MQTT.

## Dependencies
 - [Easylogging](https://github.com/amrayn/easyloggingpp)
 - [JSON for Modern C++](https://github.com/nlohmann/json)
 - [Eclipse Paho](https://www.eclipse.org/paho/index.php) in [C](https://github.com/eclipse/paho.mqtt.c) and [C++](https://github.com/eclipse/paho.mqtt.cpp)
 - [Googletest](https://github.com/google/googletest)
 - [Unified Automation C++ SDK](https://www.unified-automation.com/products/server-sdk/c-ua-server-sdk.html)

## Build
This project uses [cmake](cmake.org/) for building.

## Components
 - [ModelOpcUa](ModelOpcUa) An abstration for OPC UA Type Definitions and OPC UA Types (e.g. NodeId and BrowseName)
 - [DashboardClient](DashboardClient) Read available OPC UA types and check instances and prepare data for publishing. Defines Interfaces for the [OPC UA client](DashboardClient/IDashboardDataClient.hpp) and the [publisher](DashboardClient/IPublisher.hpp).
 - [MachineObserver](MachineObserver) Looks for new machines and publishes a list of all online machines. Uses the interfaces defined by the DashboardClient
 - [MqttPublisher Paho](MqttPublisher_Paho) An implementation of a publisher for MQTT using Eclipse Paho.
 - [OpcUaClient](OpcUaClient) Implementation of an OPC UA client for the Dashboard using Unified Automation C++ SDK
 - [Tests](Tests) Some basic test, mainly for debugging past errors.
 - [Util](Util) General purpose code, e.g. Encoding of machine Ids


# NOTE

The state of the project is not final. There are some parts that needs to be tweaked, especially the security and subscription parts.
There are also quite some memory leaks that have not been adressed yet because we thought about some abstraction layer for memory management like `Open62541Cpp`.
We used the docker image from the [Umati Sampel-Server](https://github.com/umati/Sample-Server). 
The scripts to install each dependency are in the `Tools` folder along with the config we were using.  

Right now, the subscription is disabled by removing the `subscribeValues` function in `DashboardClient/DashboardClient.cpp` on line 44.
Nevertheless the project can be built and run but it will crash after a while:
```
[2021-04-01 15:12:36.584 (UTC+0200)] warn/channel       Connection 8 | SecureChannel 36 | Receiving the response failed with StatusCode BadConnectionClosed
[2021-04-01 15:12:36.584 (UTC+0200)] info/client        Client Status: ChannelState: Closed, SessionState: Created, ConnectStatus: Good
```

# Umati Dashboard OPC UA Client
This is the OPC UA client for the [Umati Dashboard](http://umati.app). The client subscribes to the values of one or multiple machine instances in one OPC UA Server and publish them via MQTT in a JSON encoding. All instances are read based on OPC UA ObjectType-definitions. Invalid instances (e.g. missing mandatory nodes) are skipped. Additional nodes aside from the specified nodes are also ignored to ensure a uniform output via MQTT.

## Dependencies
 - [Easylogging](https://github.com/amrayn/easyloggingpp)
 - [JSON for Modern C++](https://github.com/nlohmann/json)
 - [Eclipse Paho](https://www.eclipse.org/paho/index.php) in [C](https://github.com/eclipse/paho.mqtt.c) and [C++](https://github.com/eclipse/paho.mqtt.cpp)
 - [Googletest](https://github.com/google/googletest)
 - [Open62541](https://open62541.org/)
 - [Open62541Cpp](https://github.com/umati/open62541Cpp)
 - [Python](https://www.python.org/)

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

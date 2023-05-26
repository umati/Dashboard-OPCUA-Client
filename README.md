# umati Dashboard OPC UA Client

This is the OPC UA gateway specifically developed for the umati.app Dashboard.

The client subscribes to values from one or more machine instances on a single OPC UA server and then publishes them using MQTT in a JSON encoding format.

All instances are read based on OPC UA ObjectType-definitions.

Invalid instances (e.g., those missing mandatory nodes) are skipped.

To maintain uniform output via MQTT, additional nodes not specified are also ignored.

## Features

### Tested Companion Specifications

- Flatglass :waning_gibbous_moon:
- Textil Test Devices :waning_gibbous_moon:
- Geometrical Measuring Systems :heavy_check_mark:
- MachineTools :heavy_check_mark:
- PlasticsRubber :heavy_check_mark:
- WoodWorking :heavy_check_mark:
- Robotics  :heavy_check_mark:
- Surface Technology :waning_gibbous_moon:
- Additive Manufacturing DRAFT :waning_gibbous_moon:
- MachineVision Part 2 Release Canidate :waning_gibbous_moon:
- IJT Tightening :waning_gibbous_moon:

### Tested Features

- Typed Objects :heavy_check_mark:
- Objects with InterfaceType :heavy_check_mark:
- Custom DataType with TypeDictionary 1.04 :waning_gibbous_moon:

### Upcoming Features

- Custom DataType  based on DataTypeTypeDefinition

## Usage

## Installation and Configuration

The `Dashboard-OPCUA-Client` can be run either as a standalone application or as a container image.

To install and configure the client, please follow these instructions:

- [Use as standalone](doc/Standalone.md)
- [Use as container](doc/Container.md)
- [Configure your client](doc/Configuration.md)

You can use the client for instance testing or for connecting to the umati.app Dashboard.

### Usage for instance testing

Follow these instructions to use the client as a testing tool for your implementation: [Model Testing usage](doc/usage_as_model_test.md)

### Usage for connecting a server to the dashboard

Follow these instructions to use the client for connecting your local OPC UA Server to the umati.app Dashboard: [umati Dashboard Connection](doc/usage_for_dashboard.md)

## Dependencies

- [Easylogging](https://github.com/amrayn/easyloggingpp)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [Eclipse Paho](https://www.eclipse.org/paho/index.php) in [C](https://github.com/eclipse/paho.mqtt.c) and [C++](https://github.com/eclipse/paho.mqtt.cpp)
- [tinyxml2](https://github.com/leethomason/tinyxml2)
- [Googletest](https://github.com/google/googletest)
- [Open62541](https://open62541.org/)
- [Open62541Cpp](https://github.com/umati/open62541Cpp)
- [Python](https://www.python.org/)

## Build

This project uses [cmake](https://cmake.org/) for building.

### Ubuntu or Debian

The following packages are necessary for building:

- git
- build-essential
- gcc
- g++
- cmake
- python3

```shell
# Clone the repository and initialize the submodules
git clone git@github.com:umati/Dashboard-OPCUA-Client.git
cd Dashboard-OPCUA-Client
git submodule update --init --recursive

# Build the dependencies
cd .github
mkdir build
cd build
cmake ..
make

# Build the Dashboard OPC UA Client
cd ../..
mkdir build
cd build
cmake ..
make

# Adjust configuration regading your setup
cp configuration.json.example configuration.json
nano configuration.json

# Start the Dashboard OPC UA Client
./DashboardOpcUaClient

# Alternatively build everything in one go:
mkdir -p install
mkdir -p build
cd build
cmake ../.github/ -DCMAKE_INSTALL_PREFIX:PATH=<PATH/TO/>Dashboard-OPCUA-Client/install/ -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## Components

- [ModelOpcUa](ModelOpcUa) An abstration for OPC UA Type Definitions and OPC UA Types (e.g. NodeId and BrowseName)
- [DashboardClient](DashboardClient) Read available OPC UA types and check instances and prepare data for publishing. Defines Interfaces for the [OPC UA client](DashboardClient/IDashboardDataClient.hpp) and the [publisher](DashboardClient/IPublisher.hpp).
- [MachineObserver](MachineObserver) Looks for new machines and publishes a list of all online machines. Uses the interfaces defined by the DashboardClient
- [MqttPublisher Paho](MqttPublisher_Paho) An implementation of a publisher for MQTT using Eclipse Paho.
- [OpcUaClient](OpcUaClient) Implementation of an OPC UA client for the Dashboard using Unified Automation C++ SDK
- [Tests](Tests) Some basic test, mainly for debugging past errors.
- [Util](Util) General purpose code, e.g. Encoding of machine IDs

## License

![GitHub](https://img.shields.io/github/license/umati/Dashboard-OPCUA-Client)

Unless otherwise specified, source code in this repository is licensed under the [Mozilla Public License v2.0 (MPLv2)](LICENSE).

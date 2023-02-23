# umati Dashboard OPC UA Client

This is the OPC UA client for the [umati Dashboard](http://umati.app). The client subscribes to the values of one or multiple machine instances in one OPC UA Server and publish them via MQTT in a JSON encoding. All instances are read based on OPC UA ObjectType-definitions.
Invalid instances (e.g. missing mandatory nodes) are skipped. Additional nodes aside from the specified nodes are also ignored to ensure a uniform output via MQTT.

## Dependencies

- [Easylogging](https://github.com/amrayn/easyloggingpp)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [Eclipse Paho](https://www.eclipse.org/paho/index.php) in [C](https://github.com/eclipse/paho.mqtt.c) and [C++](https://github.com/eclipse/paho.mqtt.cpp)
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

## Tested Companion Specifications

- Flatglass :waning_gibbous_moon:
- Geometrical Measuring Systems :waning_gibbous_moon:
- MachineTools :heavy_check_mark:
- PlasticsRubber :heavy_check_mark:
- WoodWorking :heavy_check_mark:
- Robotics  :heavy_check_mark:
- Surface Technology :waning_gibbous_moon:

## Usage for connecting a server to the dashboard

Follow these instructions to use the client to connect your local OPC UA Server to the umati dashboard:
[usage_for_dashboard.md](usage_for_dashboard.md)

## Usage for instance testing

Follow these instructions to use the client as a testing tool for your implementation:
[usage_as_model_test.md](usage_as_model_test.md)

## License

![GitHub](https://img.shields.io/github/license/umati/Dashboard-OPCUA-Client)

Unless otherwise specified, source code in this repository is licensed under the [Mozilla Public License v2.0 (MPLv2)](LICENSE).

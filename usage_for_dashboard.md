# Usage as a client to connect a locally deployed OPC UA server to the umati dashboard

Requirements:

- A local OPC UA Server with a instance model according to a companion specification, which is supported by the Dashboard-OPCUA-Client. You can find an overview of all supported specification here: [README](https://github.com/umati/Dashboard-OPCUA-Client/blob/development/README.md)
- Admin privileges
- Open port to the OPC UA Server

## Executing of Dashboard-OPCUA-Client

1. Get OPC UA Client Binaries:

    1. Two Options:

        1. [https://github.com/umati/Dashboard-OPCUA-Client/releases](https://github.com/umati/Dashboard-OPCUA-Client/releases)

            1. Select newest release and download
            2. If older than 4 weeks use alternative option:

        2. [https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml](https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml)

            1. Select newest workflow run:
![Client_findBinaryFiles](https://user-images.githubusercontent.com/105195460/178679784-acf99801-94e2-44e0-a0b2-0a8378ffba05.png)
            2. Download artefact for your operating system

2. Unzip Folder
3. Create configuration for Client

    1. Within folder `bin` create a file called `configuration.json`
    2. Open `configuration.json` (use any text editor) and insert content from here

        1. [https://github.com/umati/Dashboard-OPCUA-Client/blob/development/configuration.json.example](https://github.com/umati/Dashboard-OPCUA-Client/blob/development/configuration.json.example)
        2. Explanation of the JSON:
          1. ObjectTypeNamespaces: Need to contain all Namespaces, which are also present in your OPC UA Server
          2. NamespaceInformation: Contains the Type Definitions the Client will be looking for and he will continously monitor.
          3. OpcUa: Need to contain the endpoint information of your OPC UA Server and if required the logging credentials
          4. Mqtt: The Content of these keypair values will be provided by the umati team and describe the endpoint of an Mqtt Broker which relays the information to the dashboard.

4. Start `DashboardOpcUaClient.exe`

    1. If browser was used a console comes up. The output should look like this:
![Client_Output](https://user-images.githubusercontent.com/105195460/178679686-8a3fc388-ef05-45cd-aeaf-da880036e526.png)
    2. This start up phase can last for up to 10 minutes
    3. While this phase a `1` is transmitted to the MQTT Client

5. After the start up a JSON containing all machine values is transmitted to the client

## Compare errors by hand

1. Open your OPC UA server with a generic client and show all values that shall be transmitted
2. Compare the values from the JSON and the values from the server:

    1. If all values are available and correct within the JSON, the OPC UA Server implementation and the OPC UA instances are correct.
    2. If some values are not correct or are shown at different places, please check the modelling and the naming of the values within the server

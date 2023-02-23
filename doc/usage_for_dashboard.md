# Usage as a client to connect a locally deployed OPC UA server to the umati dashboard

Requirements:

- A local OPC UA Server with an instance model according to a companion specification, which is supported by the Dashboard-OPCUA-Client. You can find an overview of all supported specification here: [README](../README.md)
- Admin privileges
- Internet connection for the Dashboard-OPCUA-Client and a open Port between the Client and the OPC UA Server

## Executing of Dashboard-OPCUA-Client on Windows

1. Get OPC UA Client Binaries:

    Two Possibilities:
    1. Stable version from [Releases](https://github.com/umati/Dashboard-OPCUA-Client/releases)

        1. Select newest release.
        2. Download artefact for your operating system.
        3. If older than 4 weeks use alternative option.

    2. Development version from latest [CI](https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml) run

        1. Select newest workflow run.
        2. Download artifact for your operating system.

2. Unzip Folder
3. Create configuration for Client

    1. Within folder `bin` create a file called `configuration.json`
    2. Open `configuration.json` (use any text editor) and insert content from here

        1. [configuration.json.example](../configuration.json.example)
        2. Explanation of the JSON:
            1. ObjectTypeNamespaces: Need to contain all Namespaces, which are also present in your OPC UA Server
            2. NamespaceInformation: Contains the Type Definitions the Client will be looking for and he will continuously monitor.
            3. OpcUa: Need to contain the endpoint information of your OPC UA Server and if required the logging credentials
            4. Mqtt: The Content of these keypair values will be provided by the umati team and describe the endpoint of an Mqtt Broker which relays the information to the dashboard.

4. Start `DashboardOpcUaClient.exe`

    1. If browser was used a console comes up. The output should look like this:
![Client_Output](https://user-images.githubusercontent.com/105195460/178679686-8a3fc388-ef05-45cd-aeaf-da880036e526.png)
    2. This start-up phase can last for up to 10 minutes
    3. While this phase a `1` is transmitted to the MQTT Client

5. After the start up a JSON containing all machine values is transmitted to the client

<!-- markdownlint-disable MD033 -->
<details><summary>Troubleshooting</summary>

Common errors:

1. Missing DLLs
    In case DLLs are missing, those are most likely from the Visual C++ Redistributable package. Those can be downloaded [here](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).

</details>

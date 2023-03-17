# Executing Standalone Dashboard-OPCUA-Client

1. Get OPC UA Client Binaries:

    Two Possibilities:
    1. Stable version from [Releases](https://github.com/umati/Dashboard-OPCUA-Client/releases)

        1. Select `latest` release.
        2. Download artefact for your operating system.
        3. If older than 4 weeks use alternative option.

    2. Development version from latest [CI](https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml) run

        1. Select newest workflow run.
        2. Download artifact for your operating system.

2. Unzip Folder
    1. Linux: Install `.deb` package with `sudo dpkg -i dashboardopcuaclient_*_*.deb`
3. Create configuration for Client on Windows:
    1. Within folder `bin` create a file called `configuration.json`
4. Create configuration for Client on Linux:
    1. Create a file `configuration.json` in your home directory
5. Adapt configuration
    1. Open `configuration.json` (use any text editor) and insert content from here
        1. [configuration.json.example](../configuration.json.example)
        2. Change OPC UA and MQTT endpoint information according to [Configuration](./Configuration.md)

6. Start on Windows `DashboardOpcUaClient.exe`.

7. Start on Linux `DashboardOpcUaClient configuration.json` from your home directory

    1. The start-up phase can last for up to 10 minutes, depending of the size of the server. During the start-up phase a `1` is transmitted to the MQTT Broker on the topic `TODO`. The output of the console should look like this:
![Client_Output](sample-log.png)

    2. After the start up a JSON containing all machine values is transmitted to the MQTT Broker. `TODO: Topic`

<!-- markdownlint-disable MD033 -->
<details><summary>Troubleshooting</summary>

## Common errors

1. Missing DLLs
    In case DLLs are missing, those are most likely from the Visual C++ Redistributable package. Those can be downloaded [here](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).

</details>

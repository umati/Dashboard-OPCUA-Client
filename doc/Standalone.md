# Executing Standalone Dashboard-OPCUA-Client

1. Obtain the OPC UA Client Binaries:
There are two options for obtaining the binaries:
    1. Download the stable version from [Releases](https://github.com/umati/Dashboard-OPCUA-Client/releases)

        1. Select `latest` release.
        2. Download artefact for your operating system.
           - use `windows-2022` for most versions of Win 10 and Win 11,(only older versions need `windows-2019` (see [#477](https://github.com/umati/Dashboard-OPCUA-Client/issues/477) for details)
        3. If older than 4 weeks use alternative option.

    2. Download the development version from the latest [CI](https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml) run

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

    1. The start-up phase can last for up to 10 minutes, depending of the size of the server. When the client is connected to the MQTT broker a `1` is transmitted to the MQTT Broker on the topic `umati/v2/<CompanyName>/<ClientName>/clientOnline`. The output of the console should look like this:
![Client_Output](sample-log.png)

    2. After the start up a JSON containing all machine values is published on the MQTT Broker at topic `umati/v2/<CompanyName>/<ClientName>/<SpecificationType>/<EscapedMachineNodeId>` (e.g. `umati/v2/ISW/ClientSampleServer/MachineToolType/nsu=http:_2F_2Fexample.com_2FBasicMachineTool_2F;i=66382`)

## Automatic restarting on Windows

To monitor and restart the client automatically, please use the provided [restart.ps1](../Tools/restart.ps1).
You need to modifiy the path to the `DashboardOpcUaClient.exe` to use it.

```shell
# Add own path to the executable
$ProcessPath = "C:\work\umati\UmatiDashboardOpcUaClient-Release-windows-2022-amd64\bin"
```

## Installing the `debug` version

For the Debug version also the DLLs of Debugging are nessasry:

- C++ 2013: <https://aka.ms/highdpimfc2013x86enu>
- C++ 2022: <https://aka.ms/vs/17/release/vc_redist.x86.exeWindows10>
- SDK: <https://go.microsoft.com/fwlink/p/?linkid=2196241>

## Troubleshooting

[see Troubleshooting](./Troubleshooting.md)

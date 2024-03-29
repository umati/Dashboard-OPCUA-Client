# Usage as a client to connect a locally deployed OPC UA server to the umati dashboard

## Requirements

- A local OPC UA Server with an instance model according to a companion specification, which is supported by the Dashboard-OPCUA-Client. You can find an overview of all supported specification here: [README](../README.md)
- Admin privileges
- Internet connection for the Dashboard-OPCUA-Client and a open Port between the Client and the OPC UA Server
- Credentials for connecting to the umati.app MQTT broker, please register at [info@umati.org](mailto:info@umati.org) to obtain those.

## MQTT Configuration for umati.app enviroment

```json
"Mqtt": {
    "Hostname": "staging.umati.app", // MQTT Broker umati.app for Prod or staging.umati.app for staging environment
    "Port": 443,
    "Username": "<SUPPLIED-USERNAME>", // This is formatted like this: <CompanyName>/<ClientName>
    "Password": "<SUPPLIED-PASSWORD",
    "Prefix": "umati/v2",
    "ClientId": "<SUPPLIED-ClientID>", // Identical to Username on umati.app
    "Protocol": "wss"
}
```

The credentials and and clientId will be provided by umati.app administration.

## MQTT broker endpoint of umati.app

```text
Hostname: staging.umati.app or umati.app
Port: 443
Path: /ws
Protocol: wss
Topic: umati/v2/#
Username: umatiApp
Password: _request at umati team_
```

## Deployment

1. Prepare the `configuration.json` MQTT part according to above information and supplied information
2. Deploy the client in [Standalone](./Standalone.md) or [Container](./Configuration.md) mode.
3. Subscribe to the MQTT broker of the `umati.app` environment and check that data is published.

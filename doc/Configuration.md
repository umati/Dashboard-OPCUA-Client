# Configuration options

The example `configuration.json` options are explained:

```jsonc
{
  "MachinesFilter": [
    {
      "Uri": "http://example.com/BasicMachineTool/",
      "Id": "i=66382",
      "$comment": "BasicMachineTool"
    }
  ],  
// MachineFilter enables to filter for specific instances in a server. Add Uri, NodeId and comment 
// The MachineFilter can be an empty array if all machine should be added to the dashboard e.g.
// "MachinesFilter": [],

  "ObjectTypeNamespaces": [
    "http://opcfoundation.org/UA/",
    "http://opcfoundation.org/UA/IA/",
    "http://opcfoundation.org/UA/DI/",
    "http://opcfoundation.org/UA/Machinery/",
    "http://opcfoundation.org/UA/MachineTool/"

  ], // ObjectTypeNamespaces configures the companion specifications the client creates it's Typemap from.
  "NamespaceInformations": [
    {
      "Namespace": "http://opcfoundation.org/UA/MachineTool/",
        "Types": [
        {
          "Uri": "http://opcfoundation.org/UA/MachineTool/",
          "Id": "i=13",
          "$comment": "MachineToolType" // Defines the TypeDefinition of instances the client will be looking for and he will continuously monitor.
        }
      ],
      "IdentificationType": {
        "Uri": "http://opcfoundation.org/UA/MachineTool/",
        "Id": "i=11",
        "$comment": "MachineToolIdentificationType" // Defines the IdentificationType of this instance to look for the identification information

      }
    }
  ]
  ,
  "OpcUa": {
    "Endpoint": "opc.tcp://localhost:4840", // OPC UA Endpoint to connect to
    "Username": "",
    "Password": "",
    "Security": 1, // 1 plain, 3, Sign&Encrypt
    "ByPassCertVerification": true // If you are using Sign&Encrypt, you must disable certificate verification with this option
  },
  "Mqtt": {
    "Hostname": "localhost", // MQTT Broker
    "Port": 443, // Port to connect to the broker 1883 for tcp, 8883 for TLS or 443 for wss
    "Username": "MyCompany/ClientName", 
    "Password": "",
    "Prefix": "umati/v2", // Topic prefix
    "ClientId": "MyCompany/ClientName", // ClientId part of topic structure
    "Protocol": "wss", // tcp: plain; tls: TLS secured; wss: WebSocket TLS secured
    "CaCertPath":"", // path to the CA-Cert directory, only to be set if advised
    "CaTrustStorePath": "", // path to the CA-Cert file, only to be set if advised
    "HttpProxy": "http://<ip>:<port>", // Optional, Explicitly configure a proxy for http:// or https:// or both
    "HttpsProxy": "https://<ip>:<port>" 
  }
}
```

## HTTPS/HTTP Proxy Working Example

- `configuration.json`:

  ```json
    "Mqtt": {
        "Hostname": "umati.app",
        "Port": 443,
        "Username": "org/user",
        "Password": "",
        "Prefix": "umati/v2",
        "ClientId": "org/user",
        "Protocol": "wss",
        "HttpProxy": "http://proxy.myproxy.net:8082",
        "HttpsProxy": "http://proxy.myproxy.net:8082"  
    }
  ```

- `docker-compose.yaml`:

  ```yaml
  services:
  umati-gateway:
    image: ghcr.io/umati/dashboard-opcua-client:wss-proxy
    container_name: umati-gateway
    volumes:
      - ./config/configuration.json:/app/configuration.json
    environment:
      - http_proxy=proxy.myproxy.net:8082
      - https_proxy=proxy.myproxy.net:8082
  ```

- `docker run` command:
  
  ```sh
  docker run  \
    --name client \
    --rm -it \
    -v <path>/<to>configuration.json:/app/configuration.json \
    -e http_proxy=proxy.myproxy.net:8082 \
    -e https_proxy=proxy.myproxy.net:8082 \
    ghcr.io/umati/dashboard-opcua-client:wss-proxy
  ```

- `docker run` command for `WssTroubleshooter`:

  Start container with

  ```sh
  docker run  \
    --name client \
    --rm -it \
    -v <path>/<to>configuration.json:/app/configuration.json \
    -e http_proxy=proxy.myproxy.net:8082 \
    -e https_proxy=proxy.myproxy.net:8082 \
    --entrypoint "/bin/sh" \
    ghcr.io/umati/dashboard-opcua-client:wss-proxy
  ```

  Then run `# ./WssTroubleshooter` inside container.
  If the connection is working, you should see a connection successful message inside the logs.
  

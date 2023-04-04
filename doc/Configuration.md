# Configuration options

The example `configuration.json` options are explained:

``` JSON
{
  "MachineCacheFile": "MachineCache_datahub.json", // Location of the MachineCacheFile used by the client
  "MachinesFilter": [
    {
      "Uri": "http://example.com/BasicMachineTool/",
      "Id": "i=66382",
      "$comment": "BasicMachineTool"
    }
  ],  // MachineFilter enables to filter for specific instances in a server. Add Uri, NodeId and comment 


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
    "Port": 1883, // Port to conenect to
    "Username": "opcua_client", 
    "Password": "",
    "Prefix": "umati", // Topic prefix
    "ClientId": "umati", // ClientId part of topic structure
    "Protocol": "tcp", // tcp: plain; tls: TLS secured; wss: WebSocket TLS secured
    "CaCertPath":"", // path to the CA-Cert file, only to be set if advised
    "CaTrustStorePath": "" // path to the CA-Cert file, only to be set if advised
  }
}
```

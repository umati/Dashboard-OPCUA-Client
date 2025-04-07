# umati Dashboard

Documentation of the topic and payload for `umati/v2/`

Gateway: <https://github.com/umati/Dashboard-OPCUA-Client/tree/development>
Server: <https://github.com/umati/Sample-Server/blob/develop/MachineTools/ShowcaseMachineTool.cpp>
OPC UA Machine instance: NamespaceUri <http://example.com/ShowcaseMachineTool/>

## Topics and payload

### Overall topic

`umati/v2/#`

### Company and client spefic topic

`umati/v2/<company>/<client>/`
no payload

### Online detection topic

`umati/v2/<company>/<client>/clientOnline`
Payload: 1 or 0

### Gateway software version

`umati/v2/<company>/<client>/gw-version`
Payload: version string

### Online detection per machine instance of a server

`umati/v2/<company>/<client>/online`
No payload

#### Online detection per machine instance

`umati/v2/<company>/<client>/online/nsu=http:_2F_2Fexample.com_2FShowcaseMachineTool_2F;i=66382`
sub topic is the NamespaceUri of the instance in HTML encoding
Payload: 1 or 0
Heartbeat 90 sec
Description: allows the online/offline detection of a single machine instance

### ObjectType topic

`umati/v2/<company>/<client>/<OPC ObjectTypedefinition name>`
e.g., `umati/v2/<company>/<client>/MachineToolType`
Description: appears per configured or detected ObjectType definiton

#### ObjectType per machine instance

`umati/v2/<company>/<client>/MachineToolType/nsu=http:_2F_2Fexample.com_2FShowcaseMachineTool_2F;i=66382`
Payload: JSON object - sample content see below

### Machine list topic

`umati/v2/<company>/<client>/list/<OPC ObjectTypedefinition name>`
e.g., `umati/v2/<company>/<client>/list/MachineToolType`
Payload: List of all machine instances with identification

### Error list

`umati/v2/<company>/<client>/bad_list/errors`
Payload: JSON object
Description: Contains a list of machine which are not correctly instanciated/publisch

## Payload MachineToolType list topic

```json
[
  //... shortend
  {
    "Data": {
      "DeviceClass": "Machining centre (other)",
      "Location": "INNOTEQ 3.2 B12/VIRTUAL 1 1/N 49.871215 E 8.654204",
      "Manufacturer": {
        "locale": "",
        "text": "umati Showcase"
      },
      "Model": {
        "locale": "",
        "text": "ShowcaseMachineTool"
      },
      "ProductCode": "2653837gg1548",
      "ProductInstanceUri": "https://showcase.umati.org/Specs/Machinetools.html",
      "SerialNumber": "2021-15360620311222485159",
      "SoftwareRevision": "v1.02.1",
      "YearOfConstruction": 2021
    },
    "MachineId": "nsu=http:_2F_2Fexample.com_2FShowcaseMachineTool_2F;i=66382",
    "ParentId": "nsu=http:_2F_2Fopcfoundation.org_2FUA_2FMachinery_2F;i=1001",
    "Topic": "umati/v2/vdw/staging1/MachineToolType/nsu=http:_2F_2Fexample.com_2FShowcaseMachineTool_2F;i=66382",
    "TypeDefinition": "MachineToolType"
  }
  // ... shortend
]
```

## Sample Payload ShowcaseMachineTool

```JSON
{
  "Equipment": {
    "Tools": {
      "<Tool>": {
        "Tool1": {
          "$TypeDefinition": "nsu=http://opcfoundation.org/UA/MachineTool/;i=50",
          "ControlIdentifier1": 12,
          "ControlIdentifierInterpretation": 0,
          "Locked": {
            "properties": {
              "ReasonForLocking": 5
            },
            "value": false
          },
          "Name": "Tool1"
        }
      },
      "NodeVersion": ""
    }
  },
  "Identification": {
    "DeviceClass": "Machining centre (other)",
    "Location": "INNOTEQ 3.2 B12/VIRTUAL 1 1/N 49.871215 E 8.654204",
    "Manufacturer": {
      "locale": "",
      "text": "umati Showcase"
    },
    "Model": {
      "locale": "",
      "text": "ShowcaseMachineTool"
    },
    "ProductCode": "2653837gg1548",
    "ProductInstanceUri": "https://showcase.umati.org/Specs/Machinetools.html",
    "SerialNumber": "2021-15360620311222485159",
    "SoftwareRevision": "v1.02.1",
    "YearOfConstruction": 2021
  },
  "Monitoring": {
    "<MonitoredElement>": {
      "Channel 1": {
        "$TypeDefinition": "nsu=http://opcfoundation.org/UA/MachineTool/;i=16",
        "ChannelMode": 0,
        "ChannelState": 1,
        "FeedOverride": {
          "properties": {
            "EURange": {
              "High": 110.0,
              "Low": 0.0
            },
            "EngineeringUnits": {
              "Description": {
                "locale": "",
                "text": ""
              },
              "DisplayName": {
                "locale": "",
                "text": "%"
              },
              "NamespaceUri": "",
              "UnitId": 0
            }
          },
          "value": 98.75
        },
        "Name": "Channel 1"
      },
      "Spindle": {
        "$TypeDefinition": "nsu=http://opcfoundation.org/UA/MachineTool/;i=22",
        "IsRotating": true,
        "IsUsedAsAxis": false,
        "Name": "Spindle",
        "Override": {
          "properties": {
            "EURange": {
              "High": 125.0,
              "Low": 0.0
            },
            "EngineeringUnits": {
              "Description": {
                "locale": "",
                "text": ""
              },
              "DisplayName": {
                "locale": "",
                "text": "%"
              },
              "NamespaceUri": "",
              "UnitId": 0
            }
          },
          "value": 103.0
        }
      }
    },
    "MachineTool": {
      "OperationMode": 1,
      "PowerOnDuration": 9
    },
    "Stacklight": {
      "<OrderedObject>": {
        "Light 0": {
          "$TypeDefinition": "nsu=http://opcfoundation.org/UA/IA/;i=1006",
          "IsPartOfBase": false,
          "NumberInList": 0,
          "SignalColor": 1,
          "SignalMode": 0,
          "SignalOn": true
        },
        "Light 1": {
          "$TypeDefinition": "nsu=http://opcfoundation.org/UA/IA/;i=1006",
          "IsPartOfBase": false,
          "NumberInList": 1,
          "SignalColor": 4,
          "SignalMode": 0,
          "SignalOn": true
        },
        "Light 2": {
          "$TypeDefinition": "nsu=http://opcfoundation.org/UA/IA/;i=1006",
          "IsPartOfBase": false,
          "NumberInList": 2,
          "SignalColor": 2,
          "SignalMode": 0,
          "SignalOn": false
        }
      },
      "NodeVersion": "",
      "StacklightMode": 0
    }
  },
  "Production": {
    "ActiveProgram": {
      "Name": "Basic Program",
      "NumberInList": 0,
      "State": {
        "CurrentState": {
          "properties": {
            "Id": 138,
            "Number": 1
          },
          "value": {
            "locale": "en",
            "text": "Running"
          }
        }
      }
    }
  }
}
```

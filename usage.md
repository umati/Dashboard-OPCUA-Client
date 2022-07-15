# Usage as an instance control

Requirements:

-   Admin privileges
-   Open port to the OPC UA Server

## Installing a MQTT Broker:

Example with Eclipse Mosquitto

1.  Download Mosquitto

2.  [https://mosquitto.org/download/](https://mosquitto.org/download/)

3.  Install Mosquitto
4.  Change the content of "mosquitto.conf" (Use any text editor) to:

	1.  [https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2](https://github.com/umati/infrastructure-Dashboard/blob/main/config-templates/mosquitto/mosquitto.conf.jinja2)
	2.  "{{MQTT_WS_PORT}}" change for "1884"

5.  Start "mosquitto.exe"

6.  A console windows appears without any output

## Installing a MQTT Client:

Example with MQTTX

1.  Download MQTTX:
	1.  [https://mqttx.app/](https://mqttx.app/)

2.  Install and start MQTTX
3.  Create new connection via button with these inputs
	1.  Name: "OPCUA Client"
	2.  Host: "mqtt://" "127.0.0.1"
	3.  Port: 1883

![MQTT_Client](https://user-images.githubusercontent.com/105195460/178679858-6de3771b-7976-4d5e-ae0f-e34d25880fa1.png)

4.  Press Button "Connect"
5.  Press Button "New Subscription"
	1.  Topic: "umati/#"
	2.  Press "Confirm"

## Installing OPCUA Client:

1.  Get OPC UA Client Binaries:

	1.  Two Options:

		1.  [https://github.com/umati/Dashboard-OPCUA-Client/releases](https://github.com/umati/Dashboard-OPCUA-Client/releases)

			1.  Select newest release and download
			2.  If older than 4 weeks use alternative option:

		2.  [https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml](https://github.com/umati/Dashboard-OPCUA-Client/actions/workflows/build.yml)

			1.  Select newest workflow run:
![Client_findBinaryFiles](https://user-images.githubusercontent.com/105195460/178679784-acf99801-94e2-44e0-a0b2-0a8378ffba05.png)
			2.  Download artifact for your operating system

2.  Unzip Folder
3.  Create configuration for Client

	1.  Within folder "bin" create a file called "configuration.json"
	2.  Open "configuration.json" (use any text editor) and insert content from here

		1.  [https://github.com/umati/Dashboard-OPCUA-Client/blob/development/configuration.json.example](https://github.com/umati/Dashboard-OPCUA-Client/blob/development/configuration.json.example)
		2.  Change constant of OPCUA to your specific local OPC UA implementation

4.  Start "DashboardOpcUaClient.exe"

	1.  If browser was used a console comes up. The output should look like this:
![Client_Output](https://user-images.githubusercontent.com/105195460/178679686-8a3fc388-ef05-45cd-aeaf-da880036e526.png)
	2.  This start up phase can last for up to 10 minutes
	3.  While this phase a "1" is transmittet to the MQTT Client

5.  After the start up a JSON containing all machine values is transmitted to the client

## Compare errors by hand:

1.  Open your OPC UA server with a generic client and show all values that shall be transmitted
2.  Compare the values from the JSON and the values from the server:

	1.  If all values are available and correct within the JSON, the OPC UA Server implementation and the OPC UA instances are correct.
	2.  If some values are not correct or are shown at different places, please check the modelling and the naming of the values within the server

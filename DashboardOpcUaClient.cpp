#include "OpcUaClient/OpcUaClient.hpp"
#include <DashboardClient.hpp>
#if defined(PUBLISHER_REDIS)
#include <RedisPublisher.hpp>
#endif
#if defined(PUBLISHER_MQTT_MOSQUITTO)
#include <MqttPublisher.hpp>
#endif

#if defined(PUBLISHER_MQTT_PAHO)
#include <MqttPublisher_Paho.hpp>
#endif

#include <TypeDefinition/IdentificationType.hpp>
#include <TypeDefinition/StacklightType.hpp>
#include <TypeDefinition/ToolListType.hpp>
#include <TypeDefinition/ProductionJobListType.hpp>
#include <TypeDefinition/StateModeListType.hpp>

#include <signal.h>

#include <easylogging++.h>

#include <thread>
#include <chrono>
#include <atomic>

#include <ConfigureLogger.hpp>
#include <ConfigurationJsonFile.hpp>
#include <PublishTopics.hpp>
#include <Exceptions/ConfigurationException.hpp>

std::atomic_bool running = true;

static void stopHandler(int sig) {
	LOG(INFO) << "Execution termindated by ctrl-c";
	running = false;
}

int main(int argc, char* argv[])
{
	Umati::Util::ConfigureLogger("DashboardOpcUaClient");

	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);
	LOG(INFO) << "Start Dashboard OPC UA Client";

	std::string configFilename("configuration.json");

	if (argc >= 2)
	{
		configFilename = argv[1];
	}

	std::shared_ptr<Umati::Util::Configuration> config;
	try {
		config = std::make_shared<Umati::Util::ConfigurationJsonFile>(configFilename);
	}
	catch (Umati::Util::Exception::ConfigurationException &ex)
	{
		LOG(ERROR) << "Configuration could not be loaded: " << ex.what();
		std::cout << "Usage <>.exe [ConfigurationFileName]";
		return 1;
	}

	auto pClient = std::make_shared<Umati::OpcUa::OpcUaClient>(config->OpcUaEndpoint());

	std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher;
	config->Mqtt().Hostname;
	config->Mqtt().Port;
	config->Mqtt().Username;
	config->Mqtt().Password;

	Umati::Util::PublishTopics pubTopics(config->Mqtt().TopicPrefix);

#if defined(PUBLISHER_MQTT_MOSQUITTO)
	pPublisher = std::make_shared <Umati::MqttPublisher::MqttPublisher>(
		config->Mqtt().Hostname,
		config->Mqtt().Port,
		pubTopics.ClientOnline,
		config->Mqtt().Username,
		config->Mqtt().Password);
#elif defined(PUBLISHER_REDIS)
	pPublisher = std::make_shared<Umati::RedisPublisher::RedisPublisher>("prj-umati01");
#elif defined(PUBLISHER_MQTT_PAHO)
	pPublisher = std::make_shared<Umati::MqttPublisher_Paho::MqttPublisher_Paho>(
		config->Mqtt().Hostname,
		config->Mqtt().Port,
		pubTopics.ClientOnline,
		config->Mqtt().Username,
		config->Mqtt().Password);
#else
#error "No publisher defined"
#endif

	Umati::Dashboard::DashboardClient dashClient(pClient, pPublisher);

	LOG(INFO) << "Begin read model";

	const std::string NodeIdIdentifier_Identification("i=5001");
	dashClient.addDataSet(
		{ config->InstanceNamespaceURI(), NodeIdIdentifier_Identification },
		Umati::Dashboard::TypeDefinition::getIdentificationType(),
		pubTopics.Information
	);

	const std::string NodeIdIdentifier_Stacklight("i=5005");
	dashClient.addDataSet(
		{ config->InstanceNamespaceURI(), NodeIdIdentifier_Stacklight },
		Umati::Dashboard::TypeDefinition::getStacklightType(),
		pubTopics.Stacklight
	);

	const std::string NodeIdIdentifier_Tools("i=5024");
	dashClient.addDataSet(
		{ config->InstanceNamespaceURI(), NodeIdIdentifier_Tools },
		Umati::Dashboard::TypeDefinition::getToolListType(),
		pubTopics.Tools
	);

	const std::string NodeIdIdentifier_ProductionPlan("i=5012");
	dashClient.addDataSet(
		{ config->InstanceNamespaceURI(), NodeIdIdentifier_ProductionPlan },
		Umati::Dashboard::TypeDefinition::getProductionJobListType(),
		pubTopics.ProductionPlan
	);

	const std::string NodeIdIdentifier_StateMode("i=5007");
	dashClient.addDataSet(
		{ config->InstanceNamespaceURI(), NodeIdIdentifier_StateMode },
		Umati::Dashboard::TypeDefinition::getStateModeListType(),
		pubTopics.StateMode
	);

	LOG(INFO) << "Read model finished, start publishing";

	int i = 0;
	while (running)
	{
		++i;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if ((i % 10) == 0)
		{
			dashClient.Publish();
		}
	}

	LOG(INFO) << "End Dashboard OPC UA Client";
	return 0;
}

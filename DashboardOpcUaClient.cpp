#include "OpcUaClient/OpcUaClient.hpp"
#include <DashboardClient.hpp>
#include <OpcUaTypeReader.hpp>
#include <MqttPublisher_Paho.hpp>
#include <signal.h>
#include <easylogging++.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <ConfigureLogger.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>
#include <DashboardMachineObserver.hpp>

std::atomic_bool running = {true};

static void stopHandler(int sig)
{
	LOG(INFO) << "Execution termindated by ctrl-c";
	running = false;
}

int main(int argc, char *argv[])
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
	try
	{
		config = std::make_shared<Umati::Util::ConfigurationJsonFile>(configFilename);
	}
	catch (Umati::Util::Exception::ConfigurationException &ex)
	{
		LOG(ERROR) << "Configuration could not be loaded: " << ex.what();
		std::cout << "Usage <>.exe [ConfigurationFileName]";
		return 1;
	}

	std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper;
	try
	{
		opcUaWrapper = std::make_shared<Umati::OpcUa::OpcUaWrapper>();
	}
	catch (std::exception &ex)
	{
		LOG(ERROR) << "OpcUaInterface couldn't be initialized: " << ex.what();
	}
 
	auto pClient = std::make_shared<Umati::OpcUa::OpcUaClient>(
		config->getOpcUa().Endpoint,
		config->getOpcUa().Username,
		config->getOpcUa().Password,
		config->getOpcUa().Security,
		config->getObjectTypeNamespaces(),
		opcUaWrapper);

	auto pPublisher = std::make_shared<Umati::MqttPublisher_Paho::MqttPublisher_Paho>(
		config->getMqtt().Hostname,
		config->getMqtt().Port,
		config->getMqtt().Username,
		config->getMqtt().Password);

	auto pOpcUaTypeReader = std::make_shared<Umati::Dashboard::OpcUaTypeReader>(
		pClient,
		config->getObjectTypeNamespaces(),
		config->getNamespaceInformations());

	{
		std::size_t i = 0;
		while (running && !pClient->isConnected() && i < 100)
		{
			LOG(INFO) << "Waiting for OPC UA connection.";
			std::this_thread::sleep_for(std::chrono::seconds(1));
			++i;
		}

		if (!pClient->isConnected())
		{
			LOG(INFO) << "Connection not established, exiting.";
			return 1;
		}
		pOpcUaTypeReader->readTypes();
	}

	Umati::MachineObserver::DashboardMachineObserver dashboardMachineObserver(
		pClient,
		pPublisher,
		pOpcUaTypeReader);


	int i = 0;
	while (running)
	{
		
		{
		std::lock_guard<std::recursive_mutex> l(pClient.get()->m_clientMutex);
		auto retval = UA_Client_run_iterate(pClient->m_pClient.get(), 1000);
		}

		++i;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if ((i % 10) == 0)
		{
			dashboardMachineObserver.PublishAll();
		}
	}

	LOG(INFO) << "End Dashboard OPC UA Client";
	return 0;
}

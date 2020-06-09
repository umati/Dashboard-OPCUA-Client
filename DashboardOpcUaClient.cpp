#include "OpcUaClient/OpcUaClient.hpp"
#include <DashboardClient.hpp>
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


std::atomic_bool running = { true };

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

    std::shared_ptr<Umati::OpcUa::OpcUaInterface> opcUaWrapper;
	try {
        opcUaWrapper = std::make_shared<Umati::OpcUa::OpcUaWrapper>();
    }
	catch(std::exception &ex) {
        LOG(ERROR) << "OpcUaInterface couldn't be initialized: " << ex.what();
    }

	auto pClient = std::make_shared<Umati::OpcUa::OpcUaClient>(
		config->OpcUa().Endpoint,
		config->OpcUa().Username,
		config->OpcUa().Password,
		config->OpcUa().Security,
		config->ObjectTypeNamespacesVector(),
		opcUaWrapper
		);

	auto pPublisher = std::make_shared<Umati::MqttPublisher_Paho::MqttPublisher_Paho>(
		config->Mqtt().Hostname,
		config->Mqtt().Port,
		config->Mqtt().Username,
		config->Mqtt().Password);

	Umati::MachineObserver::DashboardMachineObserver dashboardMachineObserv(
		pClient,
		pPublisher);

	int i = 0;
	while (running)
	{
		++i;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if ((i % 50) == 0)
		{
			dashboardMachineObserv.PublishAll();
		}
	}

	LOG(INFO) << "End Dashboard OPC UA Client";
	return 0;
}

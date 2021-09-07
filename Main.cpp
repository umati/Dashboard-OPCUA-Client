#include "DashboardOpcUaClient.hpp"
#include <signal.h>
#include <easylogging++.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <ConfigureLogger.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>
#include <chrono>


std::atomic_bool running = {true};
std::atomic_bool reset = {false};

static void stopHandler(int sig)
{
	LOG(INFO) << "Execution termindated by ctrl-c";
	running = false;
}

static void issueReset()
{
	LOG(INFO) << "Requesting reset";
	reset = true;
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

	std::size_t resetCounter = 0;
	while(running) {
		if(reset) {
			++resetCounter;
			reset = false;
		}
		DashboardOpcUaClient dashboardClient(config, issueReset);

		if (!dashboardClient.connect(running))
		{
			LOG(INFO) << "Connection not established, exiting.";
			return -1;
		}

		dashboardClient.ReadTypes();
		dashboardClient.StartMachineObserver();
		while (running && !reset)
		{
			dashboardClient.Iterate();
		}
	}
	
	LOG(INFO) << "End Dashboard OPC UA Client";
	return 0;
}

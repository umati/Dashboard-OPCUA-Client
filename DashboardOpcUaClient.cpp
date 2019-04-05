#include "OpcUaClient/OpcUaClient.hpp"
#include <signal.h>

#include <easylogging++.h>

#include <thread>
#include <chrono>
#include <atomic>

#include <ConfigureLogger.hpp>

std::atomic_bool running = true;

static void stopHandler(int sig) {
	LOG(INFO) << "Execution termindated by ctrl-c";
	running = false;
}

int main(int argc, char* argv[])
{
	Util::ConfigureLogger("DashboardOpcUaClient");

	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);
	LOG(INFO) << "Start Dashboard OPC UA Client";
	
	if (argc < 2)
	{
		LOG(ERROR) << "No Server URL provided.";
		return 1;
	}

	OpcUa::OpcUaClient client(argv[1]);
	
	while (running)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	LOG(INFO) << "End Dashboard OPC UA Client normally";
	return 0;
}

#include "OpcUaClient/OpcUaClient.hpp"
#include <DashboardClient.hpp>
#include <RedisPublisher.hpp>
#include <TypeDefinition/IdentificationType.hpp>
#include <TypeDefinition/StacklightType.hpp>
#include <TypeDefinition/ToolListType.hpp>
#include <TypeDefinition/ProductionJobListType.hpp>
#include <TypeDefinition/NotificationFolderType.hpp>
#include <TypeDefinition/StateModeListType.hpp>

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
	Umati::Util::ConfigureLogger("DashboardOpcUaClient");

	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);
	LOG(INFO) << "Start Dashboard OPC UA Client";

	if (argc < 2)
	{
		LOG(ERROR) << "No Server URL provided.";
		return 1;
	}

	auto pClient = std::make_shared<Umati::OpcUa::OpcUaClient>(argv[1]);
	auto pPublisher = std::make_shared<Umati::RedisPublisher::RedisPublisher>("prj-umati01");
	Umati::Dashboard::DashboardClient dashClient(pClient, pPublisher);

	LOG(INFO) << "Begin read model";

	dashClient.addDataSet(
		{ "http://www.umati.info/example", "i=5001" },
		Umati::Dashboard::TypeDefinition::getIdentificationType(),
		"/umati/emo/ISW/ExampleMachine/Information"
	);

	dashClient.addDataSet(
		{ "http://www.umati.info/example", "i=5005" },
		Umati::Dashboard::TypeDefinition::getStacklightType(),
		"/umati/emo/ISW/ExampleMachine/Monitoring/Stacklight"
	);

	dashClient.addDataSet(
		{ "http://www.umati.info/example", "i=5024" },
		Umati::Dashboard::TypeDefinition::getToolListType(),
		"/umati/emo/ISW/ExampleMachine/ToolManagement/Tools"
	);

	dashClient.addDataSet(
		{ "http://www.umati.info/example", "i=5012" },
		Umati::Dashboard::TypeDefinition::getProductionJobListType(),
		"/umati/emo/ISW/ExampleMachine/ProductionPlan"
	);

	dashClient.addDataSet(
		{ "http://www.umati.info/example", "i=5030" },
		Umati::Dashboard::TypeDefinition::getNotificationFolderType(),
		"/umati/emo/ISW/ExampleMachine/Notification"
	);

	dashClient.addDataSet(
		{ "http://www.umati.info/example", "i=5007" },
		Umati::Dashboard::TypeDefinition::getStateModeListType(),
		"/umati/emo/ISW/ExampleMachine/StateMode"
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

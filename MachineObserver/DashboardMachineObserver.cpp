#include "DashboardMachineObserver.hpp"
#include "DashboardMachineObserver.hpp"
#include "DashboardMachineObserver.hpp"
#include "DashboardMachineObserver.hpp"

#include <easylogging++.h>

#include <TypeDefinition/IdentificationType.hpp>
#include <TypeDefinition/StacklightType.hpp>
#include <TypeDefinition/ToolListType.hpp>
#include <TypeDefinition/ProductionJobListType.hpp>
#include <TypeDefinition/StateModeListType.hpp>

#include "MachineCacheJsonFile.hpp"

#include "Exceptions/MachineInvalidException.hpp"
#include "Exceptions/MachineOfflineException.hpp"
#include "Exceptions/NoPublishTopicSet.hpp"

#include <Exceptions/OpcUaException.hpp>

namespace Umati {
	namespace MachineObserver {

		DashboardMachineObserver::DashboardMachineObserver(
			std::shared_ptr<Dashboard::IDashboardDataClient> pDataClient,
			std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher,
			std::string machineCacheFilename
		) : MachineObserver(pDataClient),
			m_pPublisher(pPublisher),
			m_pubTopicFactory(std::make_shared<MachineCacheJsonFile>(machineCacheFilename))
		{
			startUpdateMachineThread();
		}

		DashboardMachineObserver::~DashboardMachineObserver()
		{
			stopMachineUpdateThread();
		}

		void DashboardMachineObserver::PublishAll()
		{

			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			for (auto pDashClient : m_dashboardClients)
			{
				pDashClient.second->Publish();
			}

			if ((--m_publishMachinesOnline) <= 0)
			{
				m_publishMachinesOnline = PublishMachinesListResetValue;

				this->publishMachinesList();
			}
		}

		void DashboardMachineObserver::startUpdateMachineThread()
		{
			if (m_running)
			{
				LOG(INFO) << "Machine update trhead already running";
				return;
			}

			auto func = [this]()
			{
				int cnt = 0;
				while (this->m_running)
				{
					if ((cnt % 10) == 0)
					{
						this->UpdateMachines();
					}

					++cnt;
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			};
			m_running = true;

			m_updateMachineThread = std::thread(func);
		}

		void DashboardMachineObserver::stopMachineUpdateThread()
		{
			m_running = false;
			if (m_updateMachineThread.joinable())
			{
				m_updateMachineThread.join();
			}
		}

		void DashboardMachineObserver::publishMachinesList()
		{
			nlohmann::json publishData = nlohmann::json::array();
			for (const auto machineOnline : m_onlineMachines)
			{
				publishData.push_back(
					static_cast<nlohmann::json>(machineOnline.second)
				);
			}

			m_pPublisher->Publish(MachinesListTopic, publishData.dump(2));
		}

		void DashboardMachineObserver::publishOnlineStatus(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine, bool online)
		{
			auto pubTopics = m_pubTopicFactory.getPubTopics(machine);
			if (pubTopics.isValid())
			{
				m_pPublisher->Publish(pubTopics.Online, nlohmann::json(online).dump(2));
			}
		}

		void DashboardMachineObserver::addMachine(
			Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine
		)
		{
			try {
				LOG(INFO) << "New Machine: " << machine.BrowseName.Name << " NodeId:" << static_cast<std::string>(machine.NodeId);

				auto pDashClient = std::make_shared<Umati::Dashboard::DashboardClient>(m_pDataClient, m_pPublisher);
				auto pubTopics = m_pubTopicFactory.getPubTopics(machine);
				auto nsUri = machine.NodeId.Uri;

				if (!pubTopics.isValid())
				{
					std::stringstream ss;
					ss << "Invalid publisher topic. Machine: " << machine.BrowseName.Name
						<< " NodeId:" << static_cast<std::string>(machine.NodeId);
					LOG(ERROR) << ss.str();
					throw Exceptions::NoPublishTopicSet(ss.str());
				}

				MachineInformation_t machineInformation;
				machineInformation.TopicPrefix = pubTopics.TopicPrefix;

				if (!isOnline(machine))
				{
					std::stringstream ss;
					ss << "Machine: '" << machine.BrowseName.Name << "' (" << machine.NodeId.Uri << ") is offline";
					LOG(INFO) << ss.str();
					throw Umati::MachineObserver::Exceptions::MachineOfflineException(ss.str());
				}

				LOG(INFO) << "Read machine data";

				const std::string NodeIdIdentifier_LocationPlant("i=6005");
				const std::string NodeIdIdentifier_Manufacturer("i=6006");
				const std::string NodeIdIdentifier_NameCatalog("i=6002");

				auto valuesList = m_pDataClient->readValues(
					{
						{nsUri, NodeIdIdentifier_LocationPlant},
						{nsUri, NodeIdIdentifier_Manufacturer},
						{nsUri, NodeIdIdentifier_NameCatalog},
					}
				);

				if (!valuesList.at(0)["value"].is_string())
				{
					std::stringstream ss;
					ss << "LocationPlant is not a string. Machine: " << machine.BrowseName.Name
						<< " NodeId:" << static_cast<std::string>(machine.NodeId);
					LOG(ERROR) << ss.str();
					throw Exceptions::MachineInvalidException(ss.str());
				}
				machineInformation.LocationPlant = valuesList.at(0)["value"].get<std::string>();
				LOG(INFO) << "LocationPlant: " << machineInformation.LocationPlant;

				if (!valuesList.at(1)["value"].is_string())
				{
					std::stringstream ss;
					ss << "Manufacturer is not a string. Machine: " << machine.BrowseName.Name
						<< " NodeId:" << static_cast<std::string>(machine.NodeId);
					LOG(ERROR) << ss.str();
					throw Exceptions::MachineInvalidException(ss.str());
				}
				machineInformation.DisplayManufacturer = valuesList.at(1)["value"].get<std::string>();
				LOG(INFO) << "Manufacturer: " << machineInformation.DisplayManufacturer;

				if (!valuesList.at(2)["value"].is_string())
				{
					std::stringstream ss;
					ss << "NameCatalog is not a string. Machine: " << machine.BrowseName.Name
						<< " NodeId:" << static_cast<std::string>(machine.NodeId);
					LOG(ERROR) << ss.str();
					throw Exceptions::MachineInvalidException(ss.str());
				}
				machineInformation.DisplayName = valuesList.at(2)["value"].get<std::string>();
				LOG(INFO) << "NameCatalog: " << machineInformation.DisplayName;

				LOG(INFO) << "Begin read model";

				const std::string NodeIdIdentifier_Identification("i=5001");
				pDashClient->addDataSet(
					{ nsUri, NodeIdIdentifier_Identification },
					Umati::Dashboard::TypeDefinition::getIdentificationType(),
					pubTopics.Information
				);

				const std::string NodeIdIdentifier_Stacklight("i=5005");
				pDashClient->addDataSet(
					{ nsUri, NodeIdIdentifier_Stacklight },
					Umati::Dashboard::TypeDefinition::getStacklightType(),
					pubTopics.Stacklight
				);

				const std::string NodeIdIdentifier_Tools("i=5024");
				pDashClient->addDataSet(
					{ nsUri, NodeIdIdentifier_Tools },
					Umati::Dashboard::TypeDefinition::getToolListType(),
					pubTopics.Tools
				);

				const std::string NodeIdIdentifier_ProductionPlan("i=5012");
				pDashClient->addDataSet(
					{ nsUri, NodeIdIdentifier_ProductionPlan },
					Umati::Dashboard::TypeDefinition::getProductionJobListType(),
					pubTopics.ProductionPlan
				);

				const std::string NodeIdIdentifier_StateMode("i=5007");
				pDashClient->addDataSet(
					{ nsUri, NodeIdIdentifier_StateMode },
					Umati::Dashboard::TypeDefinition::getStateModeListType(),
					pubTopics.StateMode
				);

				LOG(INFO) << "Read model finished, start publishing";

				{
					std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
					m_dashboardClients.insert(std::make_pair(machine.NodeId, pDashClient));
					m_onlineMachines.insert(std::make_pair(machine.NodeId, machineInformation));
				}

			}
			catch (const Umati::Exceptions::OpcUaException &ex)
			{
				LOG(ERROR) << "Could not add Machine " << machine.BrowseName.Name
					<< " NodeId:" << static_cast<std::string>(machine.NodeId) <<
					"OpcUa Error: " << ex.what();

				throw Exceptions::MachineInvalidException(static_cast<std::string>(machine.NodeId));
			}
		}

		void DashboardMachineObserver::removeMachine(
			Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine
		)
		{
			std::unique_lock<decltype(m_dashboardClients_mutex)> ul(m_dashboardClients_mutex);
			LOG(INFO) << "Remove Machine: " << machine.BrowseName.Name << " NodeId:" << static_cast<std::string>(machine.NodeId);
			auto it = m_dashboardClients.find(machine.NodeId);
			if (it != m_dashboardClients.end())
			{
				m_dashboardClients.erase(it);
			}
			else
			{
				LOG(INFO) << "Machine not known: '" << static_cast<std::string>(machine.NodeId) << "'";
			}

			auto itOnlineMachines = m_onlineMachines.find(machine.NodeId);
			if (itOnlineMachines != m_onlineMachines.end())
			{
				m_onlineMachines.erase(itOnlineMachines);
			}
			else
			{
				LOG(INFO) << "Machine was not online: '" << static_cast<std::string>(machine.NodeId) << "'";
			}
		}

		bool DashboardMachineObserver::isOnline(Umati::Dashboard::IDashboardDataClient::BrowseResult_t machine)
		{
			bool retIsOnline;
			const std::string NodeIdIdentifier_BuildYear("i=6001");

			auto valuesList = m_pDataClient->readValues(
				{
					{machine.NodeId.Uri, NodeIdIdentifier_BuildYear},
				}
			);

			if (!valuesList.at(0)["statusCode"].is_object() ||
				!valuesList.at(0)["statusCode"]["code"].is_number_unsigned() ||
				valuesList.at(0)["statusCode"]["code"] == 0x800d0000 // BadServerNotConnected (0x800d0000)
				)
			{
				retIsOnline = false;
			}
			else
			{
				retIsOnline = true;
			}

			this->publishOnlineStatus(machine, retIsOnline);

			return retIsOnline;
		}
	}
}

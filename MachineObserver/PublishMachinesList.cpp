#include "PublishMachinesList.hpp"
#include <Topics.hpp>

namespace Umati
{
	namespace MachineObserver
	{
		PublishMachinesList::PublishMachinesList(std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher, std::vector<std::string> &specifications, std::function<std::string(const std::string&)> getTopic)
		:m_pPublisher(pPublisher), m_Specifications(specifications), m_getTopic(getTopic)
		{}

		void PublishMachinesList::AddMachine(std::string specification, nlohmann::json data)
		{
			m_Machines[specification].push_back(data);
		}

		void PublishMachinesList::Publish()
		{
			for(auto el : m_Machines)
			{
				nlohmann::json publishData = nlohmann::json::array();
				for(auto machineData : el.second)
				{
					publishData.push_back(machineData);
				}
				m_pPublisher->Publish(m_getTopic(el.first), publishData.dump(0));
			}

			for(auto spec : m_Specifications)
			{
				if(m_Machines.count(spec) == 0)
				{
					m_pPublisher->Publish(m_getTopic(spec), nlohmann::json::array().dump(0));
				}
			}
		}
	}
}

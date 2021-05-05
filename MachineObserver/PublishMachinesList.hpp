#include <memory>
#include <IPublisher.hpp>
#include <ModelOpcUa/ModelDefinition.hpp>
#include <map>
#include <list>
#include <string>
#include <nlohmann/json.hpp>

namespace Umati
{
	namespace MachineObserver
	{
		/// Sorting and preparing machines for publish machines list.
		class PublishMachinesList {
			public:
			PublishMachinesList(std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher, std::vector<std::string> &specifications);

			void AddMachine(std::string specification, nlohmann::json data);
			void Publish();
			protected:
			const std::vector<std::string> &m_Specifications;
			std::map<std::string, std::list<nlohmann::json>> m_Machines;
			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;
		};
	}
}

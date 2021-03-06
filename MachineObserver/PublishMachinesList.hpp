 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

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
			PublishMachinesList(std::shared_ptr<Umati::Dashboard::IPublisher> pPublisher, std::vector<std::string> &specifications, std::function<std::string(const std::string&)> getTopic);

			void AddMachine(std::string specification, nlohmann::json data);
			void Publish();
			protected:
			const std::vector<std::string> &m_Specifications;
			std::map<std::string, std::list<nlohmann::json>> m_Machines;
			std::shared_ptr<Umati::Dashboard::IPublisher> m_pPublisher;
            std::function<std::string(const std::string&)> m_getTopic;
		};
	}
}

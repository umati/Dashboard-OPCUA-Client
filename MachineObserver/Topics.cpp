 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include "Topics.hpp"
#include <IdEncode.hpp>

namespace Umati
{
    namespace MachineObserver
    {
        std::string Topics::Prefix = "umati";
        std::string Topics::ClientId = "umati";
        std::string Topics::Machine(
            const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
            const std::string &machineId)
        {
            std::string specification = p_type->SpecifiedBrowseName.Name;
            std::stringstream topic;
            topic << Topics::Prefix << "/" << Topics::ClientId << "/" << specification << "/" << Umati::Util::IdEncode(machineId);
            return topic.str();
        }

        std::string Topics::List(const std::string &specType)
        {
            std::stringstream topic;
            topic << Topics::Prefix << "/" << Topics::ClientId << "/list/" << specType;
            return topic.str();
        }

        std::string Topics::ErrorList(const std::string &specType)
        {
            std::stringstream topic;
            topic << Topics::Prefix << "/" << Topics::ClientId << "/bad_list/" << specType;
            return  topic.str();
        }

        std::string Topics::OnlineStatus(const std::string &machineId) {
            std::stringstream topic;
            topic << Topics::Prefix << "/" << Topics::ClientId << "/online/" << Umati::Util::IdEncode(machineId);
            return topic.str();
        }
    } // namespace MachineObserver
} // namespace Umati

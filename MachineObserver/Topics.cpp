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
        std::string const Topics::Prefix = "/umati";
        std::string Topics::Machine(
            const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
            const std::string &namespaceUri)
        {
            std::string specification = p_type->SpecifiedBrowseName.Name;
            std::stringstream topic;
            topic << Topics::Prefix << "/" << specification << "/" << Umati::Util::IdEncode(namespaceUri);
            return topic.str();
        }

        std::string Topics::List(const std::string &specType)
        {
            std::stringstream topic;
            topic << Topics::Prefix << "/list/" << specType;
            return topic.str();
        }

        std::string Topics::ErrorList(const std::string &specType)
        {
            std::stringstream topic;
            topic << Topics::Prefix << "/bad_list/" << specType;
            return  topic.str();
        }
    } // namespace MachineObserver
} // namespace Umati

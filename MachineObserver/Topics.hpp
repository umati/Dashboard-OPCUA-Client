 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */
#pragma once
#include <string>
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
    namespace MachineObserver
    {
        class Topics
        {
        public:
            static std::string Prefix;
            static std::string ClientId;
            static std::string Machine(
                const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
                const std::string &machineId);
            static std::string List(const std::string &specType);
            static std::string ErrorList(const std::string &specType);
            static std::string OnlineStatus(const std::string &machineId);
        };
    } // namespace MachineObserver
} // namespace Umati

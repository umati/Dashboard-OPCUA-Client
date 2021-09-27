/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#pragma once

#include <ModelOpcUa/ModelDefinition.hpp>

namespace Interfaces {
	class ReadAddressSpace {
	public:
		struct BrowseResult_t {
			ModelOpcUa::NodeId_t NodeId;
			ModelOpcUa::NodeId_t TypeDefinitionId;
			std::string BrowseName;
			ModelOpcUa::NodeClass_t NodeClass;
		};

		virtual std::list<BrowseResult_t>
		browseNodes(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId,
					ModelOpcUa::NodeId_t nodeTypeId) = 0;

		virtual ModelOpcUa::NodeId_t
		translateBrowsePathToNodeId(ModelOpcUa::NodeId_t startNode, std::string BrowseName) = 0;
	};
}
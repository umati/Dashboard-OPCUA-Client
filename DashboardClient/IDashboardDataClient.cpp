/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2020-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include "IDashboardDataClient.hpp"

namespace Umati {
	namespace Dashboard {
		IDashboardDataClient::ValueSubscriptionHandle::~ValueSubscriptionHandle() = default;

		ModelOpcUa::ModellingRule_t IDashboardDataClient::BrowseModellingRule(ModelOpcUa::NodeId_t nodeId)
		{
			BrowseContext_t brContext;
			brContext.nodeClassMask = (std::uint32_t) BrowseContext_t::NodeClassMask::OBJECT;
			auto browseResults = this->Browse(nodeId, brContext);
			for(auto & browseResult : browseResults)
			{
				///\TODO Use NodeId!
				if (browseResult.BrowseName.Name == "Mandatory")
				{
					return ModelOpcUa::Mandatory;
				}
				else if (browseResult.BrowseName.Name == "Optional")
				{
					return  ModelOpcUa::Optional;
				}
				else if (browseResult.BrowseName.Name == "MandatoryPlaceholder")
				{
					return  ModelOpcUa::MandatoryPlaceholder;
				}
				else if (browseResult.BrowseName.Name == "OptionalPlaceholder")
				{
					return  ModelOpcUa::OptionalPlaceholder;
				}
			}
			return ModelOpcUa::ModellingRule_t::Optional;
		}
	}
}
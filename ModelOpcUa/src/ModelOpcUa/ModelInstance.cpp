 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include "ModelInstance.hpp"

#include <utility>

#include <utility>

namespace ModelOpcUa {

	void PlaceholderNode::addInstance(const PlaceholderElement &instance) {
		this->Instances.push_back(instance);
	}

	std::list<PlaceholderElement> PlaceholderNode::getInstances() const {
		return this->Instances;
	}
	void PlaceholderNode::removeInstance(const PlaceholderElement &instance) {
		for(std::list<PlaceholderElement>::iterator it = this->Instances.begin(); it != this->Instances.end(); it++ ) {
			if(it->pNode->NodeId == instance.pNode->NodeId) {
				this->Instances.erase(it);
				break;
			}
		}
	}

	Node::Node(const NodeDefinition &nodeDefinition, std::list<std::shared_ptr<const Node>> childNodes)
			: NodeDefinition(nodeDefinition), ChildNodes(std::move(childNodes)) {
	}

	Node::~Node() = default;

	SimpleNode::SimpleNode(
			NodeId_t nodeId,
			NodeId_t typeNodeId,
			const NodeDefinition &nodeDefinition,
			const std::list<std::shared_ptr<const Node>> &childNodes
	)
			: NodeId(std::move(std::move(nodeId))), TypeNodeId(std::move(std::move(typeNodeId))),
			  Node(nodeDefinition, childNodes) {

	}

	SimpleNode::SimpleNode(const NodeDefinition &nodeDefinition) : Node(nodeDefinition, {}) {
	}

	bool SimpleNode::isAvaliable() const {
		return !NodeId.isNull();
	}

}

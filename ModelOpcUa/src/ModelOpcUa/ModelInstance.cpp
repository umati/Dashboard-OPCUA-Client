#include "ModelInstance.hpp"

#include <utility>

#include <utility>

namespace ModelOpcUa {

	void PlaceholderNode::addInstance(const PlaceholderElement& instance) {
		this->Instances.push_back(instance);
	}

	std::list<PlaceholderElement> PlaceholderNode::getInstances() const {
		return this->Instances;
	}

	Node::Node(const NodeDefinition &nodeDefinition, std::list<std::shared_ptr<const Node>> childNodes)
			: NodeDefinition(nodeDefinition), ChildNodes(std::move(childNodes)) {
	}

	Node::~Node() = default;

	SimpleNode::SimpleNode(
			NodeId_t nodeId,
			NodeId_t typeNodeId,
			const NodeDefinition& nodeDefinition,
			const std::list<std::shared_ptr<const Node>> &childNodes
	)
			: NodeId(std::move(std::move(nodeId))), TypeNodeId(std::move(std::move(typeNodeId))), Node(nodeDefinition, childNodes) {

	}

	SimpleNode::SimpleNode(const NodeDefinition& nodeDefinition) : Node(nodeDefinition, {}) {
	}

	bool SimpleNode::isAvaliable() const {
		return !NodeId.isNull();
	}

}

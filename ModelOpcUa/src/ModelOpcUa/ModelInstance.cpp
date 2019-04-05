#include "ModelInstance.hpp"
#include "ModelInstance.hpp"

#include "ModelInstance.hpp"

namespace ModelOpcUa {

	void PlaceholderNode::addInstance(PlaceholderElement instance)
	{
		this->Instances.push_back(instance);
	}

	std::list<PlaceholderElement> PlaceholderNode::getInstances()
	{
		return this->Instances;
	}

	Node::Node(const NodeDefinition & nodeDefinition, const std::list<std::shared_ptr<const Node>> &childNodes) 
		: NodeDefinition(nodeDefinition), ChildNodes(childNodes)
	{
	}

	SimpleNode::SimpleNode(
		NodeId_t nodeId,
		NodeId_t typeNodeId,
		NodeDefinition nodeDefinition,
		const std::list<std::shared_ptr<const Node>> &childNodes
	)
		:NodeId(nodeId), TypeNodeId(typeNodeId), Node(nodeDefinition, childNodes)
	{

	}

	SimpleNode::SimpleNode(NodeDefinition nodeDefinition) : Node(nodeDefinition, { })
	{
	}

	bool SimpleNode::isAvaliable()
	{
		return !NodeId.isNull();
	}

}

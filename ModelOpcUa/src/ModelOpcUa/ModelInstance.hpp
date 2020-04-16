
#pragma once
#include "ModelDefinition.hpp"

#include <memory>

namespace ModelOpcUa {

	//Instance of a node, contains the structur informations
	class Node : public NodeDefinition
	{
	public:
		using NodeDefinition::NodeDefinition;
		Node(const NodeDefinition&, const std::list<std::shared_ptr<const Node>> &childNodes);
		virtual ~Node() = 0;

		/// All child elements
		const std::list<std::shared_ptr<const Node>> ChildNodes;
	protected:
	};

	// ModellingRule is one of {Optional, Mandatory}
	class SimpleNode : public Node {
	public:
		virtual ~SimpleNode() = default;

		// Node available
		SimpleNode(
			NodeId_t nodeId,
			NodeId_t typeNodeId,
			NodeDefinition nodeDefinition,
			const std::list<std::shared_ptr<const Node>> &childNodes
		);

		// Node not available
		SimpleNode(NodeDefinition nodeDefinition);

		bool isAvaliable();
		NodeId_t NodeId;

		// The instanciated type of the Node (might be a subtype of the defined one)
		NodeId_t TypeNodeId;
	};

	struct PlaceholderElement
	{
		std::shared_ptr<const SimpleNode> pNode;
		QualifiedName_t BrowseName;
	};

	// ModellingRule is one of {OptionalPlaceholder, MandatoryPlaceholder}
	class PlaceholderNode : public Node {
	public:
		using Node::Node;
		void addInstance(PlaceholderElement instance);
		
		std::list<PlaceholderElement> getInstances() const;

	protected:
		/// The found instances according to the predefined Type
		std::list<PlaceholderElement> Instances;
	};

}

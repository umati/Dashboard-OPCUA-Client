
#pragma once
#include "ModelDefinition.hpp"

#include <memory>

namespace ModelOpcUa {

	//Instance of a node, contains the structur informations
	class Node : public NodeDefinition
	{
	public:
		virtual ~Node() = 0;
	protected:
	};

	// ModellingRule is one of {Optional, Mandatory}
	class SimpleNode : public Node {
	public:
		virtual ~SimpleNode() = default;

		bool isAvaliable();
		NodeId_t NodeId;

		// The instanciated type of the Node (might be a subtype of the defined one)
		NodeId_t TypeNodeId;
	};

	struct PlaceholderElement
	{
		std::shared_ptr<SimpleNode> pNode;
		std::string BrowseName;
	};

	// ModellingRule is one of {OptionalPlaceholder, MandatoryPlaceholder}
	class PlaceholderNode : public Node {
	public:
		void addInstance(PlaceholderElement instance);
		
		std::list<PlaceholderElement> getInstances();

	protected:
		/// The found instances according to the predefined Type
		std::list<PlaceholderElement> Instances;
	};

}

#pragma once

#include <string>
#include <list>
#include <memory>
/// \todo split into two files, one for the type/ structure information, one for the instances

namespace ModelOpcUa {

	typedef std::string NodeId_t;

	enum ModellingRule_t
	{
		Optional,
		Mandatory,
		OptionalPlaceholder,
		MandatoryPlaceholder
	};

	enum NodeClass_t
	{
		Object,
		Variable
	};


	/// Only Information about the overall structure
	class NodeDefinition {

	public:
		NodeDefinition(NodeClass_t nodeClass,
			ModellingRule_t modellingRule,
			NodeId_t referenceType,
			NodeId_t specifiedTypeNodeId,
			std::string specifiedBrowseName
		);
		const ModellingRule_t ModellingRule;
		const NodeClass_t NodeClass;

		/// Valid for Single- and Placeholder-Children,
		/// needed by Browse and TranslateBrowsenameToNodeId
		const NodeId_t ReferenceType;

		/// The type which is specified in the specification
		const NodeId_t SpecifiedTypeNodeId;

		/// BrowseName as specified in the companion specification, might be different from
		/// the instance BrowseName in case of placeholder components
		const std::string SpecifiedBrowseName;

		virtual ~NodeDefinition() = default;
	};

	/// Defines the definition and child elements
	class StructureNode : public NodeDefinition
	{
	public:
		StructureNode(NodeClass_t nodeClass,
			ModellingRule_t modellingRule,
			NodeId_t referenceType,
			NodeId_t specifiedTypeNodeId,
			std::string specifiedBrowseName,
			std::list<std::shared_ptr<const StructureNode>> childNodes =
				std::list<std::shared_ptr<const StructureNode>>()
		);

		/// All child elements
		/// \TODO what is used in case of an Placeholder, use a different Type for non placeholder elements?
		const std::list<std::shared_ptr<const StructureNode>> SpecifiedChildNodes;
	};

	class StructurePlaceholderNode : public StructureNode {
	public:

		StructurePlaceholderNode(
			NodeClass_t nodeClass,
			ModellingRule_t modellingRule,
			NodeId_t referenceType,
			NodeId_t specifiedTypeNodeId,
			std::string specifiedBrowseName,
			std::list<std::shared_ptr<const StructureNode>> childNodes,
			std::list<std::shared_ptr<const StructureNode>> possibleTypes
		);

		// All predefined subtypes that are handeled separately
		const std::list<std::shared_ptr<const StructureNode>> PossibleTypes;
	};

}

#pragma once

#include <string>
#include <list>
#include <memory>
#include <sstream>

namespace ModelOpcUa
{

	//typedef std::string NodeId_t;
	struct NodeId_t
	{
		std::string Uri;
		/// Format e.g. "i=10"
		std::string Id;

		bool isNull() const
		{
			return Uri.empty() && Id.empty();
		}

		explicit operator std::string() const
		{
			std::stringstream ss;
			if (!Uri.empty())
			{
				ss << "nsu=" << Uri << ";";
			}

			if (Id.empty())
			{
				ss << "i=0";
			}
			else
			{
				ss << Id;
			}

			return ss.str();
		}

		bool operator==(const NodeId_t &other) const
		{
			return this->Uri == other.Uri && this->Id == other.Id;
		}

		bool operator<(const NodeId_t &other) const
		{
			if (this->Uri != other.Uri)
			{
				return this->Uri < other.Uri;
			}

			return this->Id < other.Id;
		}
	};

	struct QualifiedName_t
	{
		std::string Uri;

		std::string Name;

		bool isNull() const
		{
			return Uri.empty() && Name.empty();
		}

		explicit operator std::string() const
		{
			std::stringstream ss;
			if (!Uri.empty())
			{
				ss << "nsu=" << Uri << ";";
			}

			ss << Name;

			return ss.str();
		}

		bool operator==(const QualifiedName_t &other) const
		{
			return this->Uri == other.Uri && this->Name == other.Name;
		}
	};

	enum ModellingRule_t
	{
		None,
		Optional,
		Mandatory,
		OptionalPlaceholder,
		MandatoryPlaceholder,
	};

	/*
			* - OpcUa_NodeClass_Object        = 1,
			* - OpcUa_NodeClass_Variable      = 2,
			* - OpcUa_NodeClass_Method        = 4,
			* - OpcUa_NodeClass_ObjectType    = 8,
			* - OpcUa_NodeClass_VariableType  = 16,
			* - OpcUa_NodeClass_ReferenceType = 32,
			* - OpcUa_NodeClass_DataType      = 64,
			* - OpcUa_NodeClass_View          = 128
			* */
	enum NodeClass_t
	{
		Object = 1 << 0,
		Variable = 1 << 1,
		Method = 1 << 2,
		ObjectType = 1 << 3,
		VariableType = 1 << 4,
		ReferenceType = 1 << 5,
		DataType = 1 << 6,
		View = 1 << 7
	};

	struct BrowseResult_t
	{
		ModelOpcUa::NodeClass_t NodeClass;
		ModelOpcUa::NodeId_t NodeId;
		ModelOpcUa::NodeId_t TypeDefinition;
		ModelOpcUa::NodeId_t ReferenceTypeId;
		ModelOpcUa::QualifiedName_t BrowseName;
	};

	/// Only Information about the overall structure
	class NodeDefinition
	{

	public:
		NodeDefinition(NodeClass_t nodeClass,
					   ModellingRule_t modellingRule,
					   NodeId_t referenceType,
					   NodeId_t specifiedTypeNodeId,
					   QualifiedName_t specifiedBrowseName);

		const ModellingRule_t ModellingRule;
		const NodeClass_t NodeClass;

		/// Valid for Single- and Placeholder-Children,
		/// needed by Browse and TranslateBrowsenameToNodeId
		const NodeId_t ReferenceType;

		/// The type which is specified in the specification
		const NodeId_t SpecifiedTypeNodeId;

		/// BrowseName as specified in the companion specification, might be different from
		/// the instance BrowseName in case of placeholder components
		const QualifiedName_t SpecifiedBrowseName;

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
					  QualifiedName_t specifiedBrowseName, bool ofBaseDataVariableType,
					  std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes = std::make_shared<std::list<std::shared_ptr<StructureNode>>>()

		);

		StructureNode(StructureNode *structureNode, bool ofBaseDataVariableType,
					  std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes =
						  std::make_shared<std::list<std::shared_ptr<StructureNode>>>()

		);

		StructureNode(const BrowseResult_t &browseResult, bool ofBaseDataVariableType,
					  std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes =
						  std::make_shared<std::list<std::shared_ptr<StructureNode>>>()

		);

		StructureNode(const BrowseResult_t &browseResult,
					  std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
					  ModellingRule_t modellingRule, bool ofBaseDataVariableType);

		std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> SpecifiedChildNodes;
		bool ofBaseDataVariableType = false;

		static std::string printType(const std::shared_ptr<StructureNode> &node, const std::string &parentTree);

	private:
		static std::string
		printJsonIntern(const std::shared_ptr<StructureNode> &node, const std::string &parentTree, int tabs);
	};

	/**
	 * Bidirectional node holding a shared_ptr to the parent
	 */
	class StructureBiNode
	{
	public:
		StructureBiNode(NodeClass_t nodeClass,
						ModellingRule_t modellingRule,
						NodeId_t referenceType,
						NodeId_t specifiedTypeNodeId,
						QualifiedName_t specifiedBrowseName,
						bool ofBaseDataVariableType,
						std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes =
							std::make_shared<std::list<std::shared_ptr<StructureNode>>>(),
						std::shared_ptr<StructureBiNode> parent = nullptr,
						std::string namespaceUri = "");

		StructureBiNode(BrowseResult_t browseResult,
						bool ofBaseDataVariableType,
						std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes =
							std::make_shared<std::list<std::shared_ptr<StructureNode>>>(),
						std::shared_ptr<StructureBiNode> parent = nullptr,
						std::string namespaceUri = "");

		StructureBiNode(BrowseResult_t browseResult,
						bool ofBaseDataVariableType,
						std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes =
							std::make_shared<std::list<std::shared_ptr<StructureNode>>>(),
						std::shared_ptr<StructureBiNode> parent = nullptr,
						std::string namespaceUri = "",
						ModellingRule_t = Optional);

		std::shared_ptr<StructureNode> toStructureNode();

		std::shared_ptr<StructureNode> structureNode;
		std::shared_ptr<StructureBiNode> parent;
		std::string namespaceUri;
		bool isType = false;
		bool ofBaseDataVariableType = false;
		std::shared_ptr<std::list<std::shared_ptr<StructureBiNode>>> SpecifiedBiChildNodes = std::make_shared<std::list<std::shared_ptr<StructureBiNode>>>();
	};

	class StructurePlaceholderNode : public StructureNode
	{
	public:
		StructurePlaceholderNode(
			NodeClass_t nodeClass,
			ModellingRule_t modellingRule,
			NodeId_t referenceType,
			NodeId_t specifiedTypeNodeId,
			QualifiedName_t specifiedBrowseName, bool ofBaseDataVariableType,
			std::shared_ptr<std::list<std::shared_ptr<StructureNode>>> childNodes,
			std::list<std::shared_ptr<const StructureNode>> possibleTypes

		);

		explicit StructurePlaceholderNode(const std::shared_ptr<StructureNode> &sharedPtr);

		// All predefined subtypes that are handled separately
		const std::list<std::shared_ptr<const StructureNode>> PossibleTypes;
	};

} // namespace ModelOpcUa

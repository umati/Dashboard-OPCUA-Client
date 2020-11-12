#pragma once

#include <nlohmann/json.hpp>

#include <ModelOpcUa/ModelDefinition.hpp>
#include <functional>

namespace Umati {

	namespace Dashboard {
		/**
		* Interface that describes functions to e.g. browse a source (e.g. OPC UA Server)
		* Is implemented e.g. by Umati::OpcUa::OpcUaClient. 
		*/
		class IDashboardDataClient {
		public:

			typedef std::function<void(nlohmann::json value)> newValueCallbackFunction_t;

			virtual ~IDashboardDataClient() = default;


			virtual std::list<ModelOpcUa::BrowseResult_t>
			Browse(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId,
				   ModelOpcUa::NodeId_t typeDefinition) = 0;

			virtual std::list<ModelOpcUa::BrowseResult_t>
			BrowseHasComponent(ModelOpcUa::NodeId_t startNode,
							   ModelOpcUa::NodeId_t typeDefinition) = 0;

			virtual ModelOpcUa::NodeId_t TranslateBrowsePathToNodeId(
					ModelOpcUa::NodeId_t startNode,
					ModelOpcUa::QualifiedName_t browseName
			) = 0;

			std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureNode>>> m_typeMap = std::make_shared<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureNode>>>();
			std::shared_ptr<std::map<std::string, ModelOpcUa::NodeId_t>> m_nameToId = std::make_shared<std::map<std::string, ModelOpcUa::NodeId_t>>();

			struct NamespaceInformation_t {
				std::string Namespace;
				std::string NamespaceUri;
				std::string NamespaceType;
				std::string NamespaceIdentificationType;
			};

			std::map<uint16_t, NamespaceInformation_t> m_availableObjectTypeNamespaces;
			std::map<std::string, uint16_t> m_uriToIndexCache;

			class ValueSubscriptionHandle {
			public:
				virtual ~ValueSubscriptionHandle() = 0;

				virtual void unsubscribe() = 0;

				bool isUnsubscribed() const { return m_unsubscribed; }

			protected:
				void setUnsubscribed() {
					m_unsubscribed = true;
				}

			private:
				bool m_unsubscribed = false;
			};

			virtual std::string readNodeBrowseName(const ModelOpcUa::NodeId_t &nodeId) = 0;

			virtual std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId) = 0;

			virtual std::shared_ptr<ValueSubscriptionHandle>
			Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) = 0;

			virtual std::vector<nlohmann::json> ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> nodeIds) = 0;

			virtual uint GetImplementedNamespaceIndex(const ModelOpcUa::NodeId_t &nodeId) = 0;

			virtual void
			CreateMachineListForNamespaceUnderStartNode(std::list<ModelOpcUa::BrowseResult_t> &machineList,
														const std::string &startNodeNamespaceUri,
														const ModelOpcUa::NodeId_t &startNode) = 0;

			virtual void
			FillIdentificationValuesFromBrowseResult(std::list<ModelOpcUa::BrowseResult_t> &identification,
													 std::list<ModelOpcUa::NodeId_t> &identificationNodes,
													 std::vector<std::string> &identificationValueKeys) = 0;
		};
	}
}

#pragma once
#include <nlohmann/json.hpp>

#include <ModelOpcUa/ModelDefinition.hpp>
#include <functional>

namespace Umati {

	namespace Dashboard
	{
		/**
		* Interface that describes functions to e.g. browse a source (e.g. OPC UA Server)
		* Is implemented e.g. by Umati::OpcUa::OpcUaClient. 
		*/
		class IDashboardDataClient
		{
		public:

			typedef std::function<void(nlohmann::json value)> newValueCallbackFunction_t;

			virtual ~IDashboardDataClient() = default;


			virtual std::list<ModelOpcUa::BrowseResult_t> Browse(ModelOpcUa::NodeId_t startNode, ModelOpcUa::NodeId_t referenceTypeId, ModelOpcUa::NodeId_t typeDefinition) = 0;

            virtual ModelOpcUa::NodeId_t TranslateBrowsePathToNodeId(
				ModelOpcUa::NodeId_t startNode,
				ModelOpcUa::QualifiedName_t browseName
			) = 0;

			class ValueSubscriptionHandle {
			public:
				virtual ~ValueSubscriptionHandle() = 0;
				virtual void unsubscribe() = 0;

				bool isUnsubscribed() { return m_unsubscribed; }
			protected:
				void setUnsubscribed() {
					m_unsubscribed = true;
				}

			private:
				bool m_unsubscribed = false;
			};

			virtual std::shared_ptr<ValueSubscriptionHandle> Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) = 0;

			virtual std::vector<nlohmann::json> readValues(std::list<ModelOpcUa::NodeId_t> nodeIds) = 0;

		};
	}
}

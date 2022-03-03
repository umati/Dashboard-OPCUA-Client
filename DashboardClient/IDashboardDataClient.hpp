/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#pragma once

#include <nlohmann/json.hpp>
#include <ModelOpcUa/ModelDefinition.hpp>
#include <functional>
#include <stack>
#include <list>
#include "NodeIdsWellKnown.hpp"

namespace Umati
{

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

            /// See "Browse Service" from OPC UA Part 4 for details
            struct BrowseContext_t
            {
                enum class BrowseDirection
                {
                    FORWARD = 0,
                    BACKWARD = 1,
                    BOTH = 2,
                } browseDirection = BrowseDirection::FORWARD;
                ModelOpcUa::NodeId_t referenceTypeId;
                bool includeSubtypes = true;
                enum class NodeClassMask
                {
                    ALL = 0x00,
                    OBJECT = 0X01,
                    VARIABLE = 0X02,
                    METHOD = 0X04,
                    OBJECT_TYPE = 0X08,
                    VARIABLE_TYPE = 0X10,
                    REFERENCE_TYPE = 0X20,
                    DATATYPE = 0X40,
                    VIEW = 0X80,
                };
                std::uint32_t nodeClassMask = (std::uint32_t) NodeClassMask::ALL;
                enum class ResultMask
                {
                    REFERENCETYPE = 0X01,
                    ISFORWARD = 0X02,
                    NODECLASS = 0X04,
                    BROWSENAME = 0X08,
                    DISPLAYNAME = 0X10,
                    TYPEDEFINITION = 0X20,
                    ALL = 0X3F,
                };
                std::uint32_t resultMask = (std::uint32_t) ResultMask::ALL;

                inline static BrowseContext_t HasComponent()
                {
                    BrowseContext_t ret;
                    ret.referenceTypeId = NodeId_HasComponent;
                    return ret;
                }

                inline static BrowseContext_t HasTypeDefinition()
                {
                    BrowseContext_t ret;
                    ret.referenceTypeId = NodeId_HasTypeDefinition;
                    return ret;
                }

                inline static BrowseContext_t Hierarchical()
                {
                    BrowseContext_t ret;
                    ret.referenceTypeId = NodeId_HierarchicalReferences;
                    return ret;
                }

                inline static BrowseContext_t DescriptionOf() {
                    BrowseContext_t ret;
                    ret.referenceTypeId = NodeId_HasDescription;
                    ret.browseDirection = BrowseDirection::BACKWARD;
                    return ret;
                }

                inline static BrowseContext_t EncodingOf() {
                    BrowseContext_t ret;
                    ret.referenceTypeId = NodeId_HasEncoding;
                    ret.browseDirection = BrowseDirection::BACKWARD;
                    return ret;
                }

                inline static BrowseContext_t OrganizedBy() {
                    BrowseContext_t ret;
                    ret.referenceTypeId = NodeId_Organizes;
                    ret.browseDirection = BrowseDirection::BACKWARD;
                    return ret;
                }

                inline static BrowseContext_t ComponentOf() {
                    BrowseContext_t ret;
                    ret.referenceTypeId = NodeId_HasComponent;
                    ret.browseDirection = BrowseDirection::BACKWARD;
                    return ret;
                }

                inline static BrowseContext_t WithReference(
                    ModelOpcUa::NodeId_t referenceTypeId)
                {
                    BrowseContext_t ret;
                    ret.referenceTypeId = referenceTypeId;
                    return ret;
                }

                inline static BrowseContext_t ObjectAndVariableWithTypes()
                {
                    BrowseContext_t ret;
                    ret.nodeClassMask =
                    (std::uint32_t)NodeClassMask::OBJECT |
                    (std::uint32_t)NodeClassMask::OBJECT_TYPE |
                    (std::uint32_t)NodeClassMask::VARIABLE |
                    (std::uint32_t)NodeClassMask::VARIABLE_TYPE;
                    ret.referenceTypeId = NodeId_HierarchicalReferences;
                    return ret;
                }

                inline static BrowseContext_t Variable()
                {
                    BrowseContext_t ret;
                    ret.nodeClassMask =
                    (std::uint32_t)NodeClassMask::VARIABLE |
                    (std::uint32_t)NodeClassMask::VARIABLE_TYPE;
                    ret.referenceTypeId = NodeId_HierarchicalReferences;
                    return ret;
                }
            };

            virtual void updateCustomTypes() = 0;

            virtual std::list<ModelOpcUa::BrowseResult_t>
            Browse(
                ModelOpcUa::NodeId_t startNode,
                BrowseContext_t browseContext) = 0;

            virtual bool
            isSameOrSubtype(
                const ModelOpcUa::NodeId_t &expectedType,
                const ModelOpcUa::NodeId_t &checkType,
                std::size_t maxDepth) = 0;

            virtual std::list<ModelOpcUa::BrowseResult_t>
            BrowseWithResultTypeFilter(
                ModelOpcUa::NodeId_t startNode,
                BrowseContext_t browseContext,
                ModelOpcUa::NodeId_t typeDefinition) = 0;

            // Deprecated
            inline virtual std::list<ModelOpcUa::BrowseResult_t>
            Browse(
                ModelOpcUa::NodeId_t startNode,
                ModelOpcUa::NodeId_t referenceTypeId,
                ModelOpcUa::NodeId_t typeDefinition)
            {
                return BrowseWithResultTypeFilter(startNode, BrowseContext_t::WithReference(referenceTypeId), typeDefinition);
            }

            // Deprecated
            inline virtual std::list<ModelOpcUa::BrowseResult_t>
            BrowseHasComponent(
                ModelOpcUa::NodeId_t startNode,
                ModelOpcUa::NodeId_t typeDefinition)
            {
                return BrowseWithResultTypeFilter(startNode, BrowseContext_t::HasComponent(), typeDefinition);
            }

            ModelOpcUa::ModellingRule_t BrowseModellingRule(ModelOpcUa::NodeId_t nodeId);

            virtual ModelOpcUa::NodeId_t TranslateBrowsePathToNodeId(
                ModelOpcUa::NodeId_t startNode,
                ModelOpcUa::QualifiedName_t browseName) = 0;

            std::map<std::string, uint16_t> m_uriToIndexCache;

            class ValueSubscriptionHandle
            {
            public:

                ValueSubscriptionHandle(int32_t clientHandle, int32_t monItemId, ModelOpcUa::NodeId_t nodeId) : m_clientHandle(clientHandle), m_monitoredItemId(monItemId), m_nodeId(nodeId){}

                ~ValueSubscriptionHandle();

                void unsubscribe() {m_unsubscribed = true;}

                bool isUnsubscribed() const { return m_unsubscribed; }

                int32_t getClientHandle() const { return m_clientHandle; }

                int32_t getMonitoredItemId() const { return m_monitoredItemId; }

                ModelOpcUa::NodeId_t getNodeId() const { return m_nodeId; }

            protected:
                void setUnsubscribed()
                {
                    m_unsubscribed = true;
                }

            private:
                bool m_unsubscribed = false;
                int32_t m_clientHandle;
                int32_t m_monitoredItemId;
                ModelOpcUa::NodeId_t m_nodeId;
            };

            virtual void readTypeDictionaries() = 0;

            virtual void buildCustomDataTypes() = 0;

            virtual std::string readNodeBrowseName(const ModelOpcUa::NodeId_t &nodeId) = 0;

            /// \todo Extract from interface!
            virtual std::string getTypeName(const ModelOpcUa::NodeId_t &nodeId) = 0;

            virtual std::shared_ptr<ValueSubscriptionHandle>
            Subscribe(ModelOpcUa::NodeId_t nodeId, newValueCallbackFunction_t callback) = 0;

            virtual void Unsubscribe(std::vector<int32_t> monItemIds, std::vector<int32_t> clientHandles) = 0;

            virtual std::vector<nlohmann::json> ReadeNodeValues(std::list<ModelOpcUa::NodeId_t> nodeIds) = 0;

            virtual std::vector<std::string> Namespaces() = 0;

            /// Verify that the connection and session are ok
            virtual bool VerifyConnection() = 0;
        };
    } // namespace Dashboard
} // namespace Umati

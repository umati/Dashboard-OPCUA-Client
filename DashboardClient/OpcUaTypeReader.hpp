#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <ModelOpcUa/ModelInstance.hpp>
#include "IDashboardDataClient.hpp"

namespace Umati
{
    namespace Dashboard
    {
        class OpcUaTypeReader
        {
        public:
            OpcUaTypeReader(
                std::shared_ptr<IDashboardDataClient> pIClient,
                std::vector<std::string> expectedObjectTypeNamespaces);
            void readTypes();
            struct NamespaceInformation_t
            {
                std::string Namespace;
                std::string NamespaceUri;
                std::string NamespaceType;
                std::string NamespaceIdentificationType;
            };
            // Key: NsUri
            std::map<std::string, NamespaceInformation_t> m_availableObjectTypeNamespaces;
            std::vector<std::string> m_expectedObjectTypeNamespaces;
            std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureNode>>> m_typeMap = std::make_shared<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureNode>>>();
            std::shared_ptr<std::map<std::string, ModelOpcUa::NodeId_t>> m_nameToId = std::make_shared<std::map<std::string, ModelOpcUa::NodeId_t>>();
            std::shared_ptr<ModelOpcUa::StructureNode> getTypeOfNamespace(const std::string &namespaceUri) const;
        protected:
            std::shared_ptr<Umati::Dashboard::IDashboardDataClient> m_pClient;
            const ModelOpcUa::NodeId_t m_emptyId = ModelOpcUa::NodeId_t{"", ""};
            const ModelOpcUa::NodeId_t m_basicVariableTypeNode = ModelOpcUa::NodeId_t{"http://opcfoundation.org/UA/",
                                                                                      "i=63"};
            const ModelOpcUa::NodeId_t m_basicObjectTypeNode = ModelOpcUa::NodeId_t{"http://opcfoundation.org/UA/",
                                                                                    "i=58"};
            void initialize(std::vector<std::string> &notFoundObjectTypeNamespaces);
            void browseObjectOrVariableTypeAndFillBidirectionalTypeMap(
                const ModelOpcUa::NodeId_t &basicTypeNode,
                std::shared_ptr<std::map<std::string,
                                         std::shared_ptr<
                                             ModelOpcUa::StructureBiNode>>>
                    bidirectionalTypeMap,
                bool ofBaseDataVariableType);

            void updateTypeMap();
            uint GetImplementedNamespaceIndex(const ModelOpcUa::NodeId_t &nodeId);
            void findObjectTypeNamespacesAndCreateTypeMap(
                const std::string &namespaceURI,
                std::shared_ptr<
                    std::map<
                        std::string,
                        std::shared_ptr<ModelOpcUa::StructureBiNode>>>
                    bidirectionalTypeMap =
                        std::make_shared<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>>());
            static void
            createTypeMap(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap, const std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureNode>>> &sharedPtr,
                          std::string namespaceUri);

            std::shared_ptr<ModelOpcUa::StructureBiNode> handleBrowseTypeResult(
                std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>

                                > &bidirectionalTypeMap,
                const ModelOpcUa::BrowseResult_t &entry,
                const std::shared_ptr<ModelOpcUa::StructureBiNode> &parent, ModelOpcUa::ModellingRule_t modellingRule,
                bool ofBaseDataVariableType);

            void browseTypes(
                std::shared_ptr<
                    std::map<
                        std::string,
                        std::shared_ptr<ModelOpcUa::StructureBiNode>>>
                    bidirectionalTypeMap,
                const IDashboardDataClient::BrowseContext_t &browseContext,
                const ModelOpcUa::NodeId_t &startNodeId,
                const std::shared_ptr<ModelOpcUa::StructureBiNode> &parent,
                bool ofBaseDataVariableType);
        };
    } // namespace OpcUa
} // namespace Umati

#include "OpcUaTypeReader.hpp"
#include <easylogging++.h>
#include <regex>

namespace Umati
{
    namespace Dashboard
    {

        OpcUaTypeReader::OpcUaTypeReader(
            std::shared_ptr<IDashboardDataClient> pIClient,
            std::vector<std::string> expectedObjectTypeNamespaces)
            : m_expectedObjectTypeNamespaces(std::move(expectedObjectTypeNamespaces)),
              m_pClient(pIClient)
        {
        }

        void OpcUaTypeReader::readTypes()
        {
            std::vector<std::string> notFoundObjectTypeNamespaces;
            auto bidirectionalTypeMap =
                std::make_shared<
                    std::map<
                        std::string,
                        std::shared_ptr<ModelOpcUa::StructureBiNode>>>();
            initialize(notFoundObjectTypeNamespaces);
            browseObjectOrVariableTypeAndFillBidirectionalTypeMap(NodeId_BaseVariableType, bidirectionalTypeMap, true);
            LOG(INFO) << "Browsing variable types finished, continuing browsing object types";

            browseObjectOrVariableTypeAndFillBidirectionalTypeMap(NodeId_BaseObjectType, bidirectionalTypeMap, false);
            LOG(INFO) << "Browsing object types finished";

            auto namespaces = m_pClient->Namespaces();
            for (std::size_t iNamespace = 0; iNamespace < namespaces.size(); ++iNamespace)
            {
                auto namespaceURI = namespaces[iNamespace];
                auto it = find(notFoundObjectTypeNamespaces.begin(), notFoundObjectTypeNamespaces.end(), namespaceURI);
                if (it == notFoundObjectTypeNamespaces.end())
                {
                    continue;
                }
                else
                {
                    notFoundObjectTypeNamespaces.erase(it);
                }
                findObjectTypeNamespacesAndCreateTypeMap(
                    namespaceURI,
                    bidirectionalTypeMap);
            }

            for (auto &notFoundObjectTypeNamespace : notFoundObjectTypeNamespaces)
            {
                LOG(WARNING) << "Unable to find namespace " << notFoundObjectTypeNamespace;
            }

            updateTypeMap();
        }

        void OpcUaTypeReader::initialize(std::vector<std::string> &notFoundObjectTypeNamespaces)
        {
            m_availableObjectTypeNamespaces.clear();
            for (auto &m_expectedObjectTypeNamespace : m_expectedObjectTypeNamespaces)
            {
                notFoundObjectTypeNamespaces.push_back(m_expectedObjectTypeNamespace);
            }
        }

        void OpcUaTypeReader::updateTypeMap()
        {
            for (auto mapIterator = m_typeMap->begin(); mapIterator != m_typeMap->end(); mapIterator++)
            {
                for (auto childIterator = mapIterator->second->SpecifiedChildNodes->begin();
                     childIterator != mapIterator->second->SpecifiedChildNodes->end(); childIterator++)
                {
                    try
                    {
                        std::string childTypeName = m_pClient->getTypeName(childIterator->get()->SpecifiedTypeNodeId);
                        auto childType = m_typeMap->find(childTypeName);

                        if (childType != m_typeMap->end())
                        {

                            childIterator->get()->SpecifiedChildNodes = childType->second->SpecifiedChildNodes;
                            childIterator->get()->ofBaseDataVariableType = childType->second->ofBaseDataVariableType;
                            //LOG(INFO) << "Updating type " << childTypeName <<" for " << childIterator->get()->SpecifiedBrowseName.Uri << ";" << childIterator->get()->SpecifiedBrowseName.Name;
                        }
                    }
                    catch (std::exception &ex)
                    {
                        LOG(WARNING) << "Unable to update type due to " << ex.what();
                    }
                }
            }
            LOG(INFO) << "Updated typeMap";
        }

        void OpcUaTypeReader::browseObjectOrVariableTypeAndFillBidirectionalTypeMap(
            const ModelOpcUa::NodeId_t &basicTypeNode,
            std::shared_ptr<
                std::map<
                    std::string,
                    std::shared_ptr<
                        ModelOpcUa::StructureBiNode>>>
                bidirectionalTypeMap,
            bool ofBaseDataVariableType)
        {
            // startBrowseTypeResult is needed to create a startVariableType
            const ModelOpcUa::BrowseResult_t startBrowseTypeResult{
                ModelOpcUa::NodeClass_t::VariableType, basicTypeNode, m_emptyId, m_emptyId,
                ModelOpcUa::QualifiedName_t{basicTypeNode.Uri, ""} // BrowseName
            };
            auto startVariableType = handleBrowseTypeResult(bidirectionalTypeMap, startBrowseTypeResult, nullptr,
                                                            ModelOpcUa::ModellingRule_t::Mandatory,
                                                            ofBaseDataVariableType);

            auto browseTypeContext = IDashboardDataClient::BrowseContext_t::ObjectAndVariable();
            browseTypes(
                bidirectionalTypeMap,
                browseTypeContext,
                basicTypeNode,
                startVariableType,
                ofBaseDataVariableType);
        }

        void OpcUaTypeReader::findObjectTypeNamespacesAndCreateTypeMap(
            const std::string &namespaceURI,
            std::shared_ptr<
                std::map<
                    std::string,
                    std::shared_ptr<ModelOpcUa::StructureBiNode>>>
                bidirectionalTypeMap)
        {
            NamespaceInformation_t information;

            information.Namespace = CSNameFromUri(namespaceURI);
            information.NamespaceUri = namespaceURI;


            std::string identificationTypeName = information.Namespace + "IdentificationType";
            std::string startNodeNamespaceUri = "http://opcfoundation.org/UA/Machinery/";
            /// \TODO Why 1012?
            ModelOpcUa::NodeId_t startNode = ModelOpcUa::NodeId_t{startNodeNamespaceUri, "i=1012"};

            auto browseResults = m_pClient->Browse(startNode, IDashboardDataClient::BrowseContext_t::ObjectAndVariable());

            for (auto &brResult : browseResults)
            {
                if (brResult.BrowseName.Uri == namespaceURI)
                {
                    identificationTypeName = brResult.BrowseName.Name;
                    break;
                }
            }

            std::string typeName = information.Namespace + "Type";
            auto startNode2 = ModelOpcUa::NodeId_t{ns0Uri, "i=58"};
            auto browseResults2 = m_pClient->Browse(startNode2, IDashboardDataClient::BrowseContext_t::ObjectAndVariable());

            for (auto &brResult : browseResults2)
            {
                /// \TODO Why 1014?
                if (brResult.NodeId.Id == "i=1014")
                {
                    typeName = brResult.BrowseName.Name;
                }
            }

            information.NamespaceType = typeName;
            information.NamespaceIdentificationType = identificationTypeName;

            m_availableObjectTypeNamespaces[namespaceURI] = information;
            LOG(INFO) << "Expected object type namespace " << namespaceURI << " found.";
            createTypeMap(bidirectionalTypeMap, m_typeMap, namespaceURI);
            LOG(INFO) << "Finished creatingTypeMap for " << namespaceURI;
        }

        void
        OpcUaTypeReader::createTypeMap(std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap, const std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureNode>>> &typeMap,
                                       std::string namespaceUri)
        {
            for (
                auto &typeIterator : *bidirectionalTypeMap)
            {
                if (typeIterator.second->namespaceUri != namespaceUri)
                {
                    continue;
                }
                // go to highest parent and then down the ladder to add / update attributes;
                // create a list of pointers till parent is null
                // go backwards and add / update child nodes till the end
                std::shared_ptr<std::list<std::shared_ptr<ModelOpcUa::StructureBiNode>>>
                    bloodline = std::make_shared<std::list<std::shared_ptr<ModelOpcUa::StructureBiNode>>>();
                std::shared_ptr<ModelOpcUa::StructureBiNode> currentGeneration = typeIterator.second;
                while (nullptr != currentGeneration)
                {
                    bloodline->emplace_back(currentGeneration);
                    currentGeneration = currentGeneration->parent;
                }
                std::string typeName = bloodline->front()->structureNode->SpecifiedBrowseName.Uri + ";" +
                                       bloodline->front()->structureNode->SpecifiedBrowseName.Name;
                ModelOpcUa::StructureNode node = bloodline->front()->structureNode.operator*();
                node.ofBaseDataVariableType = bloodline->front()->ofBaseDataVariableType;
                std::stringstream bloodlineStringStream;
                for (
                    auto bloodlineIterator = bloodline->end();
                    bloodlineIterator != bloodline->

                                         begin();

                )
                {
                    --bloodlineIterator;
                    auto ancestor = bloodlineIterator.operator*();
                    bloodlineStringStream << "->" << ancestor->structureNode->SpecifiedBrowseName.Uri << ";"
                                          << ancestor->structureNode->SpecifiedBrowseName.Name;
                    for (
                        auto childIterator = ancestor->SpecifiedBiChildNodes->begin();
                        childIterator != ancestor->SpecifiedBiChildNodes->

                                         end();

                        childIterator++)
                    {
                        auto currentChild = childIterator.operator*();
                        if (!currentChild->isType)
                        {
                            auto structureNode = currentChild->toStructureNode();

                            auto findIterator = std::find(node.SpecifiedChildNodes->begin(),
                                                          node.SpecifiedChildNodes->end(), structureNode);

                            bool found = findIterator != node.SpecifiedChildNodes->end();
                            if (!found)
                            {
                                for (
                                    auto fIt = node.SpecifiedChildNodes->begin();
                                    fIt != node.SpecifiedChildNodes->

                                           end();

                                    fIt++)
                                {
                                    if (fIt.operator*()->SpecifiedBrowseName.Name == structureNode->SpecifiedBrowseName.Name &&
                                        fIt
                                                .
                                                operator*()
                                                ->SpecifiedBrowseName.Uri ==
                                            structureNode->SpecifiedBrowseName.Uri)
                                    {
                                        findIterator = fIt;
                                        found = true;
                                        break;
                                    }
                                }
                            }

                            if (found)
                            {
                                if (findIterator.operator*()->ModellingRule == ModelOpcUa::ModellingRule_t::Optional ||
                                    findIterator.operator*()->ModellingRule ==
                                        ModelOpcUa::ModellingRule_t::OptionalPlaceholder)
                                {
                                    // LOG(INFO) << "Changed modellingRule from " << findIterator.operator*()->ModellingRule << " to " << structureNode->ModellingRule;
                                    node.SpecifiedChildNodes->erase(findIterator++);
                                    node.SpecifiedChildNodes->emplace_back(structureNode);
                                }
                            }
                            else
                            {
                                node.SpecifiedChildNodes->emplace_back(structureNode);
                            }
                        }
                    }
                }
                auto shared = std::make_shared<ModelOpcUa::StructureNode>(node);

                std::pair<std::string, std::shared_ptr<ModelOpcUa::StructureNode>> newType(typeName, shared);
                typeMap->insert(newType);
            }
        }

        std::shared_ptr<ModelOpcUa::StructureBiNode> OpcUaTypeReader::handleBrowseTypeResult(
            std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap,
            const ModelOpcUa::BrowseResult_t &entry,
            const std::shared_ptr<ModelOpcUa::StructureBiNode> &parent, ModelOpcUa::ModellingRule_t modellingRule,
            bool ofBaseDataVariableType)
        {

            bool isObjectType = ModelOpcUa::ObjectType == entry.NodeClass;
            bool isVariableType = ModelOpcUa::VariableType == entry.NodeClass;
            ModelOpcUa::StructureBiNode node(
                entry,
                ofBaseDataVariableType,
                std::make_shared<std::list<std::shared_ptr<ModelOpcUa::StructureNode>>>(),
                parent,
                entry.NodeId.Uri,
                modellingRule);
            auto current = std::make_shared<ModelOpcUa::StructureBiNode>(node);

            if (isObjectType || isVariableType)
            {
                std::string typeName = node.structureNode->SpecifiedBrowseName.Uri + ";" +
                                       node.structureNode->SpecifiedBrowseName.Name;
                if (bidirectionalTypeMap->count(typeName) == 0)
                {
                    current->isType = true;
                    current->ofBaseDataVariableType = ofBaseDataVariableType;
                    std::pair<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>> newType(typeName, current);
                    bidirectionalTypeMap->insert(newType);
                    std::pair<std::string, ModelOpcUa::NodeId_t> newNameMapping(typeName, entry.NodeId);
                    m_nameToId->insert(newNameMapping);
                    LOG_EVERY_N(50, INFO) << "Current size BiDirectionalTypeMap: " << bidirectionalTypeMap->size();
                }
                else
                {
                    LOG(INFO) << "Found Type " << typeName << " again";
                }
            }
            if (parent != nullptr)
            {
                parent->SpecifiedBiChildNodes->emplace_back(current);
            }
            return current;
        }

        void OpcUaTypeReader::browseTypes(
            std::shared_ptr<std::map<std::string, std::shared_ptr<ModelOpcUa::StructureBiNode>>> bidirectionalTypeMap,
            const IDashboardDataClient::BrowseContext_t &browseContext,
            const ModelOpcUa::NodeId_t &startNodeId,
            const std::shared_ptr<ModelOpcUa::StructureBiNode> &parent,
            bool ofBaseDataVariableType)
        {
            auto browseResults = m_pClient->Browse(startNodeId, browseContext);

            for (auto &browseResult : browseResults)
            {
                ModelOpcUa::ModellingRule_t modellingRule = m_pClient->BrowseModellingRule(browseResult.NodeId);
                // LOG(INFO) << "currently at " << startUaNodeId.toFullString().toUtf8();
                auto current = handleBrowseTypeResult(bidirectionalTypeMap, browseResult, parent, modellingRule,
                                                      ofBaseDataVariableType);
                browseTypes(bidirectionalTypeMap, browseContext, browseResult.NodeId, current, ofBaseDataVariableType);
            }
        }

        std::shared_ptr<ModelOpcUa::StructureNode> OpcUaTypeReader::getTypeOfNamespace(const std::string &namespaceUri) const
        {
            auto it = std::find_if(
                m_availableObjectTypeNamespaces.begin(),
                m_availableObjectTypeNamespaces.end(),
                [namespaceUri](const auto &el) {
                    return el.second.NamespaceUri == namespaceUri;
                });
            if (it == m_availableObjectTypeNamespaces.end())
            {
                LOG(ERROR) << "Could not get namespace: " << namespaceUri;
                return nullptr;
            }
            std::string typeName = it->second.NamespaceUri + ";" + it->second.NamespaceType;
            auto typePair = m_typeMap->find(typeName);
            if (typePair == m_typeMap->end())
            {
                LOG(ERROR) << "Unable to find " << typeName << " in typeMap";
                return nullptr;
            }
            return typePair->second;
        }

        std::string OpcUaTypeReader::CSNameFromUri(std::string nsUri)
        {
            const std::regex regex(R"(\/([\w\-]+)\/?$)");
            std::smatch match;
            if(std::regex_search(nsUri, match, regex))
            {
                return match[1].str();
            }
            else
            {
                return nsUri;
            }
        }

    } // namespace Dashboard
} // namespace Umati

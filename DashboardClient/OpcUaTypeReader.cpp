#include "OpcUaTypeReader.hpp"
#include <easylogging++.h>
#include <regex>

namespace Umati
{
    namespace Dashboard
    {

        OpcUaTypeReader::OpcUaTypeReader(
            std::shared_ptr<IDashboardDataClient> pIClient,
            std::vector<std::string> expectedObjectTypeNamespaces,
            std::vector<Umati::Util::NamespaceInformation> namespaceInformations)
            : m_expectedObjectTypeNamespaces(std::move(expectedObjectTypeNamespaces)),
              m_pClient(pIClient)
        {
            for (auto const &el: namespaceInformations) {
                m_availableObjectTypeNamespaces[el.Namespace] = el;
                for (auto const &e: el.Types) {
                    m_identificationTypeOfTypeDefinition.insert(std::make_pair(e, el.IdentificationType));
                }
            }
        }

        void OpcUaTypeReader::readTypes()
        {
            std::vector<std::string> notFoundObjectTypeNamespaces;
            auto bidirectionalTypeMap =
                std::make_shared<
                    std::map<
                        ModelOpcUa::NodeId_t,
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

            // printTypeMapYaml();
            updateTypeMap();
            updateObjectTypeNames();
        }
        void OpcUaTypeReader::updateObjectTypeNames() {
            m_expectedObjectTypeNames.clear();
            for (const auto &el: m_identificationTypeOfTypeDefinition) {
                auto typeNodeId = el.first;
                std::shared_ptr<ModelOpcUa::StructureNode> node;
                try
                {
                    node = typeDefinitionToStructureNode(typeNodeId);
                    m_expectedObjectTypeNames.push_back(node->SpecifiedBrowseName.Name);
                }
                catch(const std::exception& e)
                {
                    LOG(WARNING) << e.what() << ". Probably because the server does not have the NodeId, specified in the config ("
                                 << static_cast<std::string>(typeNodeId) << ").";
                }
            }
        }

        void OpcUaTypeReader::initialize(std::vector<std::string> &notFoundObjectTypeNamespaces)
        {
            for (auto &m_expectedObjectTypeNamespace : m_expectedObjectTypeNamespaces)
            {
                notFoundObjectTypeNamespaces.push_back(m_expectedObjectTypeNamespace);
            }
        }

        void OpcUaTypeReader::printTypeMapYaml() {
            for (auto mapIterator = m_typeMap->begin(); mapIterator != m_typeMap->end(); mapIterator++)
            {   
               std::cout << std::endl;
               ModelOpcUa::StructureNode::printYamlIntern(mapIterator->second, static_cast<std::string>(mapIterator->first), 1, std::cout);
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
                        auto childTypeNodeId = childIterator->get()->SpecifiedTypeNodeId;
                        if (childTypeNodeId == Dashboard::NodeId_Folder) {
                            for (auto childOfChildIterator = childIterator->get()->SpecifiedChildNodes->begin(); 
                                childOfChildIterator != childIterator->get()->SpecifiedChildNodes->end(); childOfChildIterator++) {
                                    auto childOfChild = childOfChildIterator->get()->SpecifiedTypeNodeId;
                                    auto childType = m_typeMap->find(childOfChild);
                                    if (childType != m_typeMap->end())
                                    {
                                        childOfChildIterator->get()->SpecifiedChildNodes = childType->second->SpecifiedChildNodes;
                                        childOfChildIterator->get()->ofBaseDataVariableType = childType->second->ofBaseDataVariableType;
                                    }
                            } 
                            continue;
                        }
                        auto childType = m_typeMap->find(childTypeNodeId);
                        if (childType != m_typeMap->end())
                        {
                            childIterator->get()->SpecifiedChildNodes = childType->second->SpecifiedChildNodes;
                            childIterator->get()->ofBaseDataVariableType = childType->second->ofBaseDataVariableType;
                        }
                    }
                    catch (std::exception &ex)
                    {
                        LOG(WARNING) << "Unable to update type due to " << ex.what();
                    }
                }
            }
        }

        void OpcUaTypeReader::browseObjectOrVariableTypeAndFillBidirectionalTypeMap(
            const ModelOpcUa::NodeId_t &startNodeId,
            OpcUaTypeReader::BiDirTypeMap_t bidirectionalTypeMap,
            bool ofBaseDataVariableType)
        {
            // startBrowseTypeResult is needed to create a startType
            const ModelOpcUa::BrowseResult_t startBrowseTypeResult{
                ModelOpcUa::NodeClass_t::VariableType, startNodeId, m_emptyId, m_emptyId,
                ModelOpcUa::QualifiedName_t{startNodeId.Uri, ""} // BrowseName
            };
            std::weak_ptr<ModelOpcUa::StructureBiNode> emptyParent;
            std::weak_ptr<ModelOpcUa::StructureBiNode> startType = handleBrowseTypeResult(bidirectionalTypeMap, startBrowseTypeResult, emptyParent,
                                                    ModelOpcUa::ModellingRule_t::Mandatory,
                                                    ofBaseDataVariableType);

            browseTypes(
                bidirectionalTypeMap,
                startNodeId,
                startType.lock(),
                ofBaseDataVariableType);
        }

        void OpcUaTypeReader::findObjectTypeNamespacesAndCreateTypeMap(
            const std::string &namespaceURI,
            OpcUaTypeReader::BiDirTypeMap_t bidirectionalTypeMap)
        {
            setupTypeMap(bidirectionalTypeMap, namespaceURI);
        }

        void
        OpcUaTypeReader::setupTypeMap(OpcUaTypeReader::BiDirTypeMap_t &bidirectionalTypeMap, std::string namespaceUri)
        {
            for (auto &typeIterator : *bidirectionalTypeMap)
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
                    currentGeneration = currentGeneration->parent.lock();
                }
                std::string typeName = bloodline->front()->structureNode->SpecifiedBrowseName.Uri + ";" +
                                       bloodline->front()->structureNode->SpecifiedBrowseName.Name;
                ModelOpcUa::StructureNode node = bloodline->front()->structureNode.operator*();
                node.ofBaseDataVariableType = bloodline->front()->ofBaseDataVariableType;
                std::stringstream bloodlineStringStream;

                for (auto bloodlineIterator = bloodline->rbegin();
                     bloodlineIterator != bloodline->rend();
                     ++bloodlineIterator)
                {
                    auto ancestor = *bloodlineIterator;
                    bloodlineStringStream << "->" << static_cast<std::string>(ancestor->structureNode->SpecifiedBrowseName);
                    for (auto &currentChild : *ancestor->SpecifiedBiChildNodes)
                    {
                        if (currentChild->isType)
                        {
                            continue;
                        }
                        auto structureNode = currentChild->toStructureNode();

                        // Search same child, or child with same name
                        auto findIterator = std::find_if(node.SpecifiedChildNodes->begin(), node.SpecifiedChildNodes->end(), [&](const auto &el) {
                            return el == structureNode || el->SpecifiedBrowseName == structureNode->SpecifiedBrowseName;
                        });

                        /// \todo Check if a merge is required here!

                        if (findIterator != node.SpecifiedChildNodes->end())
                        {
                            // Check if original child is optional, if so, override
                            if ((*findIterator)->ModellingRule == ModelOpcUa::ModellingRule_t::Optional ||
                                (*findIterator)->ModellingRule == ModelOpcUa::ModellingRule_t::OptionalPlaceholder)
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
                auto shared = std::make_shared<ModelOpcUa::StructureNode>(node);
                std::pair<ModelOpcUa::NodeId_t, std::shared_ptr<ModelOpcUa::StructureNode>> newType(typeIterator.first, shared);
                m_typeMap->insert(newType);
            }
        }

        std::shared_ptr<ModelOpcUa::StructureBiNode> OpcUaTypeReader::handleBrowseTypeResult(
            OpcUaTypeReader::BiDirTypeMap_t &bidirectionalTypeMap,
            const ModelOpcUa::BrowseResult_t &entry,
            const std::weak_ptr<ModelOpcUa::StructureBiNode> &parent, ModelOpcUa::ModellingRule_t modellingRule,
            bool ofBaseDataVariableType)
        {

            bool isObjectType = ModelOpcUa::ObjectType == entry.NodeClass;
            bool isVariableType = ModelOpcUa::VariableType == entry.NodeClass;
            ModelOpcUa::StructureBiNode node(
                entry,
                ofBaseDataVariableType,
                std::make_shared<std::list<std::shared_ptr<ModelOpcUa::StructureNode>>>(),
                parent.lock(),
                entry.NodeId.Uri,
                modellingRule);
            auto current = std::make_shared<ModelOpcUa::StructureBiNode>(node);

            if (isObjectType || isVariableType)
            {
                std::string typeName = node.structureNode->SpecifiedBrowseName.Uri + ";" +
                                       node.structureNode->SpecifiedBrowseName.Name;
                if (bidirectionalTypeMap->count(entry.NodeId) == 0)
                {
                    current->isType = true;
                    current->ofBaseDataVariableType = ofBaseDataVariableType;
                    std::pair<ModelOpcUa::NodeId_t, std::shared_ptr<ModelOpcUa::StructureBiNode>> newType(entry.NodeId, current);
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
            if (parent.lock() != nullptr)
            {
                parent.lock()->SpecifiedBiChildNodes->emplace_back(current);
            }
            return current;
        }

        void OpcUaTypeReader::browseTypes(
            OpcUaTypeReader::BiDirTypeMap_t bidirectionalTypeMap,
            const ModelOpcUa::NodeId_t &startNodeId,
            const std::weak_ptr<ModelOpcUa::StructureBiNode> &parent,
            bool ofBaseDataVariableType)
        {
            auto browseTypeContext = IDashboardDataClient::BrowseContext_t::ObjectAndVariableWithTypes();
            auto browseResults = m_pClient->Browse(startNodeId, browseTypeContext);

            for (auto &browseResult : browseResults)
            {
                ModelOpcUa::ModellingRule_t modellingRule = m_pClient->BrowseModellingRule(browseResult.NodeId);
                // LOG(INFO) << "currently at " << startNodeId.Uri << startNodeId.Id;
                std::weak_ptr<ModelOpcUa::StructureBiNode> current = handleBrowseTypeResult(bidirectionalTypeMap, browseResult, parent, modellingRule,
                                                      ofBaseDataVariableType);
                browseTypes(bidirectionalTypeMap, browseResult.NodeId, current, ofBaseDataVariableType);
            }
            
        }

        std::shared_ptr<ModelOpcUa::StructureNode> OpcUaTypeReader::typeDefinitionToStructureNode(const ModelOpcUa::NodeId_t &typeDefinition) const
        {
            auto typePair = m_typeMap->find(typeDefinition);
			if (typePair == m_typeMap->end())
			{
				LOG(ERROR) << "Unable to find " << static_cast<std::string>(typeDefinition) + " in typeMap";
				throw Umati::MachineObserver::Exceptions::MachineInvalidException("Type not found");
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

        std::shared_ptr<ModelOpcUa::StructureNode>
        OpcUaTypeReader::getIdentificationTypeStructureNode(const ModelOpcUa::NodeId_t &typeDefinition) const
		{   
			auto identificationTypeNodeId = getIdentificationTypeNodeId(typeDefinition);            
			return typeDefinitionToStructureNode(identificationTypeNodeId);
		}

        ModelOpcUa::NodeId_t
        OpcUaTypeReader::getIdentificationTypeNodeId(const ModelOpcUa::NodeId_t &typeDefinition) const
		{            
            auto pair = m_identificationTypeOfTypeDefinition.find(typeDefinition);
            if (pair == m_identificationTypeOfTypeDefinition.end()) {
                throw Umati::MachineObserver::Exceptions::MachineInvalidException("IdentificationType not found, probably because namesapce " +
														  static_cast<std::string>(typeDefinition) + " is not in the config, or this type does   " +
                                                          "not have an identification type.");
            }   
            return pair->second;
		}

    } // namespace Dashboard
} // namespace Umati

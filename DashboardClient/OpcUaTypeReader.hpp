 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2020-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#pragma once
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <ModelOpcUa/ModelInstance.hpp>
#include "IDashboardDataClient.hpp"
#include <Configuration.hpp>
#include "../MachineObserver/Exceptions/MachineInvalidException.hpp"
#include "TypeDictionary/TypeDictionary.hpp"
#include <sstream>
#include <iostream>

namespace Umati
{
    namespace Dashboard
    {
        class OpcUaTypeReader
        {
        public:
            OpcUaTypeReader(
                std::shared_ptr<IDashboardDataClient> pIClient,
                std::vector<std::string> expectedObjectTypeNamespaces, std::vector<Umati::Util::NamespaceInformation> namespaceInformations);

            ~OpcUaTypeReader();
            
            void readTypes();
            void readTypeDictionaries();
            using NamespaceInformation_t = Util::NamespaceInformation;

            /// \todo make the following internal structures private and provide access via funcitons
            std::map<ModelOpcUa::NodeId_t, ModelOpcUa::NodeId_t> m_identificationTypeOfTypeDefinition;
            std::map<std::string, NamespaceInformation_t> m_availableObjectTypeNamespaces;
            std::vector<std::string> m_expectedObjectTypeNamespaces;
            std::vector<std::string> m_expectedObjectTypeNames;
            std::vector<ModelOpcUa::NodeId_t> m_knownMachineTypeDefinitions;
            std::vector<Umati::TypeDictionary::TypeDictionary> m_typeDictionaries;
            std::map<ModelOpcUa::NodeId_t, ModelOpcUa::NodeId_t> m_subTypeDefinitionToKnownMachineTypeDefinition;
            std::shared_ptr<std::map<ModelOpcUa::NodeId_t, std::shared_ptr<ModelOpcUa::StructureNode>>> m_typeMap = std::make_shared<std::map<ModelOpcUa::NodeId_t, std::shared_ptr<ModelOpcUa::StructureNode>>>();
            std::shared_ptr<std::map<std::string, ModelOpcUa::NodeId_t>> m_nameToId = std::make_shared<std::map<std::string, ModelOpcUa::NodeId_t>>();
            std::shared_ptr<ModelOpcUa::StructureNode> typeDefinitionToStructureNode(const ModelOpcUa::NodeId_t &typeDefinition) const;
            std::shared_ptr<ModelOpcUa::StructureNode> getIdentificationTypeStructureNode(const ModelOpcUa::NodeId_t &typeDefinition) const;
            ModelOpcUa::NodeId_t getIdentificationTypeNodeId(const ModelOpcUa::NodeId_t &typeDefinition) const;
        protected:
            /// Map of <TypeName, StructureBiNode>
            typedef std::shared_ptr<std::map<ModelOpcUa::NodeId_t,
                                         std::shared_ptr<
                                             ModelOpcUa::StructureBiNode>>> BiDirTypeMap_t;
            std::shared_ptr<Umati::Dashboard::IDashboardDataClient> m_pClient;
            const ModelOpcUa::NodeId_t m_emptyId = ModelOpcUa::NodeId_t{"", ""};
            void initialize(std::vector<std::string> &notFoundObjectTypeNamespaces);
            void browseObjectOrVariableTypeAndFillBidirectionalTypeMap(
                const ModelOpcUa::NodeId_t &startNodeId,
                BiDirTypeMap_t bidirectionalTypeMap,
                bool ofBaseDataVariableType);

            void printTypeMapYaml();
            void updateObjectTypeNames();
            void updateTypeMap();
            void findObjectTypeNamespacesAndCreateTypeMap(
                const std::string &namespaceURI,
                BiDirTypeMap_t bidirectionalTypeMap =
                        std::make_shared<std::map<ModelOpcUa::NodeId_t, std::shared_ptr<ModelOpcUa::StructureBiNode>>>());
            void
            setupTypeMap(std::shared_ptr<std::map<ModelOpcUa::NodeId_t, std::shared_ptr<ModelOpcUa::StructureBiNode>>> &bidirectionalTypeMap, std::string namespaceUri);

            std::shared_ptr<ModelOpcUa::StructureBiNode> handleBrowseTypeResult(
                BiDirTypeMap_t &bidirectionalTypeMap,
                const ModelOpcUa::BrowseResult_t &entry,
                const std::weak_ptr<ModelOpcUa::StructureBiNode> &parent, ModelOpcUa::ModellingRule_t modellingRule,
                bool ofBaseDataVariableType);

            /// Browse all Nodes (Object, Variables, ObjectTypes, VariablesTypes) and fill the BiDirectionalTypeMap
            void browseTypes(
                BiDirTypeMap_t bidirectionalTypeMap,
                const ModelOpcUa::NodeId_t &startNodeId,
                const std::weak_ptr<ModelOpcUa::StructureBiNode> &parent,
                bool ofBaseDataVariableType);
            
            /// \return Companion Specification Name
            static std::string CSNameFromUri(std::string nsUri);
        };
    } // namespace OpcUa
} // namespace Umati

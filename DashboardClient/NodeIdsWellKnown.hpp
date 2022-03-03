 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#include <ModelOpcUa/ModelDefinition.hpp>
namespace Umati
{
    namespace Dashboard
    {
        const std::string ns0UriFull = "http://opcfoundation.org/UA/";
        const std::string ns0Uri = "http://opcfoundation.org/UA/"; // Omitt for node Ids?
        const ModelOpcUa::NodeId_t NodeId_HasComponent = {ns0Uri, "i=47"};
        const ModelOpcUa::NodeId_t NodeId_HierarchicalReferences = {ns0Uri, "i=33"};
        const ModelOpcUa::NodeId_t NodeId_HasTypeDefinition = {ns0Uri, "i=40"};
        const ModelOpcUa::NodeId_t NodeId_Organizes = {ns0Uri, "i=35"};
        const ModelOpcUa::NodeId_t NodeId_BaseVariableType = {ns0Uri, "i=63"};
        const ModelOpcUa::NodeId_t NodeId_BaseDataType {ns0Uri, "i=24"};
        const ModelOpcUa::NodeId_t NodeId_Structure {ns0Uri, "i=22"};
        const ModelOpcUa::NodeId_t NodeId_OPC_Binary {ns0Uri, "i=93"};
        const ModelOpcUa::NodeId_t NodeId_DataTypeDescriptionType {"", "i=69"};
        const ModelOpcUa::NodeId_t NodeId_Enumeration {ns0Uri, "i=29"};
        const ModelOpcUa::NodeId_t NodeId_HasDescription {ns0Uri, "i=39"};
        const ModelOpcUa::NodeId_t NodeId_HasEncoding {ns0Uri, "i=38"};
        const ModelOpcUa::NodeId_t NodeId_BaseObjectType = {ns0Uri, "i=58"};
        const ModelOpcUa::NodeId_t NodeId_Folder = {ns0Uri, "i=61"};
        const ModelOpcUa::NodeId_t NodeId_UndefinedType = {ns0Uri, "i=0"};
        const ModelOpcUa::NodeId_t NodeId_MissingType = {"", "i=0"};
        const std::string nsUriMachinery = "http://opcfoundation.org/UA/Machinery/";
        const ModelOpcUa::NodeId_t NodeId_MachinesFolder = {nsUriMachinery, "i=1001"};
        const ModelOpcUa::NodeId_t NodeId_Machinery_MachineIdentificationType = {nsUriMachinery, "i=1012"};
        const ModelOpcUa::QualifiedName_t QualifiedName_ComponentsFolder = {nsUriMachinery, "Components"};
        const ModelOpcUa::QualifiedName_t QualifiedName_Identification = {"", "Identification"};
    }
} // namespace Umati

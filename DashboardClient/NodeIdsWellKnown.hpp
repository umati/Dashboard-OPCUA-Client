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

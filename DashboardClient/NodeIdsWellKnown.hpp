#include <ModelOpcUa/ModelDefinition.hpp>
namespace Umati
{
    namespace Dashboard
    {
        const std::string ns0UriFull = "http://opcfoundation.org/UA/";
        const std::string ns0Uri = "http://opcfoundation.org/UA/"; // Omitt for node Ids?
        const ModelOpcUa::NodeId_t NodeId_HasComponent = {ns0Uri, "i=47"};
        const ModelOpcUa::NodeId_t NodeId_HierarchicalReferences = {ns0Uri, "i=33"};
        const ModelOpcUa::NodeId_t NodeId_BaseVariableType = {ns0Uri, "i=63"};
        const ModelOpcUa::NodeId_t NodeId_BaseObjectType = {ns0Uri, "i=58"};
    }
} // namespace Umati

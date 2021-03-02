#include "Topics.hpp"
#include <IdEncode.hpp>

namespace Umati
{
    namespace MachineObserver
    {
        std::string const Topics::Prefix = "/umati";
        std::string Topics::Machine(
            const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
            const std::string &namespaceUri)
        {
            std::string specification = p_type->SpecifiedBrowseName.Name;
            std::stringstream topic;
            topic << Topics::Prefix << "/" << specification << "/" << Umati::Util::IdEncode(namespaceUri);
            return topic.str();
        }

        std::string Topics::List(const std::string &specType)
        {
            std::stringstream topic;
            topic << Topics::Prefix << "/list/" << specType;
            return topic.str();
        }
    } // namespace MachineObserver
} // namespace Umati

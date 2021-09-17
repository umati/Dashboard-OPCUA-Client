#pragma once
#include <string>
#include <ModelOpcUa/ModelDefinition.hpp>

namespace Umati
{
    namespace MachineObserver
    {
        class Topics
        {
        public:
            static std::string const Prefix;
            static std::string Machine(
                const std::shared_ptr<ModelOpcUa::StructureNode> &p_type,
                const std::string &namespaceUri);
            static std::string List(const std::string &specType);
            static std::string ErrorList(const std::string &specType);
        };
    } // namespace MachineObserver
} // namespace Umati

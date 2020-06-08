#pragma once

#include <ModelOpcUa/ModelInstance.hpp>
#include <nlohmann/json.hpp>
#include <functional>

namespace Umati
{
	namespace Dashboard
	{
		namespace Converter
		{
			class ModelToJson {
			public:
				typedef std::function<nlohmann::json (const std::shared_ptr<const ModelOpcUa::Node>)> getValue_t;
                /**
                * ModelToJson converts OpcUa nodes to json. ModelToJson calls itself recursively.
                * The output json contains the fields
                * - nodeClass
                * and if mandatory / optiona modeling rule
                * - nodeId
                * - specifiedTypeNodeId
                * - value (if nodeClass is variable)
                * - children (if node contains children)
                * or if mandatoryPlaceholder / optionalPlaceholder modeling rule
                * - placeholderElements
                */
				ModelToJson(const std::shared_ptr<const ModelOpcUa::Node> pNode, getValue_t getValue, bool serializeNodeInformation = false, bool nestAsChildren = false);

				nlohmann::json getJson()
				{
					return m_json;
				}

			protected:
				std::string nodeClassToString(ModelOpcUa::NodeClass_t nodeClass);
				nlohmann::json m_json;
			};
		}
	}
}

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
				ModelToJson(const std::shared_ptr<const ModelOpcUa::Node> pNode, getValue_t getValue);

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

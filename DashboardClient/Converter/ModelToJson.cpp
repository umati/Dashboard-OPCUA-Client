#include "ModelToJson.hpp"
#include <easylogging++.h>

namespace Umati
{
	namespace Dashboard
	{
		namespace Converter
		{
			ModelToJson::ModelToJson(
				const std::shared_ptr<const ModelOpcUa::Node> pNode,
				getValue_t getValue)
			{
				m_json["nodeClass"] = nodeClassToString(pNode->NodeClass);

				switch (pNode->ModellingRule)
				{
				case ModelOpcUa::ModellingRule_t::Mandatory:
				case ModelOpcUa::ModellingRule_t::Optional:
				{
					auto pSimpleNode = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(pNode);
					if (!pSimpleNode)
					{
						LOG(ERROR) << "Simple node error, instance not a simple node." << std::endl;
						break;
					}
					
					m_json["nodeId"] = static_cast<std::string> (pSimpleNode->NodeId);
					m_json["specifiedTypeNodeId"] = static_cast<std::string> (pSimpleNode->SpecifiedTypeNodeId);

					if (pSimpleNode->NodeClass == ModelOpcUa::NodeClass_t::Variable)
					{
						m_json["value"] = getValue(pNode);
					}

					nlohmann::json children;

					for (const auto &pChild : pSimpleNode->ChildNodes)
					{
						children[pChild->SpecifiedBrowseName.Name] = (ModelToJson(pChild, getValue).getJson());
					}
					if (!children.empty())
					{
						m_json["children"] = children;
					}
					
					break;
				}
				case ModelOpcUa::ModellingRule_t::MandatoryPlaceholder:
				case ModelOpcUa::ModellingRule_t::OptionalPlaceholder:
				{
					auto pPlaceholderNode = std::dynamic_pointer_cast<const ModelOpcUa::PlaceholderNode>(pNode);
					if (!pPlaceholderNode)
					{
						LOG(ERROR) << "Placeholder error, instance not a placeholder." << std::endl;
						break;
					}

					auto placeholderElements = pPlaceholderNode->getInstances();
					
					std::list<nlohmann::json> placeholderJsonElements;

					for (const auto &pPlaceholderElement : placeholderElements)
					{
						placeholderJsonElements.push_back(ModelToJson(pPlaceholderElement.pNode, getValue).getJson());
					}
					m_json["placeholderElements"] = placeholderJsonElements;
					break;
				}
                    case ModelOpcUa::ModellingRule_t::None: {
                        break;
                    }
				default:
					LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
					break;
				}
			}

			std::string ModelToJson::nodeClassToString(ModelOpcUa::NodeClass_t nodeClass)
			{
				switch (nodeClass)
				{
				case ModelOpcUa::Object:
					return "Object";
					break;
				case ModelOpcUa::Variable:
					return "Variable";
					break;
				default:
					return "Unknown";
					break;
				}
			}
		}
	}
}

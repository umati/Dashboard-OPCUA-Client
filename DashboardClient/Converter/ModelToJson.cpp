/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */


#include "ModelToJson.hpp"
#include "../../ModelOpcUa/src/ModelOpcUa/ModelInstance.hpp"
#include <easylogging++.h>

namespace Umati {
	namespace Dashboard {
		namespace Converter {
			ModelToJson::ModelToJson(
					const std::shared_ptr<const ModelOpcUa::Node> &pNode,
					const getValue_t &getValue, bool serializeNodeInformation, bool nestAsChildren,
					bool publishNullValues) {
                // if (serializeNodeInformation) {
                //     m_json["nodeClass"] = nodeClassToString(pNode->NodeClass);
                // }
				switch (pNode->ModellingRule) {
					case ModelOpcUa::ModellingRule_t::Mandatory:
					case ModelOpcUa::ModellingRule_t::Optional: {
						auto pSimpleNode = std::dynamic_pointer_cast<const ModelOpcUa::SimpleNode>(pNode);
						if (!pSimpleNode) {
							LOG(ERROR) << "Simple node error, instance not a simple node." << std::endl;
							break;
						}
						if (serializeNodeInformation) {
							m_json["nodeId"] = static_cast<std::string> (pSimpleNode->NodeId);
							m_json["specifiedTypeNodeId"] = static_cast<std::string> (pSimpleNode->SpecifiedTypeNodeId);
						}

						if (pSimpleNode->NodeClass == ModelOpcUa::NodeClass_t::Variable ||
							pSimpleNode->NodeClass == ModelOpcUa::NodeClass_t::VariableType) {
							auto value = getValue(pNode);
							if (nestAsChildren ||
								(isBaseDataVariableType(pSimpleNode) && !pSimpleNode->ChildNodes.empty())) {
								m_json["value"] = value;
							} else {
								m_json = value;
							}
						}

						nlohmann::json children;

						for (const auto &pChild : pSimpleNode->ChildNodes) {
							auto json = (ModelToJson(pChild, getValue, serializeNodeInformation, nestAsChildren,
													 publishNullValues).getJson());
							if (publishNullValues || json.dump(0) != "null") {
								children[pChild->SpecifiedBrowseName.Name] = json;
							}
							if (pSimpleNode->SpecifiedBrowseName.Name == "Results") {
								// LOG(INFO) << "TODO: Fix Sample-Server? -> Here we have one child more than expected?";
								break;
							}
						}
						if (!children.empty()) {
							if (nestAsChildren) {
								m_json["children"] = children;
							} else if (isBaseDataVariableType(pSimpleNode)) {
								m_json["properties"] = children;
							} else {
								m_json = children;
							}
						}
						break;
					}
					case ModelOpcUa::ModellingRule_t::MandatoryPlaceholder:
					case ModelOpcUa::ModellingRule_t::OptionalPlaceholder: {
						auto pPlaceholderNode = std::dynamic_pointer_cast<const ModelOpcUa::PlaceholderNode>(pNode);
						if (!pPlaceholderNode) {
							LOG(ERROR) << "Placeholder error, instance not a placeholder." << std::endl;
							break;
						}

						auto placeholderElements = pPlaceholderNode->getInstances();

						nlohmann::json placeholderJsonElements;

						for (const auto &pPlaceholderElement : placeholderElements) {
                            auto el = (ModelToJson(
                                    pPlaceholderElement.pNode, getValue, serializeNodeInformation, nestAsChildren,
                                    publishNullValues).getJson());
                            el["$TypeDefinition"] = static_cast<std::string>(pPlaceholderElement.TypeDefinition);
							placeholderJsonElements[pPlaceholderElement.BrowseName.Name] = el;
						}
						if (serializeNodeInformation) {
							m_json["placeholderElements"] = placeholderJsonElements;
						} else {
							m_json = placeholderJsonElements;
						}
						break;
					}
					case ModelOpcUa::ModellingRule_t::None: {

					}
					default:
						LOG(ERROR) << "Unknown Modelling Rule." << std::endl;
						return;
				}
			}
			//TODO use another function to check for i=17570 aka AnalogUnitRangeType and i=2755 aka StateVariableType
			//Set ofBaseDataVariableType somewhere?
			bool ModelToJson::isBaseDataVariableType(
					const std::shared_ptr<const ModelOpcUa::SimpleNode> &pSimpleNode) {
				return (pSimpleNode->SpecifiedTypeNodeId.Uri == "" 
						&& (pSimpleNode->SpecifiedTypeNodeId.Id == "i=63" || pSimpleNode->SpecifiedTypeNodeId.Id == "i=17570"
                                                                          || pSimpleNode->SpecifiedTypeNodeId.Id == "i=2755"
                                                                          || pSimpleNode->SpecifiedTypeNodeId.Id == "i=17497"
                                                                          || pSimpleNode->SpecifiedTypeNodeId.Id == "i=15318"))
					   	|| pSimpleNode->ofBaseDataVariableType;
			}

			std::string ModelToJson::nodeClassToString(ModelOpcUa::NodeClass_t nodeClass) {
				switch (nodeClass) {
					case ModelOpcUa::Object:
						return "Object";
					case ModelOpcUa::Variable:
						return "Variable";
					default:
						LOG(INFO) << "ModelToJson does not handle " << nodeClass;
						return "Unknown";
				}
			}
		}
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */
#pragma once

#include "Configuration.hpp"
#include <string>
#include <nlohmann/json.hpp>

namespace ModelOpcUa
{
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NodeId_t, Uri, Id);
}
namespace Umati {
	namespace Util {
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(MqttConfig, Hostname, Port, Username, Password, Prefix, Protocol);
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(OpcUaConfig, Endpoint, Username, Password, Security, ByPassCertVerification);
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(NamespaceInformation, Namespace, Types, IdentificationType);

		class ConfigurationJsonFile : public Configuration {
		public:
			explicit ConfigurationJsonFile(const std::string &filename);
			// Inherit from Configuration
			OpcUaConfig getOpcUa() override;
			MqttConfig getMqtt() override;
			std::vector<NamespaceInformation> getNamespaceInformations() override;
			std::vector<std::string> getObjectTypeNamespaces() override;
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConfigurationJsonFile, OpcUa, ObjectTypeNamespaces, NamespaceInformations, Mqtt)
		protected:
			nlohmann::json getValueOrException(nlohmann::json json, std::string key);
			ConfigurationJsonFile() = default;
			OpcUaConfig OpcUa;
			std::vector<std::string> ObjectTypeNamespaces;
			std::vector<NamespaceInformation> NamespaceInformations;
			MqttConfig Mqtt;
		};
	}
}

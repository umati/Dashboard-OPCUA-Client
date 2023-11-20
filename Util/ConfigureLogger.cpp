/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */
#include "ConfigureLogger.hpp"

#include <easylogging++.h>

// Initiliaze Logger once
INITIALIZE_EASYLOGGINGPP
//INITIALIZE_NULL_EASYLOGGINGPP;

namespace Umati {
	namespace Util {

		void ConfigureLogger(const std::string &name) {
			//el::Helpers::setStorage(el::base::type::StoragePointer());

			el::Configurations conf;
			conf.setToDefault();
			conf.parseFromText(
					R"LOG_CONFIG(
* GLOBAL:
  FORMAT = "%datetime [%logger] %level %func:%line %msg"
  ENABLED = true
  TO_FILE = true
  TO_STANDARD_OUTPUT = true
  SUBSECOND_PRECISION = 3
  PERFORMANCE_TRACKING = false
  MAX_LOG_FILE_SIZE = 2097152
* DEBUG:
  TO_STANDARD_OUTPUT = false
* TRACE:
  TO_STANDARD_OUTPUT = false
* VERBOSE:
  TO_STANDARD_OUTPUT = false
* INFO:
  TO_STANDARD_OUTPUT = true
)LOG_CONFIG");
			conf.setGlobally(el::ConfigurationType::Filename, name + "-%datetime{%Y%M%d}.log");

			el::Loggers::reconfigureLogger(ELPP_DEFAULT_LOGGER, conf);
			LOG(INFO) << "Logger Configured: " << name;
		}

	}
}

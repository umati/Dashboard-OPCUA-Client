#include "ConfigureLogger.hpp"

#include <easylogging++.h>

// Initiliaze Logger once
INITIALIZE_EASYLOGGINGPP;
//INITIALIZE_NULL_EASYLOGGINGPP;
namespace Util
{

	void ConfigureLogger(std::string name)
	{
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
)LOG_CONFIG");
		conf.setGlobally(el::ConfigurationType::Filename, name + "-%datetime{%Y%M%d}.log");

		el::Loggers::reconfigureLogger(ELPP_DEFAULT_LOGGER, conf);
		LOG(INFO) << "Logger Configured: " << name;
	}

}


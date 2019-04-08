#include "UaToModelConverter.hpp"
#include <easylogging++.h>

namespace Umati {
	namespace OpcUa
	{
		namespace Converter {

			UaToModelConverter::UaToModelConverter(const std::map<uint16_t, std::string>& idToUri) : m_idToUri(idToUri)
			{
			}


			std::string UaToModelConverter::getUriFromNsIndex(uint16_t nsIndex)
			{
				if (nsIndex == 0)
				{
					return std::string();
				}

				auto it = m_idToUri.find(nsIndex);
				if (it == m_idToUri.end())
				{
					/// \todo errror handling
					LOG(ERROR) << "Could not find nsIndex: " << nsIndex << std::endl;
					return std::string();
				}

				return it->second;
			}
		}
	}
}

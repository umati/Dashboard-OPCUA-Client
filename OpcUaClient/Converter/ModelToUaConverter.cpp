#include "ModelToUaConverter.hpp"
#include <easylogging++.h>

namespace Umati {
	namespace OpcUa
	{
		namespace Converter {
			ModelToUaConverter::ModelToUaConverter(const std::map<std::string, uint16_t>& uriToID) : m_uriToID(uriToID)
			{
			}

            ModelToUaConverter::~ModelToUaConverter() {}

			uint16_t ModelToUaConverter::getNsIndexFromUri(std::string uri)
			{
				if (uri.empty())
				{
					return 0;
				}

				auto it = m_uriToID.find(uri);
				if (it == m_uriToID.end())
				{
					/// \todo errror handling
					LOG(ERROR) << "Could not find uri: " << uri << std::endl;
					return 0;
				}

				return it->second;
			}

		}
	}
}

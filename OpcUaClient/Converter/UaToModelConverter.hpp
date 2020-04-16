#pragma once

#include <string>
#include <map>

namespace Umati {
	namespace OpcUa
	{
		namespace Converter {
			class UaToModelConverter
			{
			public:
				UaToModelConverter(const std::map<uint16_t, std::string> &idToUri);

				virtual ~UaToModelConverter() = 0;
			protected:

				std::string getUriFromNsIndex(uint16_t nsIndex);

				const std::map<uint16_t, std::string> &m_idToUri;
			};
		}
	}
}

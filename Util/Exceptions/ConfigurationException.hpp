#include <stdexcept>


namespace Umati
{
	namespace Util
	{
		namespace Exception
		{
			class ConfigurationException : public std::exception
			{
				using std::exception::exception;
			};
		}
	}
}

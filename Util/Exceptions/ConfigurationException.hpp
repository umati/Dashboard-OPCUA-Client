#include <stdexcept>


namespace Umati {
	namespace Util {
		namespace Exception {
			class ConfigurationException : public std::exception {
			public:
				explicit ConfigurationException(const char *message) :
						msg_(message) {
				}

				virtual ~ConfigurationException() throw() {}

			protected:
				/** Error message.
				 */
				std::string msg_;
			};
		}
	}
}

#include <stdexcept>


namespace Umati {
	namespace Util {
		namespace Exception {
			class ConfigurationException : public std::exception {
			public:
				explicit ConfigurationException(const char *message) :
						msg_(message) {
				}

				~ConfigurationException() noexcept override;

			protected:
				/** Error message.
				 */
				std::string msg_;
			};

			ConfigurationException::~ConfigurationException() noexcept {}
		}
	}
}

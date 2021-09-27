/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

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

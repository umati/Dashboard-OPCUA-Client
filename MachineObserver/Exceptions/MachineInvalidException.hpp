 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Moritz Walker, ISW University of Stuttgart (for umati and VDW e.V.)
 */

#pragma once

#include <stdexcept>
#include <Exceptions/UmatiException.hpp>

namespace Umati {
	namespace MachineObserver {
		namespace Exceptions {
			class MachineInvalidException : public Umati::Exceptions::UmatiException {
			public:
				using UmatiException::UmatiException;
			};

            class MachineInvalidChildException : public Umati::Exceptions::UmatiException {
            public:
                using UmatiException::UmatiException;
                bool hasInvalidMandatoryChild;
                MachineInvalidChildException(const std::string& err, bool hasInvalidMandatoryChild): hasInvalidMandatoryChild(hasInvalidMandatoryChild), Umati::Exceptions::UmatiException(err) {};
            };
		}
	}
}

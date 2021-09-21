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

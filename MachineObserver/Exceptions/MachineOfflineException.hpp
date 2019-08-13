#pragma once

#include <stdexcept>
#include <Exceptions/UmatiException.hpp>

namespace Umati
{
	namespace MachineObserver
	{
		namespace Exceptions
		{
			class MachineOfflineException : public Umati::Exceptions::UmatiException {
			public:
				using UmatiException::UmatiException;
			};
		}
	}
}

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "uaclientsdk_mock.hpp"
#include "../../_install-Debug/include/gmock/gmock-spec-builders.h"

#include <ConfigureLogger.hpp>
#include <OpcUaClient.hpp>
#include <algorithm>
#include <uadiscovery.h>
#include <uasession.h>
#include <opcua_proxystub.h>
#include <uaplatformlayer.h>
#include <MachineObserver.hpp>

#define OPCUA_TEST_SERVER_URL "opc.tcp://localhost:48010"

using ::testing::Return;
using ::testing::DoAll;
using ::testing::Expectation;
using ::testing::SetArgReferee;
using ::testing::Invoke;
using ::testing::AtLeast;

namespace Umati {
	namespace MachineObserver {
		class TestMachineObserver : public MachineObserver {

		};
	}
}



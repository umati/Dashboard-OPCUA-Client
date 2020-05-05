//
// Created by Dominik on 05.05.2020.
//

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



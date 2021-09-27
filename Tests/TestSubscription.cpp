/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 */

#include <gtest/gtest.h>
#include <Subscription.hpp>
#include <ConfigureLogger.hpp>
#include <opcua_errorhandling.h>
#include <uaplatformlayer.h>
#include <uasession.h>
#include "gmock/gmock.h"
#include "uaclientsdk_mock.hpp"
#include "../../_install-Debug/include/gmock/gmock-spec-builders.h"

using ::testing::Return;
using ::testing::DoAll;
using ::testing::Expectation;
using ::testing::SetArgReferee;
using ::testing::Invoke;
using ::testing::AtLeast;

namespace Umati {
	namespace OpcUa {
		class TestSubscription : public Subscription {

		};
	}
}

TEST(Subscription, subscriptionStatusChanged) {
	Umati::Util::ConfigureLogger("Subscription.subscriptionStatusChanged");

	std::map<std::string, uint16_t> uriToIndexCache;
	std::map<uint16_t, std::string> indexToUriCache;
	Umati::OpcUa::Subscription subscription(uriToIndexCache, indexToUriCache);
	subscription.subscriptionStatusChanged(1, OpcUa_Good);
}

TEST(Subscription, createSubscription) {
	Umati::Util::ConfigureLogger("Subscription.createSubscription");
	UaPlatformLayer::init();

	auto *mockSession = new UaClientSdk::MockUaSession();
	std::shared_ptr<UaClientSdk::UaSession> pMockSession(mockSession);

	Umati::OpcUa::MockOpcUaSubscriptionWrapper mockOpcUaSubscriptionWrapper;
	UaStatus uaStatus1(OpcUa_Good);
	EXPECT_CALL(mockOpcUaSubscriptionWrapper, SessionCreateSubscription).Times(1).WillOnce(Return(uaStatus1));
	std::map<std::string, uint16_t> uriToIndexCache;
	std::map<uint16_t, std::string> indexToUriCache;
	Umati::OpcUa::Subscription subscription(uriToIndexCache, indexToUriCache);
	subscription.setSubscriptionWrapper(&mockOpcUaSubscriptionWrapper);
	subscription.createSubscription(pMockSession);
}

TEST(Subscription, deleteSubscription_mPSubscriptionNull_notCalled) {
	Umati::Util::ConfigureLogger("Subscription.createSubscription");
	UaPlatformLayer::init();

	auto *mockSession = new UaClientSdk::MockUaSession();
	std::shared_ptr<UaClientSdk::UaSession> pMockSession(mockSession);

	Umati::OpcUa::MockOpcUaSubscriptionWrapper mockOpcUaSubscriptionWrapper;
	UaStatus uaStatus1(OpcUa_Good);
	EXPECT_CALL(mockOpcUaSubscriptionWrapper, SessionDeleteSubscription).Times(0);
	std::map<std::string, uint16_t> uriToIndexCache;
	std::map<uint16_t, std::string> indexToUriCache;
	Umati::OpcUa::Subscription subscription(uriToIndexCache, indexToUriCache);
	subscription.setSubscriptionWrapper(&mockOpcUaSubscriptionWrapper);
	subscription.deleteSubscription(pMockSession);
}
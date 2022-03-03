 /* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 * 
 * Copyright 2019-2021 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#include "DashboardOpcUaClient.hpp"

DashboardOpcUaClient::DashboardOpcUaClient(std::shared_ptr<Umati::Util::Configuration> configuration, std::function<void()> issueReset):
m_issueReset(issueReset),
m_opcUaWrapper(std::make_shared<Umati::OpcUa::OpcUaWrapper>()),
m_pClient(std::make_shared<Umati::OpcUa::OpcUaClient>(
        configuration->getOpcUa().Endpoint,
        issueReset,
        configuration->getOpcUa().Username,
        configuration->getOpcUa().Password,
        configuration->getOpcUa().Security,
        configuration->getObjectTypeNamespaces(),
        m_opcUaWrapper)),
m_pPublisher(std::make_shared<Umati::MqttPublisher_Paho::MqttPublisher_Paho>(
        configuration->getMqtt().Hostname,
        configuration->getMqtt().Port,
        configuration->getMqtt().Username,
        configuration->getMqtt().Password)),
m_pOpcUaTypeReader(std::make_shared<Umati::Dashboard::OpcUaTypeReader>(
        m_pClient,
        configuration->getObjectTypeNamespaces(),
        configuration->getNamespaceInformations()))
{

}

bool DashboardOpcUaClient::connect(std::atomic_bool &running) {
    std::size_t i = 0;
    while (running && !m_pClient->isConnected() && i < 60)
    {
        LOG(INFO) << "Waiting for OPC UA connection.";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++i;
    }
    return m_pClient->isConnected();
}

void DashboardOpcUaClient::ReadTypes() {
    m_pOpcUaTypeReader->readTypes();
}

void DashboardOpcUaClient::ReadTypeDictionaries() {
    m_pOpcUaTypeReader->readTypeDictionaries();
    
}

void DashboardOpcUaClient::StartMachineObserver() {
    m_pMachineObserver = std::make_shared<Umati::MachineObserver::DashboardMachineObserver>(
        m_pClient,
        m_pPublisher,
        m_pOpcUaTypeReader);
    m_lastPublish = std::chrono::steady_clock::now();
    m_lastConnectionVerify = std::chrono::steady_clock::now();
}

void DashboardOpcUaClient::Iterate() {
    
    {
        std::lock_guard<std::recursive_mutex> l(m_pClient->m_clientMutex);
        auto retval = UA_Client_run_iterate(m_pClient->m_pClient.get(), 100);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto currentTime = std::chrono::steady_clock::now();
    auto pudDiff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastPublish).count();
    if (pudDiff_ms > 900)
    {
        m_lastPublish = currentTime;
        m_pMachineObserver->PublishAll();
    }

    auto diffConnVerify_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastConnectionVerify).count();
    if (diffConnVerify_ms > 30000)
    {
        m_lastConnectionVerify = currentTime;
        if(!m_pClient->VerifyConnection()) {
            m_issueReset();
        }
    }
}

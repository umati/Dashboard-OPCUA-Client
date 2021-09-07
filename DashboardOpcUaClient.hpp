#pragma once

#include "OpcUaClient/OpcUaClient.hpp"
#include <DashboardClient.hpp>
#include <OpcUaTypeReader.hpp>
#include <MqttPublisher_Paho.hpp>
#include <DashboardMachineObserver.hpp>
#include "Util/Configuration.hpp"
#include <memory>
#include <functional>

class DashboardOpcUaClient {
public:
    DashboardOpcUaClient(std::shared_ptr<Umati::Util::Configuration> configuration, std::function<void()> issueReset);

    bool connect(std::atomic_bool &running);
    void ReadTypes();
    void StartMachineObserver();
    void Iterate();
protected:
    std::function<void()> m_issueReset;
    std::shared_ptr<Umati::OpcUa::OpcUaInterface> m_opcUaWrapper;
    std::shared_ptr<Umati::OpcUa::OpcUaClient> m_pClient;
    std::shared_ptr<Umati::MqttPublisher_Paho::MqttPublisher_Paho> m_pPublisher;
    std::shared_ptr<Umati::Dashboard::OpcUaTypeReader> m_pOpcUaTypeReader;
    std::shared_ptr<Umati::MachineObserver::DashboardMachineObserver> m_pMachineObserver;
    std::chrono::time_point<std::chrono::steady_clock> m_lastPublish;
    std::chrono::time_point<std::chrono::steady_clock> m_lastConnectionVerify;
};

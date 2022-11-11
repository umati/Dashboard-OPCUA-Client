/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2020 (c) Dominik Basner, Sotec GmbH (for VDW e.V.)
 * Copyright 2020-2022 (c) Christian von Arnim, ISW University of Stuttgart (for umati and VDW e.V.)
 * Copyright 2021 (c) Marius Dege, basysKom GmbH
 */

#ifndef DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP
#define DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP

#include <vector>
#include <easylogging++.h>
#include <utility>
#include "Subscription.hpp"

namespace Umati {
namespace OpcUa {
class OpcUaInterface {
 public:
  virtual UA_StatusCode DiscoveryGetEndpoints(
    UA_Client *client, const open62541Cpp::UA_String *sDiscoveryURL, size_t *endpointDescriptionsSize, UA_EndpointDescription **endpointDescriptions) = 0;

  virtual UA_StatusCode DiscoveryFindServers(
    UA_Client *client, const open62541Cpp::UA_String &sDiscoveryURL, size_t *registerdServerSize, UA_ApplicationDescription **applicationDescriptions) = 0;

  virtual UA_StatusCode SessionConnect(
    UA_Client *client, const open62541Cpp::UA_String &sURL
    /*,UA_CreateSessionRequest &sessionConnectInfo*/) = 0;

  virtual UA_StatusCode SessionConnectUsername(UA_Client *client, const open62541Cpp::UA_String &sURL, std::string username, std::string password) = 0;

  virtual UA_StatusCode SessionDisconnect(UA_Client *client, UA_Boolean bDeleteSubscriptions) = 0;

  virtual void SessionUpdateNamespaceTable(UA_Client *client) = 0;

  virtual std::vector<std::string> SessionGetNamespaceTable() = 0;

  virtual UA_ReadResponse SessionRead(
    UA_Client *client,
    UA_Double maxAge,
    UA_TimestampsToReturn timeStamps,
    UA_ReadValueId *nodesToRead,
    size_t nodesToReadSize,
    UA_DiagnosticInfo &diagnosticInfos) = 0;

  virtual bool SessionIsConnected(UA_Client *client) = 0;

  virtual UA_BrowseResponse SessionBrowse(
    UA_Client *client,
    const open62541Cpp::UA_NodeId &nodeToBrowse,
    const UA_BrowseDescription &browseContext,
    UA_ByteString &continuationPoint,
    std::vector<UA_ReferenceDescription> &referenceDescriptions) = 0;

  virtual UA_StatusCode SessionTranslateBrowsePathsToNodeIds(
    UA_Client *client, UA_BrowsePath &browsePaths, UA_BrowsePathResult &browsePathResults, UA_DiagnosticInfo &diagnosticInfos) = 0;

  virtual void setSubscription(Subscription *p_in_subscr) = 0;

  virtual void SubscriptionCreateSubscription(UA_Client *client) = 0;

  virtual std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> SubscriptionSubscribe(
    UA_Client *client, ModelOpcUa::NodeId_t nodeId, Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback) = 0;

  virtual void SubscriptionUnsubscribe(UA_Client *client, std::vector<int32_t> monItemIds, std::vector<int32_t> clientHandles) = 0;

 protected:
  std::vector<std::string> namespaceArray;
  Subscription *p_subscr;
};

class OpcUaWrapper : public OpcUaInterface {
 public:
  UA_StatusCode DiscoveryGetEndpoints(
    UA_Client *client, const open62541Cpp::UA_String *sDiscoveryURL, size_t *endpointDescriptionsSize, UA_EndpointDescription **endpointDescriptions) override {
    return UA_Client_getEndpoints(client, static_cast<std::string>(*sDiscoveryURL).c_str(), endpointDescriptionsSize, endpointDescriptions);
  }

  UA_StatusCode DiscoveryFindServers(
    UA_Client *client,
    const open62541Cpp::UA_String &sDiscoveryURL,
    size_t *registerdServerSize,
    UA_ApplicationDescription **applicationDescriptions) override {
    return UA_Client_findServers(
      client, (char *)sDiscoveryURL.String->data, sDiscoveryURL.String->length, nullptr, 0, nullptr, registerdServerSize, applicationDescriptions);
  }

  UA_StatusCode SessionConnect(UA_Client *client, const open62541Cpp::UA_String &sURL) override {
    return UA_Client_connect(client, static_cast<std::string>(sURL).c_str());
  }

  UA_StatusCode SessionConnectUsername(UA_Client *client, const open62541Cpp::UA_String &sURL, std::string username, std::string password) override {
    return UA_Client_connectUsername(client, static_cast<std::string>(sURL).c_str(), username.c_str(), password.c_str());
  }

  UA_StatusCode SessionDisconnect(UA_Client *client, UA_Boolean bDeleteSubscriptions) override { return UA_Client_disconnect(client); }

  UA_ReadResponse SessionRead(
    UA_Client *client,
    UA_Double maxAge,
    UA_TimestampsToReturn timeStamps,
    UA_ReadValueId *nodesToRead,
    size_t nodesToReadSize,
    UA_DiagnosticInfo &diagnosticInfos) override {
    UA_ReadRequest req;
    UA_ReadRequest_init(&req);
    UA_Double_copy(&maxAge, &req.maxAge);
    UA_TimestampsToReturn_copy(&timeStamps, &req.timestampsToReturn);
    req.nodesToReadSize = nodesToReadSize;
    req.nodesToRead = nodesToRead;

    return UA_Client_Service_read(client, req);
  }

  bool SessionIsConnected(UA_Client *client) override {
    UA_SecureChannelState secureChannelState;
    UA_SessionState sessionState;
    UA_StatusCode statusCode;

    UA_Client_getState(client, &secureChannelState, &sessionState, &statusCode);
    setChannelState(secureChannelState);
    setSessionState(sessionState);
    if (!UA_StatusCode_isBad(statusCode) && sessionState == UA_SESSIONSTATE_ACTIVATED && secureChannelState == UA_SECURECHANNELSTATE_OPEN) return true;
    return false;
  }

  void setChannelState(UA_SecureChannelState channelState) { m_pChannelState = channelState; }

  void setSessionState(UA_SessionState sessionState) { m_pSessionState = sessionState; }

  void SessionUpdateNamespaceTable(UA_Client *client) override {
    namespaceArray.clear();
    open62541Cpp::UA_NodeId node = open62541Cpp::UA_NodeId(UA_UInt16(0), UA_UInt32(2255));
    UA_ReadRequest request;
    UA_ReadRequest_init(&request);
    request.nodesToRead = UA_ReadValueId_new();
    request.nodesToRead->attributeId = UA_ATTRIBUTEID_VALUE;
    UA_NodeId_copy(node.NodeId, &request.nodesToRead->nodeId);
    request.nodesToReadSize = 1;
    UA_ReadResponse response = UA_Client_Service_read(client, request);

    UA_String *ns = (UA_String *)response.results[0].value.data;
    for (size_t i = 0; i < response.results[0].value.arrayLength; ++i) {
      namespaceArray.push_back(std::string(ns[i].data, ns[i].data + ns[i].length));
    }
    UA_ReadRequest_clear(&request);
    UA_ReadResponse_clear(&response);
  }

  std::vector<std::string> SessionGetNamespaceTable() override { return namespaceArray; }
  UA_BrowseResponse SessionBrowse(
    UA_Client *client,
    const open62541Cpp::UA_NodeId &nodeToBrowse,
    const UA_BrowseDescription &browseContext,
    UA_ByteString &continuationPoint,
    std::vector<UA_ReferenceDescription> &referenceDescriptions) override {
    UA_NodeId tmpNode;
    UA_NodeId_init(&tmpNode);
    UA_NodeId_copy(nodeToBrowse.NodeId, &tmpNode);

    UA_BrowseRequest browseRequest;
    UA_BrowseRequest_init(&browseRequest);
    browseRequest.requestedMaxReferencesPerNode = 0;
    browseRequest.nodesToBrowse = UA_BrowseDescription_new();
    browseRequest.nodesToBrowseSize = 1;
    *browseRequest.nodesToBrowse = browseContext;
    browseRequest.nodesToBrowse[0].nodeId = tmpNode;
    UA_BrowseResponse browseResponse;
    UA_BrowseResponse_init(&browseResponse);
    browseResponse = UA_Client_Service_browse(client, browseRequest);

    for (size_t i = 0; i < browseResponse.resultsSize; ++i) {
      for (size_t j = 0; j < browseResponse.results[i].referencesSize; ++j) {
        referenceDescriptions.push_back(browseResponse.results[i].references[j]);
      }
    }
    UA_BrowseRequest_clear(&browseRequest);

    return browseResponse;
  }

  UA_StatusCode SessionTranslateBrowsePathsToNodeIds(
    UA_Client *client, UA_BrowsePath &browsePaths, UA_BrowsePathResult &browsePathResults, UA_DiagnosticInfo &diagnosticInfos) override {
    UA_TranslateBrowsePathsToNodeIdsRequest request;
    UA_TranslateBrowsePathsToNodeIdsRequest_init(&request);
    request.browsePaths = &browsePaths;
    request.browsePathsSize = 1;
    auto response = UA_Client_Service_translateBrowsePathsToNodeIds(client, request);
    UA_BrowsePathResult_copy(response.results, &browsePathResults);
    UA_StatusCode retCode = response.results->statusCode;
    UA_TranslateBrowsePathsToNodeIdsResponse_clear(&response);

    return retCode;
  }

  void setSubscription(Subscription *p_in_subscr) override { p_subscr = p_in_subscr; }

  void SubscriptionCreateSubscription(UA_Client *client) override {
    if (p_subscr == nullptr) {
      LOG(ERROR) << "Unable to create subscription, pointer is NULL ";
      exit(SIGTERM);
    }
    p_subscr->createSubscription(client);
  }

  std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle> SubscriptionSubscribe(
    UA_Client *client, ModelOpcUa::NodeId_t nodeId, Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback) override {
    if (p_subscr == nullptr) {
      LOG(ERROR) << "Unable to subscribe, pointer is NULL ";
      exit(SIGTERM);
    }
    try {
      return p_subscr->Subscribe(client, nodeId, callback);
    } catch (std::exception &ex) {
      throw ex;
    }
  }

  void SubscriptionUnsubscribe(UA_Client *client, std::vector<int32_t> monItemIds, std::vector<int32_t> clientHandles) {
    p_subscr->Unsubscribe(client, monItemIds, clientHandles);
  }

  UA_SessionState m_pSessionState;
  UA_SecureChannelState m_pChannelState;
};
}  // namespace OpcUa
}  // namespace Umati

#endif  // DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP

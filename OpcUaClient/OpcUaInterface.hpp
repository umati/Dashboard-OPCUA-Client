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

			static Umati::Dashboard::IDashboardDataClient::eventCallbackFunction_t s_eventcallback;

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

			virtual std::shared_ptr<Dashboard::IDashboardDataClient::EventSubscriptionHandle> EventSubscribe(UA_Client* client,
			Dashboard::IDashboardDataClient::eventCallbackFunction_t eventcallback) = 0;
			virtual void EventUnsubscribe(UA_Client* client, std::shared_ptr<Dashboard::IDashboardDataClient::EventSubscriptionHandle> eventSubscriptionhandle)= 0;

		protected:
			std::vector<std::string> namespaceArray;
			Subscription *p_subscr;
			Dashboard::IDashboardDataClient::eventCallbackFunction_t eventcallback;
		};

		class OpcUaWrapper : public OpcUaInterface {
			

		public:
			
			UA_StatusCode DiscoveryGetEndpoints(UA_Client *client, const open62541Cpp::UA_String *sDiscoveryURL,
										   size_t *endpointDescriptionsSize,
										   UA_EndpointDescription **endpointDescriptions) override {		
				return UA_Client_getEndpoints(client, static_cast<std::string>(*sDiscoveryURL).c_str() , endpointDescriptionsSize, endpointDescriptions);
			};

			UA_StatusCode DiscoveryFindServers(UA_Client *client, const open62541Cpp::UA_String &sDiscoveryURL,
											size_t *registerdServerSize,
											UA_ApplicationDescription **applicationDescriptions) override {
				 
				return UA_Client_findServers(client,(char *)sDiscoveryURL.String->data,sDiscoveryURL.String->length, nullptr, 0, nullptr, registerdServerSize,
											 applicationDescriptions);
			};

			UA_StatusCode SessionConnect(
									UA_Client *client,
									const open62541Cpp::UA_String &sURL) override {
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

			void SubscriptionUnsubscribe(UA_Client *client, std::vector<int32_t> monItemIds, std::vector<int32_t> clientHandles){
				p_subscr->Unsubscribe(client, monItemIds, clientHandles);
			}

			static void handler_events(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, size_t nEventFields, UA_Variant *eventFields) {
			UA_NodeId* sourceNodeId;
			UA_NodeId affectedNode;
			UA_NodeId affectedType;
			UA_Byte verb;
			for(size_t i = 0; i < nEventFields; ++i) {
				UA_Variant eventField = eventFields[i];
        		if(UA_Variant_hasScalarType(&eventField, &UA_TYPES[UA_TYPES_NODEID])) {
					UA_NodeId* nodeId = (UA_NodeId *)eventField.data;
					if(i == 1) {
						sourceNodeId = nodeId;
					}
				}  else {
					UA_ExtensionObject* extensionObjects = (UA_ExtensionObject*)eventField.data;
					UA_ModelChangeStructureDataType* modelChangeStructureDataTypes = (UA_ModelChangeStructureDataType*)UA_Array_new(eventField.arrayLength, &UA_TYPES[UA_TYPES_MODELCHANGESTRUCTUREDATATYPE]);
					for(int j = 0; j < eventField.arrayLength; j ++) {
						UA_ExtensionObject extensionObject = extensionObjects[j];
						UA_ModelChangeStructureDataType* modelChangeStructureDataType =(UA_ModelChangeStructureDataType*)extensionObject.content.decoded.data;
						modelChangeStructureDataTypes[j] = *modelChangeStructureDataType;
					}
					OpcUaWrapper* pWrapper = static_cast<OpcUaWrapper*> (monContext);
					if(pWrapper != nullptr && pWrapper->eventcallback != nullptr) {
						for(int i = 0; i < eventField.arrayLength; i++) {
							UA_ModelChangeStructureDataType modelChangeStructureDataType = modelChangeStructureDataTypes[i];
							UA_NodeId affectedNode = modelChangeStructureDataType.affected;
							UA_NodeId affectedType = modelChangeStructureDataType.affectedType;
							UA_Byte verb = modelChangeStructureDataType.verb;
							Umati::Dashboard::IDashboardDataClient::StructureChangeEvent stc;
							ModelOpcUa::NodeId_t nodeId = ModelOpcUa::NodeId_t();
							nodeId.Uri = pWrapper->namespaceArray.at(affectedNode.namespaceIndex);
							std::string nodeIdPrefix = "i=";
							switch(affectedNode.identifierType){
								case UA_NODEIDTYPE_NUMERIC: nodeIdPrefix = "i="; break;
								case UA_NODEIDTYPE_STRING: nodeIdPrefix = "s="; break;
								case UA_NODEIDTYPE_BYTESTRING: nodeIdPrefix = "b="; break;
								case UA_NODEIDTYPE_GUID: nodeIdPrefix = "g="; break;
							}
							nodeId.Id = nodeIdPrefix + std::to_string(affectedNode.identifier.numeric);
							stc.refreshNode = nodeId;
							stc.nodeAdded = (verb & UA_MODELCHANGESTRUCTUREVERBMASK_NODEADDED) == UA_MODELCHANGESTRUCTUREVERBMASK_NODEADDED;
							stc.nodeDeleted = (verb & UA_MODELCHANGESTRUCTUREVERBMASK_NODEDELETED) == UA_MODELCHANGESTRUCTUREVERBMASK_NODEDELETED;
							stc.referenceAdded = (verb & UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEADDED) == UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEADDED;
							stc.referenceDeleted = (verb & UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEDELETED) == UA_MODELCHANGESTRUCTUREVERBMASK_REFERENCEDELETED;
							stc.dataTypeChanged = (verb & UA_MODELCHANGESTRUCTUREVERBMASK_DATATYPECHANGED) == UA_MODELCHANGESTRUCTUREVERBMASK_DATATYPECHANGED;
							pWrapper->eventcallback(stc);
						}	
					} else {
						LOG(ERROR) << "Unable to propagate Event callback!";
					}
					UA_Array_delete(modelChangeStructureDataTypes, eventField.arrayLength, &UA_TYPES[UA_TYPES_MODELCHANGESTRUCTUREDATATYPE]);
				}
			}
		}
			std::shared_ptr<Dashboard::IDashboardDataClient::EventSubscriptionHandle> EventSubscribe(UA_Client* client,
			Dashboard::IDashboardDataClient::eventCallbackFunction_t eventcallback) {
				this->eventcallback = eventcallback;
				UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    			UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request, NULL, NULL, NULL);
				if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
					UA_UInt32 subId = response.subscriptionId;
					UA_MonitoredItemCreateRequest item;
    				UA_MonitoredItemCreateRequest_init(&item);
					item.itemToMonitor.attributeId = UA_ATTRIBUTEID_EVENTNOTIFIER;
					item.monitoringMode = UA_MONITORINGMODE_REPORTING;
    				item.itemToMonitor.nodeId = UA_NODEID_NUMERIC(0, 2253); // Root->Objects->Server
    			
					UA_EventFilter filter;
    				UA_EventFilter_init(&filter);

					//Setup selection Clauses for Filter
					int nSelectClauses = 3;
					UA_SimpleAttributeOperand *selectClauses = (UA_SimpleAttributeOperand*)
        			UA_Array_new(nSelectClauses, &UA_TYPES[UA_TYPES_SIMPLEATTRIBUTEOPERAND]);

					for(size_t i =0; i<nSelectClauses; ++i) {
        				UA_SimpleAttributeOperand_init(&selectClauses[i]);
    				}
					selectClauses[0].typeDefinitionId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
					selectClauses[0].attributeId = UA_ATTRIBUTEID_VALUE;
    				selectClauses[0].browsePathSize = 1;
    				selectClauses[0].browsePath = (UA_QualifiedName*)
       				UA_Array_new(selectClauses[0].browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    				if(!selectClauses[0].browsePath) {
        				UA_SimpleAttributeOperand_delete(selectClauses);
    				}
    				selectClauses[0].browsePath[0] = UA_QUALIFIEDNAME_ALLOC(0, "EventType");
					selectClauses[1].typeDefinitionId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
					selectClauses[1].attributeId = UA_ATTRIBUTEID_VALUE;
    				selectClauses[1].browsePathSize = 1;
    				selectClauses[1].browsePath = (UA_QualifiedName*)
       				UA_Array_new(selectClauses[1].browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);

    				if(!selectClauses[1].browsePath) {
        				UA_SimpleAttributeOperand_delete(selectClauses);
    				}
    				selectClauses[1].browsePath[0] = UA_QUALIFIEDNAME_ALLOC(0, "SourceNode");

					selectClauses[2].typeDefinitionId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEEVENTTYPE);
					selectClauses[2].attributeId = UA_ATTRIBUTEID_VALUE;
    				selectClauses[2].browsePathSize = 1;
    				selectClauses[2].browsePath = (UA_QualifiedName*)
       				UA_Array_new(selectClauses[2].browsePathSize, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    				if(!selectClauses[2].browsePath) {
        				UA_SimpleAttributeOperand_delete(selectClauses);
    				}

    				selectClauses[2].browsePath[0] = UA_QUALIFIEDNAME_ALLOC(0, "Changes");    

					filter.selectClauses = selectClauses;
    				filter.selectClausesSize = 3;
					// Content Filter
					UA_ContentFilter contentFilter;
					UA_ContentFilter_init(&contentFilter);

					UA_ContentFilterElement contentFilterElement;
					UA_ContentFilterElement_init(&contentFilterElement);

					UA_LiteralOperand literalOperand;
					UA_LiteralOperand_init(&literalOperand);
					UA_Variant v;
					UA_NodeId generalModelChangeEventNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_GENERALMODELCHANGEEVENTTYPE);
					UA_Variant_setScalar(&v, &generalModelChangeEventNodeId, &UA_TYPES[UA_TYPES_NODEID]);
					literalOperand.value = v;

					contentFilterElement.filterOperator = UA_FILTEROPERATOR_OFTYPE;
					contentFilterElement.filterOperandsSize = 1;
					contentFilterElement.filterOperands = (UA_ExtensionObject*)
					UA_Array_new(contentFilterElement.filterOperandsSize, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]);
					UA_ExtensionObject extensionObject;
					UA_ExtensionObject_setValue(&extensionObject, &literalOperand, &UA_TYPES[UA_TYPES_LITERALOPERAND]);
					contentFilterElement.filterOperands[0] = extensionObject;
					if(!contentFilterElement.filterOperands) {
						UA_ContentFilterElement_delete(&contentFilterElement);
					}

					contentFilter.elementsSize = 1;
					contentFilter.elements = (UA_ContentFilterElement* ) UA_Array_new(contentFilter.elementsSize, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]);
					contentFilter.elements[0] = contentFilterElement;
					filter.whereClause = contentFilter;


    				item.requestedParameters.filter.encoding = UA_EXTENSIONOBJECT_DECODED;
    				item.requestedParameters.filter.content.decoded.data = &filter;
    				item.requestedParameters.filter.content.decoded.type = &UA_TYPES[UA_TYPES_EVENTFILTER];

    				UA_MonitoredItemCreateResult result = UA_Client_MonitoredItems_createEvent(client, subId, UA_TIMESTAMPSTORETURN_BOTH, item, this, handler_events, NULL);

					if(result.statusCode == UA_STATUSCODE_GOOD) {
       					LOG(INFO) << "Created MonitoredItem";
						return std::make_shared<Dashboard::IDashboardDataClient::EventSubscriptionHandle>(result.monitoredItemId, item.requestedParameters.clientHandle);
					} else {
						LOG(ERROR) << "Unable to create MonitoredItem";
						return NULL;
					}

    			} else {
					return NULL;
				}	
				return NULL;
			}

			void EventUnsubscribe(UA_Client* client, std::shared_ptr<Dashboard::IDashboardDataClient::EventSubscriptionHandle> eventSubscriptionHandle) {
				LOG(INFO) << "Unsubscribe EventCallback";
				UA_DeleteSubscriptionsRequest deleteRequest;
    			UA_DeleteSubscriptionsRequest_init(&deleteRequest);
				deleteRequest.subscriptionIdsSize = 1;
				UA_UInt32* pSubscriptionId = (UA_UInt32 *) UA_Array_new(1, &UA_TYPES[UA_TYPES_UINT32]);
				pSubscriptionId[0] = (UA_UInt32)eventSubscriptionHandle->getSubscriptionId();
				deleteRequest.subscriptionIds = pSubscriptionId;
				UA_DeleteSubscriptionsResponse response = UA_Client_Subscriptions_delete(client, deleteRequest);
				if(UA_STATUSCODE_GOOD == response.results[0]) {
					eventSubscriptionHandle->unsubscribe();
				}
				UA_DeleteSubscriptionsRequest_clear(&deleteRequest);
				UA_DeleteSubscriptionsResponse_clear(&response);
				UA_Array_delete(pSubscriptionId, 1, &UA_TYPES[UA_TYPES_UINT32]);
			};

			UA_SessionState m_pSessionState;
			UA_SecureChannelState m_pChannelState;
		};

	}
}

#endif  // DASHBOARD_OPCUACLIENT_OPCUAINTERFACE_HPP

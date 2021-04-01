//
// Created by Dominik on 24.04.2020.
//
#include <uadiscovery.h>
#include <uasession.h>
#include <OpcUaInterface.hpp>
#include <bits/shared_ptr.h>
#include "gmock/gmock.h"  // Brings in gMock.

#ifndef DASHBOARD_OPCUACLIENT_UACLIENTSDK_MOCK_H
#define DASHBOARD_OPCUACLIENT_UACLIENTSDK_MOCK_H

#endif //DASHBOARD_OPCUACLIENT_UACLIENTSDK_MOCK_H
namespace UaClientSdk {
	class MockUaDiscovery : public UaDiscovery {
	public:

		MOCK_METHOD(UaStatus, findServers, (
				const UaString&            sDiscoveryURL,
				UaApplicationDescriptions& applicationDescriptions));

		MOCK_METHOD(UaStatus, findServers, (
				ServiceSettings & serviceSettings,
				const UaString &sDiscoveryURL,
				ClientSecurityInfo &clientSecurityInfo,
				UaApplicationDescriptions & applicationDescriptions));

		MOCK_METHOD(UaStatus, findServers, (
				ServiceSettings & serviceSettings,
				const UaString &sDiscoveryURL,
				ClientSecurityInfo &clientSecurityInfo,
				const UaStringArray &localIds,
				const UaStringArray &serverUris,
				UaApplicationDescriptions & applicationDescriptions));

		MOCK_METHOD(UaStatus, findServersOnNetwork, (
				ServiceSettings & serviceSettings,
				const UaString &sDiscoveryURL,
				ClientSecurityInfo &clientSecurityInfo,
				OpcUa_UInt32 startingRecordId,
				const UaStringArray &serverCapabilities,
				OpcUa_UInt32 maxRecordsToReturn,
				UaDateTime &lastCounterResetTime,
				UaServerOnNetworks & servers));

		MOCK_METHOD(UaStatus, findServersOnNetwork, (
				ServiceSettings & serviceSettings,
				const UaString &sDiscoveryURL,
				ClientSecurityInfo &clientSecurityInfo,
				OpcUa_UInt32 startingRecordId,
				UaDateTime &lastCounterResetTime,
				UaServerOnNetworks & servers));

		MOCK_METHOD(UaStatus, queryDirectory, (
				ServiceSettings & serviceSettings,
				const UaString &sGdsURL,
				OpcUa_UInt32 startingRecordId,
				UaDateTime &lastCounterResetTime,
				UaServerOnNetworks & servers));

		MOCK_METHOD(UaStatus, queryDirectory, (
				ServiceSettings & serviceSettings,
						UaSession * pSession,
						OpcUa_UInt32 startingRecordId,
				UaDateTime & lastCounterResetTime,
				UaServerOnNetworks & servers));

		MOCK_METHOD(UaStatus, queryDirectory, (
				ServiceSettings & serviceSettings,
						UaSession * pSession,
						OpcUa_UInt32 startingRecordId,
				OpcUa_UInt32 maxRecordsToReturn,
				const UaString &applicationName,
				const UaString &applicationUri,
				const UaString &productUri,
				const UaStringArray &serverCapabilities,
				UaDateTime &lastCounterResetTime,
				UaServerOnNetworks & servers));

		MOCK_METHOD(UaStatus, queryDirectory, (
				ServiceSettings & serviceSettings,
				const UaString &sGdsURL,
				SessionConnectInfo &sessionConnectInfo,
				SessionSecurityInfo &sessionSecurityInfo,
				OpcUa_UInt32 startingRecordId,
				OpcUa_UInt32 maxRecordsToReturn,
				const UaString &applicationName,
				const UaString &applicationUri,
				const UaString &productUri,
				const UaStringArray &serverCapabilities,
				UaDateTime &lastCounterResetTime,
				UaServerOnNetworks & servers));

		MOCK_METHOD(UaStatus, startReverseDiscovery, (
				const UaString& sClientEndpointUrl,
				UaReverseDiscoveryCallback* pReverseConnectCallback));
		MOCK_METHOD(UaStatus, stopReverseDiscovery, ());
		MOCK_METHOD(UaString, getClientEndpointUrl, ());

	};

	class MockUaSession : public UaSession {
	public:

		// Establish a connection to the OPC UA server
		MOCK_METHOD(UaStatus, connect, (
				const UaString&      sURL,
				SessionConnectInfo&  sessionConnectInfo,
				SessionSecurityInfo& sessionSecurityInfo,
				UaSessionCallback*   pSessionCallback), ());

		// Establish a connection to the OPC UA server asynchronous
		MOCK_METHOD(UaStatus, beginConnect, (
				const UaString&      sURL,
				SessionConnectInfo&  sessionConnectInfo,
				SessionSecurityInfo& sessionSecurityInfo,
				UaSessionCallback*   pSessionCallback), ());

		// Disconnect client application from OPC UA server
		MOCK_METHOD(UaStatus, disconnect, (
				ServiceSettings & serviceSettings,
						OpcUa_Boolean bDeleteSubscriptions));

		// Change user for the connection to the OPC UA server
		MOCK_METHOD(UaStatus, changeUser, (
				ServiceSettings & serviceSettings,
						UaUserIdentityToken * pUserIdentityToken));

		// Change user and LocaleIds for the connection to the OPC UA server
		MOCK_METHOD(UaStatus, changeUser, (
				ServiceSettings & serviceSettings,
						UaUserIdentityToken * pUserIdentityToken,
				const UaStringArray &localeIds));

		// Change client certificate for the connection to the OPC UA server
		MOCK_METHOD(UaStatus, changeClientCertificate, (
				const SessionSecurityInfo& sessionSecurityInfo));

		// Browse OPC server address space with one starting node.
		MOCK_METHOD(UaStatus, browse, (
				ServiceSettings & serviceSettings,
				const UaNodeId &nodeToBrowse,
				const BrowseContext &browseContext,
				UaByteString &continuationPoint,
				UaReferenceDescriptions & referenceDescriptions));

		// Continue a previous browse request
		MOCK_METHOD(UaStatus, browseNext, (
				ServiceSettings & serviceSettings,
						OpcUa_Boolean releaseContinuationPoint,
				UaByteString & continuationPoint,
				UaReferenceDescriptions & referenceDescriptions));

		// Browse OPC server address space with a list of starting node.
		MOCK_METHOD(UaStatus, browseList, (
				ServiceSettings & serviceSettings,
				const OpcUa_ViewDescription &viewDescription,
				OpcUa_UInt32 maxReferencesToReturn,
				const UaBrowseDescriptions &browseDescription,
				UaBrowseResults &browseResults,
				UaDiagnosticInfos & diagnosticInfos));

		// Continue a previous browseList request
		MOCK_METHOD(UaStatus, browseListNext, (
				ServiceSettings & serviceSettings,
						OpcUa_Boolean releaseContinuationPoint,
				const UaByteStringArray &continuationPoints,
				UaBrowseResults &browseResults,
				UaDiagnosticInfos & diagnosticInfos));

		// Translate a browse path to a NodeId.
		MOCK_METHOD(UaStatus, translateBrowsePathsToNodeIds, (
				ServiceSettings & serviceSettings,
				const UaBrowsePaths &browsePaths,
				UaBrowsePathResults &browsePathResults,
				UaDiagnosticInfos & diagnosticInfos));

		// Register nodes to create shortcuts in the server.
		MOCK_METHOD(UaStatus, registerNodes, (
				ServiceSettings & serviceSettings,
				const UaNodeIdArray &nodesToRegister,
				UaNodeIdArray & registeredNodesIds));

		// Unregister nodes to delete shortcuts in the server.
		MOCK_METHOD(UaStatus, unregisterNodes, (
				ServiceSettings & serviceSettings,
				const UaNodeIdArray &registeredNodesIds));

		// Reads variable values synchronous from OPC server
		MOCK_METHOD(UaStatus, read, (
				ServiceSettings & serviceSettings,
						OpcUa_Double maxAge,
				OpcUa_TimestampsToReturn timeStamps,
				const UaReadValueIds &nodesToRead,
				UaDataValues &values,
				UaDiagnosticInfos & diagnosticInfos));

		// Reads variable values asynchronous from OPC server
		MOCK_METHOD(UaStatus, beginRead, (
				ServiceSettings & serviceSettings,
						OpcUa_Double maxAge,
				OpcUa_TimestampsToReturn timeStamps,
				const UaReadValueIds &nodesToRead,
				OpcUa_UInt32 transactionId));

		// Writes variable values synchronous to OPC server
		MOCK_METHOD(UaStatus, write, (
				ServiceSettings & serviceSettings,
				const UaWriteValues &nodesToWrite,
				UaStatusCodeArray &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Writes variable values asynchronous to OPC server
		MOCK_METHOD(UaStatus, beginWrite, (
				ServiceSettings & serviceSettings,
				const UaWriteValues &nodesToWrite,
				OpcUa_UInt32 transactionId));

		// Creates a subscription
		MOCK_METHOD(UaStatus, createSubscription, (
				ServiceSettings & serviceSettings,
						UaSubscriptionCallback * pUaSubscriptionCallback,
						OpcUa_UInt32 clientSubscriptionHandle,
				SubscriptionSettings & subscriptionSettings,
				OpcUa_Boolean publishingEnabled,
				UaSubscription * *ppUaSubscription));

		// Deletes a subscription
		MOCK_METHOD(UaStatus, deleteSubscription, (
				ServiceSettings & serviceSettings,
						UaSubscription * *ppUaSubscription,
						OpcUa_UInt32 waitTimeForTransactionCompletion));

		// Transfer a subscription from another client to this session
		MOCK_METHOD(UaStatus, transferSubscription, (
				ServiceSettings & serviceSettings,
						UaSubscriptionCallback * pUaSubscriptionCallback,
						OpcUa_UInt32 clientSubscriptionHandle,
				OpcUa_UInt32 subscriptionId,
				SubscriptionSettings & subscriptionSettings,
				OpcUa_Boolean publishingEnabled,
				OpcUa_Boolean sendInitialValues,
				UaSubscription * *ppUaSubscription,
				UaUInt32Array & availableSequenceNumbers));

		// Calls a method of an Object in the OPC server
		MOCK_METHOD(UaStatus, call, (
				ServiceSettings & serviceSettings,
				const CallIn &callRequest,
				CallOut & results));

		// Calls a method of an Object in the OPC server asynchronous
		MOCK_METHOD(UaStatus, beginCall, (
				ServiceSettings & serviceSettings,
				const CallIn &callRequest,
				OpcUa_UInt32 transactionId));

		// Calls a list of methods of Objects in the OPC server
		MOCK_METHOD(UaStatus, callList, (
				ServiceSettings & serviceSettings,
				const UaCallMethodRequests &callMethodRequests,
				UaCallMethodResults &callMethodResults,
				UaDiagnosticInfos & diagnosticInfos));

		// Reads the history of item values synchronous from OPC server
		MOCK_METHOD(UaStatus, historyReadRawModified, (
				ServiceSettings & serviceSettings,
				const HistoryReadRawModifiedContext &historyReadRawModifiedContext,
				const UaHistoryReadValueIds &nodesToRead,
				HistoryReadDataResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Reads the processed history of item values synchronous from the OPC server based on a passed aggregate
		MOCK_METHOD(UaStatus, historyReadProcessed, (
				ServiceSettings & serviceSettings,
				const HistoryReadProcessedContext &historyReadProcessedContext,
				const UaHistoryReadValueIds &nodesToRead,
				HistoryReadDataResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Reads the history of item values synchronous from OPC server for the given timestamps
		MOCK_METHOD(UaStatus, historyReadAtTime, (
				ServiceSettings & serviceSettings,
				const HistoryReadAtTimeContext &historyReadAtTimeContext,
				const UaHistoryReadValueIds &nodesToRead,
				HistoryReadDataResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Reads the history of events from an event notifier object synchronous from OPC server
		MOCK_METHOD(UaStatus, historyReadEvent, (
				ServiceSettings & serviceSettings,
				const HistoryReadEventContext &historyReadEventContext,
				const UaHistoryReadValueIds &nodesToRead,
				HistoryReadEventResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Update the history of item values synchronous at OPC server
		MOCK_METHOD(UaStatus, historyUpdateData, (
				ServiceSettings & serviceSettings,
				const UpdateDataDetails &updateDataDetails,
				UaHistoryUpdateResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Delete the history of item values synchronous at OPC server
		MOCK_METHOD(UaStatus, historyDeleteRawModified, (
				ServiceSettings & serviceSettings,
				const DeleteRawModifiedDetails &deleteDetails,
				UaHistoryUpdateResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Delete the history of item values synchronous at OPC server for the given timestamps
		MOCK_METHOD(UaStatus, historyDeleteAtTime, (
				ServiceSettings & serviceSettings,
				const DeleteAtTimeDetails &deleteDetails,
				UaHistoryUpdateResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Update the history of events synchronous at OPC server
		MOCK_METHOD(UaStatus, historyUpdateEvents, (
				ServiceSettings & serviceSettings,
				const UpdateEventDetails &updateEventDetails,
				UaHistoryUpdateResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Delete the history of events synchronous at OPC server for the given event Ids
		MOCK_METHOD(UaStatus, historyDeleteEvents, (
				ServiceSettings & serviceSettings,
				const DeleteEventDetails &deleteDetails,
				UaHistoryUpdateResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Reads the history of item values asynchronous from OPC server
		MOCK_METHOD(UaStatus, beginHistoryReadRawModified, (
				ServiceSettings & serviceSettings,
				const HistoryReadRawModifiedContext &historyReadRawModifiedContext,
				const UaHistoryReadValueIds &nodesToRead,
				OpcUa_UInt32 transactionId));

		// Reads the processed history of item values asynchronous from the OPC server based on a passed aggregate
		MOCK_METHOD(UaStatus, beginHistoryReadProcessed, (
				ServiceSettings & serviceSettings,
				const HistoryReadProcessedContext &historyReadProcessedContext,
				const UaHistoryReadValueIds &nodesToRead,
				OpcUa_UInt32 transactionId));

		// Reads the history of item values asynchronous from OPC server for the given timestamps
		MOCK_METHOD(UaStatus, beginHistoryReadAtTime, (
				ServiceSettings & serviceSettings,
				const HistoryReadAtTimeContext &historyReadAtTimeContext,
				const UaHistoryReadValueIds &nodesToRead,
				OpcUa_UInt32 transactionId));

		// Reads the history of events from an event notifier object asynchronous from OPC server
		MOCK_METHOD(UaStatus, beginHistoryReadEvent, (
				ServiceSettings & serviceSettings,
				const HistoryReadEventContext &historyReadEventContext,
				const UaHistoryReadValueIds &nodesToRead,
				OpcUa_UInt32 transactionId));

		// Update the history of item values asynchronous at OPC server
		MOCK_METHOD(UaStatus, beginHistoryUpdateData, (
				ServiceSettings & serviceSettings,
				const UpdateDataDetails &updateDataDetails,
				OpcUa_UInt32 transactionId));

		// Delete the history of item values asynchronous at OPC server
		MOCK_METHOD(UaStatus, beginHistoryDeleteRawModified, (
				ServiceSettings & serviceSettings,
				const DeleteRawModifiedDetails &deleteDetails,
				OpcUa_UInt32 transactionId));

		// Delete the history of item values asynchronous at OPC server for the given timestamps
		MOCK_METHOD(UaStatus, beginHistoryDeleteAtTime, (
				ServiceSettings & serviceSettings,
				const DeleteAtTimeDetails &deleteDetails,
				OpcUa_UInt32 transactionId));

		// Update the history of events asynchronous at OPC server
		MOCK_METHOD(UaStatus, beginHistoryUpdateEvents, (
				ServiceSettings & serviceSettings,
				const UpdateEventDetails &updateEventDetails,
				OpcUa_UInt32 transactionId));

		// Delete the history of events asynchronous at OPC server for the given event Ids
		MOCK_METHOD(UaStatus, beginHistoryDeleteEvents, (
				ServiceSettings & serviceSettings,
				const DeleteEventDetails &deleteDetails,
				OpcUa_UInt32 transactionId));

		// Adds a list of nodes to the OPC server address space
		MOCK_METHOD(UaStatus, addNodes, (
				ServiceSettings & serviceSettings,
				const UaAddNodesItems &nodesToAdd,
				UaAddNodesResults &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Adds a list of references to the OPC server address space
		MOCK_METHOD(UaStatus, addReferences, (
				ServiceSettings & serviceSettings,
				const UaAddReferencesItems &referencesToAdd,
				UaStatusCodeArray &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Removes a list of nodes from the OPC server address space
		MOCK_METHOD(UaStatus, deleteNodes, (
				ServiceSettings & serviceSettings,
				const UaDeleteNodesItems &nodesToDelete,
				UaStatusCodeArray &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Removes a list of references from the OPC server address space
		MOCK_METHOD(UaStatus, deleteReferences, (
				ServiceSettings & serviceSettings,
				const UaDeleteReferencesItems &referencesToDelete,
				UaStatusCodeArray &results,
				UaDiagnosticInfos & diagnosticInfos));

		// Adds a list of nodes asynchronous to the OPC server address space
		MOCK_METHOD(UaStatus, beginAddNodes, (
				ServiceSettings & serviceSettings,
				const UaAddNodesItems &nodesToAdd,
				OpcUa_UInt32 transactionId));

		// Adds a list of references asynchronous to the OPC server address space
		MOCK_METHOD(UaStatus, beginAddReferences, (
				ServiceSettings & serviceSettings,
				const UaAddReferencesItems &referencesToAdd,
				OpcUa_UInt32 transactionId));

		// Removes a list of nodes asynchronous from the OPC server address space
		MOCK_METHOD(UaStatus, beginDeleteNodes, (
				ServiceSettings & serviceSettings,
				const UaDeleteNodesItems &nodesToDelete,
				OpcUa_UInt32 transactionId));

		// Removes a list of references asynchronous from the OPC server address space
		MOCK_METHOD(UaStatus, beginDeleteReferences, (
				ServiceSettings & serviceSettings,
				const UaDeleteReferencesItems &referencesToDelete,
				OpcUa_UInt32 transactionId));

		// Query OPC server address space
		MOCK_METHOD(UaStatus, queryFirst, (
				ServiceSettings & serviceSettings,
				const OpcUa_ViewDescription &view,
				const UaNodeTypeDescriptions &nodeTypes,
				const UaContentFilter &filter,
				OpcUa_UInt32 maxDataSetsToReturn,
				OpcUa_UInt32 maxReferencesToReturn,
				UaByteString &continuationPoint,
				UaQueryDataSets &queryDataSets,
				UaDiagnosticInfos &diagnosticInfos,
				UaParsingResults & parsingResults));

		// Continue a previous query request
		MOCK_METHOD(UaStatus, queryNext, (
				ServiceSettings & serviceSettings,
						OpcUa_Boolean releaseContinuationPoint,
				UaByteString & continuationPoint,
				UaQueryDataSets & queryDataSets));

		// Cancel outstanding service requests in the OPC UA server
		MOCK_METHOD(UaStatus, cancel, (
				ServiceSettings & serviceSettings,
						OpcUa_UInt32 requestHandle,
				OpcUa_UInt32 & cancelCount));

		// Indicates if the UaSession is connected. This flag does not reflect the actual server status.
		MOCK_METHOD(OpcUa_Boolean, isConnected, (), (const));

		// Provides the actual connection status to the UA server
		MOCK_METHOD(UaClient::ServerStatus, serverStatus, (), (const));

		// Provides the actual state of the UA server
		MOCK_METHOD(OpcUa_ServerState, serverState, (), (const));

		// Provides the currently used URL for the connection to the UA server
		MOCK_METHOD(UaString, currentlyUsedEndpointUrl, (), (const));

		// Provides the Id assigned from the server to this session. This Id uniquely identifies the session on the server.
		MOCK_METHOD(UaNodeId, sessionId, (), (const));

		// Provides the certificate of the UA server
		MOCK_METHOD(UaByteString, serverCertificate, (), (const));

		// Get a description for all arguments of a given Method
		MOCK_METHOD(UaStatus, getMethodArguments, (
				ServiceSettings & serviceSettings,
				const UaNodeId &methodId,
				UaArguments &inputArguments,
				UaArguments & outputArguments));

		MOCK_METHOD(OpcUa_UInt32, connectTimeout, (), (const));
		MOCK_METHOD(void, setConnectTimeout, (OpcUa_UInt32 connectTimeout), ());

		MOCK_METHOD(OpcUa_UInt32, maxOperationsPerServiceCall, (), (const));
		MOCK_METHOD(void, setMaxOperationsPerServiceCall, (OpcUa_UInt32 maxOperationsPerServiceCall), ());
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerBrowse, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerHistoryReadData, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerHistoryReadEvents, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerHistoryUpdateData, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerHistoryUpdateEvents, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerMethodCall, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerNodeManagement, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerRead, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerRegisterNodes, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerTranslateBrowsePathsToNodeIds, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxNodesPerWrite, (), (const));

		MOCK_METHOD(OpcUa_UInt32, watchdogTime, (), (const));
		MOCK_METHOD(void, setWatchdogTime, (OpcUa_UInt32 watchdogTime), ());
		MOCK_METHOD(OpcUa_UInt32, watchdogTimeout, (), (const));
		MOCK_METHOD(void, setWatchdogTimeout, (OpcUa_UInt32 watchdogTimeout), ());

		MOCK_METHOD(OpcUa_UInt32, revisedSecureChannelLifetime, (), (const));
		MOCK_METHOD(OpcUa_Double, revisedSessionTimeout, (), (const));

		MOCK_METHOD(UaStringArray, getNamespaceTable, (), (const));
		MOCK_METHOD(void, updateNamespaceTable, (), ());

		MOCK_METHOD(UaEndpointDescriptions, getServerEndpointDescriptions, (), ());
		MOCK_METHOD(SessionSecurityInfo, getSessionSecurityInfo, (), ());
		MOCK_METHOD(UaString, getServerProductUri, (), ());
		MOCK_METHOD(UaString, getServerApplicationUri, (), ());

		// UaDataTypeDictionary interface
		MOCK_METHOD(UaDataTypeDictionary::DefinitionType, definitionType, (const UaNodeId &dataTypeId), (override));
		//MOCK_METHOD(UaEnumDefinition, enumDefinition, (const UaNodeId &dataTypeId), (override));
		MOCK_METHOD(UaStructureDefinition, structureDefinition, (const UaNodeId &dataTypeId), (override));
		//MOCK_METHOD(UaOptionSetDefinition, optionSetDefinition, (const UaNodeId &dataTypeId), (override));

		MOCK_METHOD(void, loadDataTypeDictionaries, (), ());
		MOCK_METHOD(void, clearDataTypeDictionaries, (), ());

		MOCK_METHOD(OpcUa_UInt32, maxArrayLength, (), (const));
		MOCK_METHOD(OpcUa_UInt16, maxBrowseContinuationPoints, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxByteStringLength, (), (const));
		MOCK_METHOD(OpcUa_UInt16, maxHistoryContinuationPoints, (), (const));
		MOCK_METHOD(OpcUa_UInt16, maxQueryContinuationPoints, (), (const));
		MOCK_METHOD(OpcUa_UInt32, maxStringLength, (), (const));
		MOCK_METHOD(OpcUa_Double, minSupportedSampleRate, (), (const));


	protected:
		// CertificateValidationCallback interface
		MOCK_METHOD(bool, verificationError, (SessionSecurityInfo * pInfo,
				OpcUa_Void * pvVerifyContext,
				const UaByteString &certificateChain,
				OpcUa_StatusCode uVerificationResult,
				OpcUa_UInt32 uDepth), (override));
	};
}

namespace Umati {
	namespace OpcUa {
		class MockOpcUaWrapper : public OpcUaInterface {
		public:
			MockOpcUaWrapper() = default;

			virtual ~MockOpcUaWrapper() = default;

			MOCK_METHOD(UaStatus, DiscoveryGetEndpoints, (
					UaClientSdk::ServiceSettings & serviceSettings,
					const UaString &sDiscoveryURL,
							UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
							UaEndpointDescriptions & endpointDescriptions
			), (override));
			MOCK_METHOD(UaStatus, DiscoveryFindServers, (
					UaClientSdk::ServiceSettings & serviceSettings,
					const UaString &sDiscoveryURL,
							UaClientSdk::ClientSecurityInfo &clientSecurityInfo,
							UaApplicationDescriptions & applicationDescriptions
			), (override));
			MOCK_METHOD(void, GetNewSession, (std::shared_ptr<UaClientSdk::UaSession> & m_pSession), (override));
			MOCK_METHOD(UaStatus, SessionConnect, (const UaString&      sURL,
					UaClientSdk::SessionConnectInfo&  sessionConnectInfo,
					UaClientSdk::SessionSecurityInfo& sessionSecurityInfo,
					UaClientSdk::UaSessionCallback*   pSessionCallback), (override));
			MOCK_METHOD(void, SessionUpdateNamespaceTable, (), (override));

			MOCK_METHOD(UaStatus, SessionDisconnect, (UaClientSdk::ServiceSettings & serviceSettings,
					OpcUa_Boolean bDeleteSubscriptions), (override));
			MOCK_METHOD(UaStringArray, SessionGetNamespaceTable, (), (override));
			MOCK_METHOD(UaStatus, SessionRead, (UaClientSdk::ServiceSettings & serviceSettings,
					OpcUa_Double maxAge,
					OpcUa_TimestampsToReturn timeStamps,
					const UaReadValueIds &nodesToRead,
					UaDataValues &values,
					UaDiagnosticInfos & diagnosticInfos), (override));
			MOCK_METHOD(bool, SessionIsConnected, (), (override));
			MOCK_METHOD(UaStatus, SessionBrowse, (
					UaClientSdk::ServiceSettings & serviceSettings,
					const UaNodeId &nodeToBrowse,
					const UaClientSdk::BrowseContext &browseContext,
					UaByteString &continuationPoint,
					UaReferenceDescriptions & referenceDescriptions), (override));
			MOCK_METHOD(UaStatus, SessionTranslateBrowsePathsToNodeIds, (
					UaClientSdk::ServiceSettings & serviceSettings,
					const UaBrowsePaths &browsePaths,
							UaBrowsePathResults&browsePathResults,
							UaDiagnosticInfos & diagnosticInfos
			), (override));

			MOCK_METHOD(void, setSubscription, (Subscription * p_subscr), (override));
			MOCK_METHOD(void, SubscriptionCreateSubscription, (std::shared_ptr<UaClientSdk::UaSession> & m_pSession),
						(override));
			MOCK_METHOD((std::shared_ptr<Dashboard::IDashboardDataClient::ValueSubscriptionHandle>),
						SubscriptionSubscribe, (ModelOpcUa::NodeId_t nodeId,
					Dashboard::IDashboardDataClient::newValueCallbackFunction_t callback), (override));
		};

		class MockOpcUaSubscriptionWrapper : public OpcUaSubscriptionInterface {
		public:
			MOCK_METHOD(UaStatus, SessionCreateSubscription, (
					std::shared_ptr<UaClientSdk::UaSession> & rSession,
							UaClientSdk::ServiceSettings & serviceSettings,
							UaClientSdk::UaSubscriptionCallback * pUaSubscriptionCallback,
							OpcUa_UInt32 clientSubscriptionHandle,
					UaClientSdk::SubscriptionSettings & subscriptionSettings,
					OpcUa_Boolean publishingEnabled,
					UaClientSdk::UaSubscription * *ppUaSubscription),
						(override));
			MOCK_METHOD(UaStatus, SessionDeleteSubscription, (
					std::shared_ptr<UaClientSdk::UaSession> & rSession,
							UaClientSdk::ServiceSettings & serviceSettings,
							UaClientSdk::UaSubscription * *ppUaSubscription,
							OpcUa_UInt32 waitTimeForTransactionCompletion),
						(override));
		};
	}
}
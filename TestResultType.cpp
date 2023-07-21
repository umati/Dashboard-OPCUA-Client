#include <iostream>
#include "OpcUaClient/OpcUaClient.hpp"
#include <thread>
#include <chrono>
#include <open62541/client.h>
#include <ConfigureLogger.hpp>
#include <Converter/UaDataValueToJsonValue.hpp>
#include <iomanip>

using namespace std::chrono_literals;

constexpr const char* SERVER_URI = "opc.tcp://141.58.102.46:40451";

int test(int argc, char* argv[]) {
  Umati::Util::ConfigureLogger("DashboardOpcUaClient");
  Umati::OpcUa::OpcUaClient cl(SERVER_URI, []() { std::cout << "Reset of client issued." << std::endl; });
  for (std::size_t i = 0; i < 10; ++i) {
    if (cl.isConnected()) {
      break;
    }
    std::this_thread::sleep_for(1s);
  }
  if (cl.isConnected()) {
    cl.readTypeDictionaries();
    cl.buildCustomDataTypes();
    cl.updateCustomTypes();
    //ResultDataType
    UA_NodeId resultDataTypeNodeId = UA_NODEID_NUMERIC(4,3004);
    auto dataType = UA_Client_findDataType(cl.m_pClient.get(), &resultDataTypeNodeId);
    auto v = UA_new(dataType);
    memset(v, 0, dataType->memSize);
    UA_clear(v, dataType);
    UA_delete(v, dataType);
    UA_Variant value;
    UA_Variant_init(&value);
    UA_NodeId resultsId = UA_NODEID_STRING_ALLOC(1, "/ObjectsFolder/TighteningSystem/ResultManagement/Results/Result");
    auto retCode = UA_Client_readValueAttribute(cl.m_pClient.get(), resultsId, &value);
    if (retCode != UA_STATUSCODE_GOOD) {
      std::cout << "Status not Good" << std::endl;
    } else {
      std::cout << "Status is Good" << std::endl;
      UA_DataValue dv;
      UA_DataValue_init(&dv);
      dv.hasValue = UA_TRUE;
      UA_Variant_setScalarCopy(&dv.value, v, dataType);
      auto valueJson = Umati::OpcUa::Converter::UaDataValueToJsonValue(dv, cl.m_pClient.get(), UA_NODEID_NULL, false).getValue();
      std::cout << std::setw(4) << "Value as JSON:" << valueJson << std::endl;
      UA_DataValue_clear(&dv);
    }
    UA_Variant_clear(&value);
    UA_NodeId_clear(&resultsId);
  }
  cl.disconnect();
  return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
  std::cout << "Test ResultType Begin" << std::endl;
  auto ret = test(argc, argv);
  std::cout << "Test ResultType End" << std::endl;
  return ret;
}

cmake_minimum_required(VERSION 3.9)

#find_package(PahoMqttCpp REQUIRED)

set(MQTTPUBLISHER_PAHO_SRC
	"MqttPublisher_Paho.cpp"
)

add_library(MqttPublisher_Paho ${MQTTPUBLISHER_PAHO_SRC})

target_link_libraries(MqttPublisher_Paho PUBLIC DashboardClient)
target_link_libraries(MqttPublisher_Paho PUBLIC PahoMqttCpp::paho-mqttpp3-static)

target_include_directories(MqttPublisher_Paho PUBLIC 
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>
)

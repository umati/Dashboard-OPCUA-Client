cmake_minimum_required(VERSION 3.9)

find_package(MosquittoMqtt REQUIRED)

set(MQTTPUBLISHER_SRC
	"MqttPublisher.cpp"
)

add_library(MqttPublisher ${MQTTPUBLISHER_SRC})

target_link_libraries(MqttPublisher PUBLIC DashboardClient)
target_link_libraries(MqttPublisher PUBLIC MosquittoMqtt::MosquittoMqtt)

target_include_directories(MqttPublisher PUBLIC 
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>
)


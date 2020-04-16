cmake_minimum_required(VERSION 3.9)

message("### opcua_dashboardclient/RedisPublisher: loading RedisPublisher")

find_package(cpp_redis REQUIRED)

set(REDISPUBLISHER_SRC
	"RedisPublisher.cpp"
)
message("### opcua_dashboardclient/RedisPublisher: collecting source file list for library: ${REDISPUBLISHER_SRC}")

add_library(RedisPublisher ${REDISPUBLISHER_SRC})

target_link_libraries(RedisPublisher PUBLIC DashboardClient)
target_link_libraries(RedisPublisher PUBLIC cpp_redis::cpp_redis)

target_include_directories(RedisPublisher PUBLIC 
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>
)


cmake_minimum_required(VERSION 3.9)

find_package(cpp_redis REQUIRED)

set(REDISPUBLISHER_SRC
	"RedisPublisher.cpp"
)

add_library(RedisPublisher ${REDISPUBLISHER_SRC})

target_link_libraries(RedisPublisher PUBLIC DashboardClient)
target_link_libraries(RedisPublisher PUBLIC cpp_redis::cpp_redis)

target_include_directories(RedisPublisher PUBLIC 
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
$<INSTALL_INTERFACE:include>
)


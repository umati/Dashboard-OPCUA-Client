#include <gtest/gtest.h>
#include <ConfigurationJsonFile.hpp>

TEST(ConfigurationJsonFile, NormalConfiguration)
{
	Umati::Util::ConfigurationJsonFile conf("Configuration.json");
	EXPECT_EQ(conf.OpcUaEndpoint(), "opc.tcp://localhost:4840");
	EXPECT_EQ(conf.InstanceNamespaceURI(), "http://www.umati.info/example");
	EXPECT_EQ(conf.Mqtt().Hostname, "localhost");
	EXPECT_EQ(conf.Mqtt().Port, 1883);
	EXPECT_EQ(conf.Mqtt().Username, "MyUser");
	EXPECT_EQ(conf.Mqtt().Password, "MyPassword");
	EXPECT_EQ(conf.Mqtt().TopicPrefix, "/MyUser/MyMachine");

}

#include <gtest/gtest.h>
#include <Configuration.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>

TEST(ConfigurationJsonFile, NormalConfiguration) {
	Umati::Util::ConfigurationJsonFile conf("Configuration.json");
	EXPECT_EQ(conf.OpcUa().Endpoint, "opc.tcp://localhost:4840");
	EXPECT_EQ(conf.OpcUa().Username, "User");
	EXPECT_EQ(conf.OpcUa().Password, "Password");
	EXPECT_EQ(conf.OpcUa().Security, 1);
	std::vector<std::string> objectTypeNamespaces;
	objectTypeNamespaces.emplace_back("http://www.umati.info");
	EXPECT_EQ(conf.ObjectTypeNamespacesVector(), objectTypeNamespaces);
	EXPECT_EQ(conf.Mqtt().Hostname, "localhost");
	EXPECT_EQ(conf.Mqtt().Port, 1883);
	EXPECT_EQ(conf.Mqtt().Username, "MyUser");
	EXPECT_EQ(conf.Mqtt().Password, "MyPassword");

}

TEST(ConfigurationJsonFile, WithoutNamespaces) {
	Umati::Util::ConfigurationJsonFile conf("Configuration2.json");
	EXPECT_EQ(conf.OpcUa().Endpoint, "opc.tcp://localhost:4840");
	EXPECT_EQ(conf.OpcUa().Username, "User");
	EXPECT_EQ(conf.OpcUa().Password, "Password");
	EXPECT_EQ(conf.OpcUa().Security, 1);
	std::vector<std::string> objectTypeNamespaces;
	EXPECT_EQ(conf.ObjectTypeNamespacesVector(), objectTypeNamespaces);
	EXPECT_EQ(conf.Mqtt().Hostname, "localhost");
	EXPECT_EQ(conf.Mqtt().Port, 1883);
	EXPECT_EQ(conf.Mqtt().Username, "MyUser");
	EXPECT_EQ(conf.Mqtt().Password, "MyPassword");

}

TEST(ConfigurationJsonFile, FileNotFound) {
	EXPECT_THROW(
			Umati::Util::ConfigurationJsonFile conf("ConfigurationNotThere.json"),
			Umati::Util::Exception::ConfigurationException
	);
}

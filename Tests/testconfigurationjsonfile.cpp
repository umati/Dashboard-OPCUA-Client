#include <gtest/gtest.h>
#include <Configuration.hpp>
#include <ConfigurationJsonFile.hpp>
#include <Exceptions/ConfigurationException.hpp>

TEST(ConfigurationJsonFile, NormalConfiguration) {
	Umati::Util::ConfigurationJsonFile conf("Configuration.json");
	EXPECT_EQ(conf.getOpcUa().Endpoint, "opc.tcp://localhost:4840");
	EXPECT_EQ(conf.getOpcUa().Username, "User");
	EXPECT_EQ(conf.getOpcUa().Password, "Password");
	EXPECT_EQ(conf.getOpcUa().Security, 1);
	std::vector<std::string> objectTypeNamespaces;
	objectTypeNamespaces.emplace_back("http://www.umati.info");
	EXPECT_EQ(conf.getObjectTypeNamespaces(), objectTypeNamespaces);
	EXPECT_EQ(conf.getMqtt().Hostname, "localhost");
	EXPECT_EQ(conf.getMqtt().Port, 1883);
	EXPECT_EQ(conf.getMqtt().Username, "MyUser");
	EXPECT_EQ(conf.getMqtt().Password, "MyPassword");

}

TEST(ConfigurationJsonFile, WithoutNamespaces) {
	Umati::Util::ConfigurationJsonFile conf("Configuration2.json");
	EXPECT_EQ(conf.getOpcUa().Endpoint, "opc.tcp://localhost:4840");
	EXPECT_EQ(conf.getOpcUa().Username, "User");
	EXPECT_EQ(conf.getOpcUa().Password, "Password");
	EXPECT_EQ(conf.getOpcUa().Security, 1);
	std::vector<std::string> objectTypeNamespaces;
	EXPECT_EQ(conf.getObjectTypeNamespaces(), objectTypeNamespaces);
	EXPECT_EQ(conf.getMqtt().Hostname, "localhost");
	EXPECT_EQ(conf.getMqtt().Port, 1883);
	EXPECT_EQ(conf.getMqtt().Username, "MyUser");
	EXPECT_EQ(conf.getMqtt().Password, "MyPassword");

}

TEST(ConfigurationJsonFile, FileNotFound) {
	EXPECT_THROW(
			Umati::Util::ConfigurationJsonFile conf("ConfigurationNotThere.json"),
			Umati::Util::Exception::ConfigurationException
	);
}

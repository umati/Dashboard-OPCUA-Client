#include <MachineCacheJsonFile.hpp>
#include <gtest/gtest.h>

TEST(MachineCache, ReadFile)
{
	Umati::MachineObserver::MachineCacheJsonFile machCache("MachineCacheTest.json", false);

	EXPECT_EQ(machCache.GetEntry("NotAvailable"), nullptr);

	auto pEntry = machCache.GetEntry("http://umati.info");
	ASSERT_NE(pEntry, nullptr);

}
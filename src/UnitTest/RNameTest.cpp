// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include <fmt/format.h>

#include <RName.h>

using namespace dcclite;

TEST(RName, BasicTest)
{
	RName name1{ "abc" };
	RName name2{ "abc" };

	//We use this to make sure we are not referencing the original strings
	RName nameInvalid{ std::string{"hello"} };
	RName nameInvalid1{ std::string{"hello"} };

	ASSERT_EQ(nameInvalid, nameInvalid1);

	RName nullName;

	ASSERT_FALSE(nullName);
	ASSERT_TRUE(name1);
	ASSERT_TRUE(name2);

	ASSERT_FALSE(name1 < name2);
	ASSERT_FALSE(name2 < name1);

	ASSERT_TRUE(name1 < nameInvalid);
	ASSERT_FALSE(nameInvalid < name1);

	//Test those cases, becaue using ASSERT_EQ and ASSERT_NE forces using difent operators and flows...
	ASSERT_FALSE(name1 == nameInvalid);
	ASSERT_FALSE(name1 != name2);

	ASSERT_EQ(name1, name2);

	ASSERT_STREQ(name1.GetData().data(), "abc");

	RName name3{ "xyz" };
	ASSERT_NE(name1, name3);
	ASSERT_NE(name1, nullName);
	

	ASSERT_STREQ(name3.GetData().data(), "xyz");

	ASSERT_STREQ(nullName.GetData().data(), "null_name");
}

TEST(RName, FullCluster)
{
	auto numClusters = dcclite::detail::RName_GetNumClusters();

	RName cluster0{ "FullCluster0" };

	for (size_t i = 0;; ++i)
	{
		RName name{ fmt::format("unittest{}", i) };

		if (numClusters != dcclite::detail::RName_GetNumClusters())
			break;
	}

	RName cluster1{ "FullCluster1" };

	ASSERT_NE(cluster0, cluster1);

	ASSERT_STREQ(cluster0.GetData().data(), "FullCluster0");
	ASSERT_STREQ(cluster1.GetData().data(), "FullCluster1");

	auto info = dcclite::detail::RName_GetClusterInfo(numClusters - 1);
	ASSERT_GT(info.m_uRoomLeft, 0);	

	RName a = RName::TryGetName("a");
	ASSERT_FALSE(a);

	a = RName::Create("a");

	//make sure we used room left on cluster 0, special case on name allocation
	ASSERT_NE(a.FindCluster(), numClusters - 1);
}

TEST(RName, LimitTest)
{
	auto numClusters = dcclite::detail::RName_GetNumClusters();	

	RName cluster0{ "LimitTestcluster0" };

	for (size_t i = 0;; ++i)
	{
		RName name{ fmt::format("unittest{:800}", i) };

		if (numClusters != dcclite::detail::RName_GetNumClusters())
		{
			//force special test on RName
			RName name{ fmt::format("unittest{:800}", i+1) };

			break;
		}
	}

	RName cluster1{ "LimitTestcluster1" };

	ASSERT_NE(cluster0, cluster1);

	ASSERT_STREQ(cluster0.GetData().data(), "LimitTestcluster0");
	ASSERT_STREQ(cluster1.GetData().data(), "LimitTestcluster1");

	auto info = dcclite::detail::RName_GetClusterInfo(numClusters - 1);
	ASSERT_GT(info.m_uRoomLeft, 0);	

	RName a = RName::TryGetName("#");
	ASSERT_FALSE(a);

	a = RName::Create("a");

	for (size_t i = 0;; ++i)
	{
		RName name{ fmt::format("{}", i) };

		if (name.FindCluster() != a.FindCluster())
			break;
	}

	//make sure we used room left on cluster 0, special case on name allocation
	ASSERT_EQ(a.FindCluster(), numClusters - 1);

	RName big("big big big big big big big big big big");
	ASSERT_NE(big.FindCluster(), numClusters - 1);

	//TEST operator bool when cluster != 1 and index == 0
	//TEST new cluster creations
	//TEST using a cluster with no string room and creating a new one

}
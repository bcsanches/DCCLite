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

#include "GuidDefs.h"
#include <dcclite/Guid.h>

using namespace dcclite;

TEST(Guid, ToStringAndBack)
{
	Guid base;
	TryGuidLoadFromString(base, "988F1C89-DD13-42AA-B883-0A16513064EF");

	Guid orig;
	orig.m_bId[0] = 0x98;
	orig.m_bId[1] = 0x8F;
	orig.m_bId[2] = 0x1C;
	orig.m_bId[3] = 0x89;
	orig.m_bId[4] = 0xDD;
	orig.m_bId[5] = 0x13;
	orig.m_bId[6] = 0x42;
	orig.m_bId[7] = 0xAA;
	orig.m_bId[8] = 0xB8;
	orig.m_bId[9] = 0x83;
	orig.m_bId[10] = 0x0A;
	orig.m_bId[11] = 0x16;
	orig.m_bId[12] = 0x51;
	orig.m_bId[13] = 0x30;
	orig.m_bId[14] = 0x64;
	orig.m_bId[15] = 0xEF;

	ASSERT_EQ(base, orig);

	// {6213C7BA-6A61-417E-923C-41ADB73A0825}
	//static const GUID << name >> =
	//{ 0x6213c7ba, 0x6a61, 0x417e, { 0x92, 0x3c, 0x41, 0xad, 0xb7, 0x3a, 0x8, 0x25 } };


	auto baseStr = GuidToString(base);

	Guid read;
	ASSERT_TRUE(TryGuidLoadFromString(read, baseStr));
	ASSERT_EQ(read, base);
}

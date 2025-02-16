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

#include <dcclite_shared/Packet.h>

using namespace dcclite;

TEST(Packet, Base)
{				
	BasePacket<32> packet;
	
	//packet is empty
	ASSERT_EQ(packet.GetSize(), 0);

	packet.Write8('a');
	ASSERT_EQ(packet.GetSize(), 1);

	packet.Write32(0xAABBCCDD);
	ASSERT_EQ(packet.GetSize(), 5);

	packet.Write16(0xEEFF);
	ASSERT_EQ(packet.GetSize(), 7);

	packet.Write8('b');
	ASSERT_EQ(packet.GetSize(), 8);

	packet.Write64(0xAABBCCDDBEEFDEAF);
	ASSERT_EQ(packet.GetSize(), 16);

	packet.Write32(36);
	ASSERT_EQ(packet.GetSize(), 20);

#if 0
	union
	{
		uint32_t i;
		char data[4];
	} u;

	u.i = 0xAABBCCDD;
#endif

	/**
		'  J  o  h  n  '
	  hex  4A 6F 68 6E
	  ----------------
	  -> 0x4A6F686E	
	*/

	packet.Write32(0x6E686F4A); // John 
	ASSERT_EQ(packet.GetSize(), 24);

	packet.Reset();
	ASSERT_EQ(packet.GetSize(), 0);

	ASSERT_EQ(packet.Read<uint8_t>(), 'a');
	ASSERT_EQ(packet.GetSize(), 1);

	ASSERT_EQ(packet.Read<uint32_t>(), 0xAABBCCDD);
	ASSERT_EQ(packet.GetSize(), 5);

	ASSERT_EQ(packet.Read<uint16_t>(), 0xEEFF);
	ASSERT_EQ(packet.GetSize(), 7);

	ASSERT_EQ(packet.Read<uint8_t>(), 'b');
	ASSERT_EQ(packet.GetSize(), 8);

	ASSERT_EQ(packet.Read<uint64_t>(), 0xAABBCCDDBEEFDEAF);
	ASSERT_EQ(packet.GetSize(), 16);

	ASSERT_EQ(packet.Read<uint32_t>(), 36);
	ASSERT_EQ(packet.GetSize(), 20);	

	char name[5];
	
	name[0] = packet.ReadByte();
	name[1] = packet.ReadByte();
	name[2] = packet.ReadByte();
	name[3] = packet.ReadByte();
	name[4] = '\0';

	ASSERT_EQ(strcmp(name, "John"), 0);
}
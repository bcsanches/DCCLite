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

#include <dcclite_shared/BitPack.h>

using namespace dcclite;


template <size_t N>
void Compare(const BitPack<N> &pack, bool proof[N])
{
	for (unsigned i = 0; i < pack.size(); ++i)
	{
		ASSERT_EQ(pack[i], proof[i]);
	}
}

		
TEST(BitPack, Basic)
{
	bool proof[32] = { false };
	BitPack<32> pack;

	ASSERT_EQ(pack.size(), unsigned{ 32 });

	pack.SetBit(0);
	proof[0] = true;

	ASSERT_TRUE(pack[0]);

	pack.SetBit(1);
	ASSERT_TRUE(pack[1]);
	proof[1] = true;

	for (unsigned i = 2; i < pack.size(); ++i)
	{
		ASSERT_FALSE(pack[i]);
	}

	pack.SetBit(8);
	proof[8] = true;

	pack.SetBit(10);
	proof[10] = true;

	pack.SetBit(15);
	proof[15] = true;

	pack.SetBit(31);
	proof[31] = true;

	Compare<32>(pack, proof);				
}

TEST(BitPack, Alternate)
{
	bool proof[32] = { false };
	BitPack<32> pack;

	for (unsigned i = 0; i < pack.size(); i += 2)
	{
		proof[i] = true;
		pack.SetBit(i);					
	}

	Compare<32>(pack, proof);

	bool value = false;
	for (unsigned i = 0; i < pack.size(); ++i)
	{
		proof[i] = value;
		pack.SetBitValue(i, value);

		value = !value;
	}

	ASSERT_FALSE(proof[0]);
	ASSERT_TRUE(proof[1]);

	Compare<32>(pack, proof);
}

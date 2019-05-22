// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "CppUnitTest.h"

#include "BitPack.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	using namespace dcclite;

	TEST_CLASS(BitPackUnitTest)
	{
		private:
			template <size_t N>
			void Compare(const BitPack<N> &pack, bool proof[N])
			{
				for (unsigned i = 0; i < pack.size(); ++i)
				{
					Assert::AreEqual(pack[i], proof[i]);
				}
			}

		public:
			TEST_METHOD(Basic)
			{
				bool proof[32] = { false };
				BitPack<32> pack;

				Assert::AreEqual(pack.size(), unsigned{ 32 });

				pack.SetBit(0);
				proof[0] = true;

				Assert::IsTrue(pack[0]);

				pack.SetBit(1);
				Assert::IsTrue(pack[1]);
				proof[1] = true;

				for (unsigned i = 2; i < pack.size(); ++i)
				{
					Assert::IsFalse(pack[i]);
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

			TEST_METHOD(Alternate)
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

				Assert::IsFalse(proof[0]);
				Assert::IsTrue(proof[1]);

				Compare<32>(pack, proof);
			}
	};
}
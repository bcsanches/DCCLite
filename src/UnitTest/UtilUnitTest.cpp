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

#include "Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTest
{
	using namespace dcclite;

	TEST_CLASS(dccliteStringTrim)
	{
		public:

			TEST_METHOD(Basic)
			{				
				Assert::IsTrue(StrTrim("test").compare("test") == 0);
				Assert::IsTrue(StrTrim("   test").compare("test") == 0);
				Assert::IsTrue(StrTrim("   test    ").compare("test") == 0);
				Assert::IsTrue(StrTrim("test   ").compare("test") == 0);
				Assert::IsTrue(StrTrim("   ").compare("") == 0);
				Assert::IsTrue(StrTrim("").compare("") == 0);
			}
	};
}
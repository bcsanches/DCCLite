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

#include "Object.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTestx
{
	using namespace dcclite;

	TEST_CLASS(ObjectPathUnitTest)
	{
		public:			

			TEST_METHOD(Empty)
			{
				ObjectPath path;
				
				Assert::IsTrue(path.begin() == path.end());
			}

			TEST_METHOD(SingleName)
			{
				ObjectPath path("test");

				auto it = path.begin();
				Assert::AreEqual(it.ToString(), std::string("test"));

				++it;
				Assert::IsTrue(it == path.end());
			}

			TEST_METHOD(Root)
			{
				ObjectPath root("/");

				auto it = root.begin();
				Assert::AreEqual(it.ToString(), std::string(1, '/'));

				++it;
				Assert::IsTrue(it == root.end());
			}

			TEST_METHOD(RootWithSubFolder)
			{
				ObjectPath root("/abc/def");

				auto it = root.begin();
				Assert::AreEqual(it.ToString(), std::string(1, '/'));

				++it;
				Assert::AreEqual(it.ToString(), std::string("abc"));

				++it;
				Assert::AreEqual(it.ToString(), std::string("def"));

				++it;

				Assert::IsTrue(it == root.end());

			}

			TEST_METHOD(RelativeWithSubFolder)
			{
				ObjectPath root("abc/def");

				auto it = root.begin();
				Assert::AreEqual(it.ToString(), std::string("abc"));

				++it;
				Assert::AreEqual(it.ToString(), std::string("def"));

				++it;

				Assert::IsTrue(it == root.end());
			}
	};
}
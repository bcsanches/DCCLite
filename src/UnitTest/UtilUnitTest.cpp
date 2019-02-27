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
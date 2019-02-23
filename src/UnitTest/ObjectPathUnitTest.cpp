#include "CppUnitTest.h"

#include "Object.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTest
{
	TEST_CLASS(ObjectPathUnitTest)
	{
	public:

		TEST_METHOD(Constructor)
		{
			using namespace dcclite;

			Object obj("test");
		}

	};
}

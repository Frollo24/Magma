#include "mgmpch.h"
#include "Assert.h"

namespace Magma
{
	void Assertions::Verify(AssertionCaller caller, bool condition, const char* file, i32 line)
	{
		if (!condition)
		{
			switch (caller)
			{
			case AssertionCaller::CoreAssertion:
				MGM_CORE_ERROR("Verification Failed at File: {0}; Line: {1}", file, line);
				break;
			case AssertionCaller::ClientAssertion:
				MGM_ERROR("Verification Failed at File: {0}; Line: {1}", file, line);
				break;
			}
			MGM_DEBUGBREAK();
		}
	}
}
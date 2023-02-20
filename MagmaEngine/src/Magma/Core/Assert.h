#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Core/Log.h"

#ifdef MGM_DEBUG
	#define MGM_ENABLE_ASSERTS
	#define MGM_DEBUGBREAK() __debugbreak()
#else
	#define MGM_DEBUGBREAK()
#endif

#ifdef MGM_ENABLE_ASSERTS
	#define MGM_VERIFY(x)           ::Magma::Assertions::Verify(Magma::AssertionCaller::ClientAssertion, (x), __FILE__, __LINE__)
	#define MGM_CORE_VERIFY(x)      ::Magma::Assertions::Verify(Magma::AssertionCaller::CoreAssertion, (x), __FILE__, __LINE__)
	#define MGM_ASSERT(x, ...)      ::Magma::Assertions::Assert(Magma::AssertionCaller::ClientAssertion, (x), __VA_ARGS__)
	#define MGM_CORE_ASSERT(x, ...) ::Magma::Assertions::Assert(Magma::AssertionCaller::CoreAssertion, (x), __VA_ARGS__)
#else
	#define MGM_VERIFY(x)
	#define MGM_CORE_VERIFY(x)
	#define MGM_ASSERT(x, ...)
	#define MGM_CORE_ASSERT(x, ...)
#endif

namespace Magma
{
	enum class AssertionCaller
	{
		CoreAssertion,
		ClientAssertion
	};

	class MAGMA_API Assertions
	{
	public:
		static void Verify(AssertionCaller caller, bool condition, const char* file, i32 line);

		template<typename... Args>
		static void Assert(AssertionCaller caller, bool condition, Args&&... args)
		{
			if (!condition)
			{
				switch (caller)
				{
				case AssertionCaller::CoreAssertion:
					MGM_CORE_ERROR("Assertion Failed: {0}", std::forward<Args>(args)...);
					break;
				case AssertionCaller::ClientAssertion:
					MGM_ERROR("Assertion Failed: {0}", std::forward<Args>(args)...);
					break;
				}
				MGM_DEBUGBREAK();
			}
		}
	};
}
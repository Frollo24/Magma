#pragma once

#ifdef MGM_PLATFORM_WINDOWS
	#ifdef MGM_BUILD_DLL
		#define MAGMA_API __declspec(dllexport)
	#else
		#define MAGMA_API __declspec(dllimport)
	#endif // MGM_BUILD_DLL
#else
	#error "Magma Engine only supports Windows at the moment!"
#endif // MGM_PLATFORM_WINDOWS
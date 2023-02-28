#pragma once

#include "Magma/Core/Build.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Magma
{
	class MAGMA_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// --- CORE LOG MACROS ---
#define MGM_CORE_TRACE(...)   ::Magma::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define MGM_CORE_INFO(...)    ::Magma::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MGM_CORE_WARN(...)    ::Magma::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MGM_CORE_ERROR(...)   ::Magma::Log::GetCoreLogger()->error(__VA_ARGS__)
#define MGM_CORE_FATAL(...)   ::Magma::Log::GetCoreLogger()->critical(__VA_ARGS__)

// --- CLIENT LOG MACROS ---
#define MGM_TRACE(...)        ::Magma::Log::GetClientLogger()->trace(__VA_ARGS__)
#define MGM_INFO(...)         ::Magma::Log::GetClientLogger()->info(__VA_ARGS__)
#define MGM_WARN(...)         ::Magma::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MGM_ERROR(...)        ::Magma::Log::GetClientLogger()->error(__VA_ARGS__)
#define MGM_FATAL(...)        ::Magma::Log::GetClientLogger()->critical(__VA_ARGS__)
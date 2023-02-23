#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
	struct Timestep
	{
	public:
		Timestep(float timeInSeconds = 0.0f) : m_Time(timeInSeconds) {}

		inline operator float() { return m_Time; }
		inline operator const float& () const { return m_Time; }

		inline float AsSeconds() { return m_Time; }
		inline float AsMilliseconds() { return m_Time * 1000.0f; }

	private:
		float m_Time;
	};

	class MAGMA_API Time final
	{
	public:
		static const Timestep& TotalTime;
		static const Timestep& DeltaTime;

	private:
		friend class Application;

		static void SetTotalTime(const Timestep& timestep);
		static void SetDeltaTime(const Timestep& timestep);
	};
}


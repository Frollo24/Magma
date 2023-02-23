#include "mgmpch.h"
#include "Time.h"

namespace Magma
{
	static Timestep s_TotalTime = Timestep();
	static Timestep s_DeltaTime = Timestep();

	const Timestep& Time::TotalTime = s_TotalTime;
	const Timestep& Time::DeltaTime = s_DeltaTime;

	void Time::SetTotalTime(const Timestep& timestep) { s_TotalTime = timestep; }
	void Time::SetDeltaTime(const Timestep& timestep) { s_DeltaTime = timestep; }
}
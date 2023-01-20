#include "pch.h"

bool Util::isNotExhausted(int32_t& timeVar, uint32_t miliseconds)
{
	uint32_t timeNow = timeGetTime();
	if (timeNow - timeVar >= miliseconds)
	{
		timeVar = timeNow;
		return true;
	}
	return false;
}

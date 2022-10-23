// Copyright Seven47 Software All Rights Reserved.


#include "PerformanceTool.h"

double UPerformanceTool::GetCurrentTimeSeconds()
{
	return FPlatformTime::Seconds();
}

double UPerformanceTool::MakeElapsedTimeMs(double StartSeconds, double EndSeconds)
{
	return (EndSeconds - StartSeconds)*1000;
}

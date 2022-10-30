/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "apps/emon/EnergyMonitor.h"
#include "apps/config/EnergyMonitorConfig.h"

using namespace apps;

// the setup function runs once when you press reset or power the board
void setup()
{
	KSF_FRAMEWORK_INIT()
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	KSF_RUN_APP_BLOCKING_LOOPED(emon::EnergyMonitor)
	KSF_RUN_APP_BLOCKING_LOOPED(config::EnergyMonitorConfig)
}
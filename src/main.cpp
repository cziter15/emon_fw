/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "apps/emon/EnergyMonitorApp.h"
#include "apps/config/EnergyMonitorConfig.h"
#include "ksf/ksAppRotator.h"

using namespace apps;

KSF_IMPLEMENT_APP_ROTATOR
(
	emon::EnergyMonitorApp, 
	config::EnergyMonitorConfig
)
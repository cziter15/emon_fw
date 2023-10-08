/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "board.h"

#include "EnergyMonitorConfig.h"
#include "EnergyMonitorConfigProvider.h"

namespace apps::config
{
	const char EnergyMonitorConfig::emonDeviceName[] = "EnergyMonitor";

	bool EnergyMonitorConfig::init()
	{
		addComponent<ksf::comps::ksLed>(STATUS_LED_PIN);
		addComponent<ksf::comps::ksLed>(EVENT_LED_PIN);

		addComponent<ksf::comps::ksWiFiConfigurator>(emonDeviceName);
		addComponent<ksf::comps::ksMqttConfigProvider>();
		addComponent<EnergyMonitorConfigProvider>();

		return true;
	}

	bool EnergyMonitorConfig::loop()
	{
		return ksApplication::loop();
	}
}

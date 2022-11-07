/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#pragma once

#include <ksIotFrameworkLib.h>
#include "ArduinoOTA.h"

#define EMON_SENSOR_PROBES 400
#define EMON_TIMER_INTERVAL 50
#define MAX_ANA_VALUE 1024
#define EMON_SENSOR_TIMEOUT 600000UL
#define EMON_STABILIZATION_PROBE_COUNT 10
#define EMON_DEVIATION_UPHILL 1.75f
#define EMON_DEVIATION_STABILIZATION 1.1f

namespace apps::emon
{
	class EnergyMonitorApp : public ksf::ksApplication
	{
		protected:
			std::weak_ptr<ksf::comps::ksLed> statusLedWp, eventLedWp;
			std::weak_ptr<ksf::comps::ksMqttConnector> mqttWp;

			std::shared_ptr<ksf::evt::ksEventHandle> connEventHandleSp, disEventHandleSp;

			ArduinoOTAClass ArduinoOTA;

			void onMqttConnected();
			void onMqttDisconnected();
		public:
			bool init() override;
			bool loop() override;
	};
}
/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */
#include "board.h"
#include <ksIotFrameworkLib.h>

#include "EnergyMonitorApp.h"
#include "../config/EnergyMonitorConfig.h"
#include "components/EnergySensor.h"

using namespace std::placeholders;
using namespace apps::config;

namespace apps::emon
{
	bool EnergyMonitorApp::init()
	{
		addComponent<ksf::comps::ksWifiConnector>(EnergyMonitorConfig::emonDeviceName);
		addComponent<ksf::comps::ksMqttDebugResponder>();

		mqttWp = addComponent<ksf::comps::ksMqttConnector>();
		statusLedWp = addComponent<ksf::comps::ksLed>(STATUS_LED_PIN);
		eventLedWp = addComponent<ksf::comps::ksLed>(EVENT_LED_PIN);

		auto sensorCompWp = addComponent<components::EnergySensor>(ANA_PIN);

		/* Setup reset button */
		addComponent<ksf::comps::ksResetButton>(CFG_PUSH_PIN, LOW);

		if (!ksApplication::init())
			return false;
				
		ArduinoOTA.setHostname(EnergyMonitorConfig::emonDeviceName);
		ArduinoOTA.setPassword("ota_ksiotframework");
		ArduinoOTA.begin();
		
		if (auto mqttSp{mqttWp.lock()})
		{
			mqttSp->onConnected->registerEvent(connEventHandleSp, std::bind(&EnergyMonitorApp::onMqttConnected, this));
			mqttSp->onDisconnected->registerEvent(disEventHandleSp, std::bind(&EnergyMonitorApp::onMqttDisconnected, this));
		}

		if (auto statusLedSp{statusLedWp.lock()})
			statusLedSp->setBlinking(500);

		if (auto sensorCompSp{sensorCompWp.lock()})
			sensorCompSp->setEventLed(eventLedWp);

		return true;
	}

	void EnergyMonitorApp::onMqttDisconnected()
	{
		if (auto statusLedSp{statusLedWp.lock()})
			statusLedSp->setBlinking(500);
	}

	void EnergyMonitorApp::onMqttConnected()
	{
		if (auto statusLedSp{statusLedWp.lock()})
			statusLedSp->setBlinking(0);
	}

	bool EnergyMonitorApp::loop()
	{
		ArduinoOTA.handle();
		return ksApplication::loop();
	}
}

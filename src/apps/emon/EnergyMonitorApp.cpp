/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "board.h"
#include <ksIotFrameworkLib.h>

#include "EnergyMonitorApp.h"
#include "components/EnergySensor.h"
#include "../config/EnergyMonitorConfig.h"

using namespace std::placeholders;

namespace apps::emon
{
	bool EnergyMonitorApp::init()
	{
		/* Add WiFi connector component. */
		addComponent<ksf::comps::ksWifiConnector>(apps::config::EnergyMonitorConfig::emonDeviceName);

		/* Add MQTT components. */
		addComponent<ksf::comps::ksMqttDebugResponder>();
		mqttWp = addComponent<ksf::comps::ksMqttConnector>();

		/* Add LED indicator components. */
		statusLedWp = addComponent<ksf::comps::ksLed>(STATUS_LED_PIN);
		eventLedWp = addComponent<ksf::comps::ksLed>(EVENT_LED_PIN);

		/* Create OTA component. */
		addComponent<ksf::comps::ksOtaUpdater>();

		/* Add sensor component. */
		auto sensorCompWp{addComponent<components::EnergySensor>(ANA_PIN)};

		/* Setup reset button. */
		addComponent<ksf::comps::ksResetButton>(CFG_PUSH_PIN, LOW);

		if (!ksApplication::init())
			return false;

		/* Bind MQTT connect/disconnect events for LED status. */
		if (auto mqttSp{mqttWp.lock()})
		{
			mqttSp->onConnected->registerEvent(connEventHandleSp, std::bind(&EnergyMonitorApp::onMqttConnected, this));
			mqttSp->onDisconnected->registerEvent(disEventHandleSp, std::bind(&EnergyMonitorApp::onMqttDisconnected, this));
		}

		/* Set event LED. */
		if (auto sensorCompSp{sensorCompWp.lock()})
			sensorCompSp->setEventLed(eventLedWp);

		/* Start blinking status LED. */
		if (auto statusLedSp{statusLedWp.lock()})
			statusLedSp->setBlinking(500);

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
		return ksApplication::loop();
	}
}

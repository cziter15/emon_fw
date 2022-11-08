/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/raesp_gateway/blob/main/firmware/LICENSE
 */

#include "EnergySensor.h"
#include "../../config/EnergyMonitorConfigProvider.h"

using namespace std::placeholders;
using apps::config::EnergyMonitorConfigProvider;

namespace apps::emon::components
{
	EnergySensor::EnergySensor(uint8_t pin)
		: lineSensor(EMON_SENSOR_PROBES, EMON_MAX_ANA_VALUE, pin)
	{}

	void EnergySensor::setEventLED(std::weak_ptr<ksf::comps::ksLed>& eventLedWp)
	{
		this->eventLedWp = eventLedWp;
	}

	bool EnergySensor::init(ksf::ksComposable* owner)
	{
		EnergyMonitorConfigProvider configProvider;
		configProvider.init(owner);

		if (!configProvider.setupRotations(rotationsPerKwh))
			return false;

		mqttWp = owner->findComponent<ksf::comps::ksMqttConnector>();
		
		if (auto mqttSp{mqttWp.lock()})
		{
			mqttSp->onMesssage->registerEvent(msgEventHandleSp, std::bind(&EnergySensor::onMqttMessage, this, _1, _2));
			mqttSp->onConnected->registerEvent(connEventHandleSp, std::bind(&EnergySensor::onMqttConnected, this));
		}
		
		return true;
	}

	void EnergySensor::onMqttConnected()
	{
		if (auto mqttSp{mqttWp.lock()})
		{
			mqttSp->subscribe("totalkWh");
			mqttSp->subscribe("clearkwh");
		}
	}

	void EnergySensor::onMqttMessage(const std::string_view& topic, const std::string_view& payload)
	{
		if (auto mqttSp{mqttWp.lock()})
		{
			if (topic.compare("totalkWh") == 0 && initialKwh < 0)
			{
				initialKwh = std::atof(std::string(payload).c_str());
				mqttSp->unsubscribe("totalkWh");
			}
			else if (topic.compare("clearkwh") == 0)
			{
				totalPulseCount = 0;
				initialKwh = 0;

				mqttSp->publish("totalkWh", "0", true);
			}
		}
	}

	void EnergySensor::onBlackLineDetected()
	{
		auto pulseTime{(millis() - lastPulseTime)};
		updateCurrentUsage(EMON_MS_PER_HOUR / (rotationsPerKwh * pulseTime) * 1000.0);

		++totalPulseCount;

		if (auto eventLedSp{eventLedWp.lock()})
			eventLedSp->setBlinking(75, 4);
	}

	void EnergySensor::updateCurrentUsage(uint16_t currentWatts)
	{
		lastPulseTime = millis();

		if (auto mqttSp{mqttWp.lock()})
			mqttSp->publish("watts", ksf::to_string(currentWatts));
	}

	void EnergySensor::onUpdateTotalUsage()
	{
		if (auto mqttSp{mqttWp.lock()})
		{
			double totalkWh{(totalPulseCount / static_cast<double>(rotationsPerKwh)) + initialKwh};
			mqttSp->publish("totalkWh", ksf::to_string(totalkWh, 2), true);
		}
	}

	bool EnergySensor::loop()
	{
		if (sensorTimer.triggered() && lineSensor.triggered())
		{
			onBlackLineDetected();
			zeroWattsTimer.restart();
		}

		if (zeroWattsTimer.triggered())
			updateCurrentUsage(0);

		if (totalUpdateTimer.triggered())
			onUpdateTotalUsage();

		return true;
	}
}
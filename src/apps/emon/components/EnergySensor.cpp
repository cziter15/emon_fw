/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "EnergySensor.h"
#include "../../config/EnergyMonitorConfigProvider.h"

using namespace std::placeholders;

namespace apps::emon::components
{
	EnergySensor::EnergySensor(uint8_t pin)
		: pin(pin)
	{}

	void EnergySensor::setEventLed(std::weak_ptr<ksf::comps::ksLed>& eventLedWp)
	{
		this->eventLedWp = eventLedWp;
	}

	bool EnergySensor::init(ksf::ksApplication* app)
	{
		apps::config::EnergyMonitorConfigProvider configProvider;
		configProvider.init(app);
		return configProvider.setupRotations(rotationsPerKwh);
	}

	bool EnergySensor::postInit(ksf::ksApplication* app)
	{
		mqttWp = app->findComponent<ksf::comps::ksMqttConnector>();
		
		if (auto mqttSp{mqttWp.lock()})
		{
			mqttSp->onDeviceMessage->registerEvent(msgEventHandleSp, std::bind(&EnergySensor::onMqttDevMessage, this, _1, _2));
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

	void EnergySensor::onMqttDevMessage(const std::string_view& topic, const std::string_view& payload)
	{
		if (auto mqttSp{mqttWp.lock()})
		{
			if (topic == "totalkWh" && !initialKwh.has_value())
			{
				initialKwh = std::atof(std::string(payload).c_str());
				mqttSp->unsubscribe("totalkWh");
			}
			else if (topic == "clearkwh")
			{
				totalPulseCount = 0;
				initialKwh = 0;
				
				mqttSp->publish("totalkWh", "0", true);
			}
		}
	}

	double EnergySensor::getKwhFromTotal() const
	{
		return totalPulseCount / static_cast<double>(rotationsPerKwh);
	}

	void EnergySensor::onBlackLineDetected()
	{
		/* We need to have valid initial value, so we can start counting watts starting from second uphill. */
		if (!lastPulseTime.has_value())
		{
			lastPulseTime = millis();
			return;
		}

		/* Calculate current Watts 'usage'. */
		auto pulseTime{(millis() - *lastPulseTime)};
		updateCurrentUsage(MS_PER_HOUR / (rotationsPerKwh * pulseTime) * 1000.0);

		/*
			Increment total pulse count for kWh calculation. 
			Handle overflow (optimistic anyway, the device will probably never reach such uptime).
		*/
		if (totalPulseCount == std::numeric_limits<uint32_t>::max())
		{
			initialKwh = initialKwh.value_or(0) + getKwhFromTotal();
			totalPulseCount = 0;
		}
		else
		{
			++totalPulseCount;
		}

		/* Blink LED. */
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
			double totalkWh{getKwhFromTotal() + initialKwh.value_or(0)};
			mqttSp->publish("totalkWh", ksf::to_string(totalkWh, 2), true);
		}
	}

	bool EnergySensor::loop(ksf::ksApplication* app)
	{
		if (!fast50msTimer.triggered())
			return true;

		/* Handle spin detection, restart zero-sending timer. */
		auto adcValue{static_cast<uint16_t>((analogRead(pin) >> 2))};

		if (adcValue > maxAdcValueTemp)
			maxAdcValueTemp = adcValue;

		if (adcMaxUpdateTimer.triggered())
		{
			highAdcTreshold = maxAdcValueTemp * 0.85f;
			lowAdcTreshold = maxAdcValueTemp * 0.5f;
			maxAdcValueTemp = 0;
		}

#if DEBUG_ADC_TRESHOLDS
		app->log([&] (std::string& obuf) {
			obuf += "ADC read: ";
			obuf += ksf::to_string(adcValue);
			obuf += " lowAdcTreshold: ";
			obuf += ksf::to_string(lowAdcTreshold);
			obuf += " highAdcTreshold: ";
			obuf += ksf::to_string(highAdcTreshold);
		});
#endif
		/* Handle black line detection. */
		if (highAdcTreshold != 0.0f)
		{
			bool highReading{adcValue >= highAdcTreshold};
			bool lowReading{adcValue <= lowAdcTreshold};

			if (numTrendReadings <= 0)
			{
				if (lowReading && --numTrendReadings == -10)
					numTrendReadings = 3;
			} 
			else if (numTrendReadings > 0 && highReading && --numTrendReadings == 0)
			{
				onBlackLineDetected();
				zeroWattsTimer.restart();
			}
		}

		/* Force send 0 watts if no rotation occured at specified interval. */
		if (zeroWattsTimer.triggered())
			updateCurrentUsage(0);

		/* Update total kWh at specified interval. */
		if (totalUpdateTimer.triggered())
			onUpdateTotalUsage();

		return true;
	}
}
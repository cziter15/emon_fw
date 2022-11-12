/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#pragma once

#include <ksIotFrameworkLib.h>
#include "utils/LineSensor.h"

namespace apps::emon::components
{
	class EnergySensor : public ksf::ksComponent
	{
		KSF_RTTI_DECLARATIONS(EnergySensor, ksComponent)

		protected:
			static constexpr uint16_t ADC_HISTORY_PROBES{400};					// How probes required to calculate dominant.
			static constexpr uint16_t ADC_READ_INTERVAL{50};					// Fast timer interval to measure ADC value.
			static constexpr uint16_t MAX_ADC_VALUE{1024};						// This is in general MAX ADC value that can be read.
			
			static constexpr uint32_t MS_SENSOR_TOTALUPDATE{300000UL};			// Interval of publishing total kWh usage.
			static constexpr uint32_t MS_ZERO_WATTS_TIMEOUT{300000UL};			// Timeout value after which '0' watts will be published.
			static constexpr double MS_PER_HOUR{3600000.0};						// Milliseconds in hour.

			utils::LineSensor plateRotationSensor;								// Sensor utility, handles analog part.

			unsigned short rotationsPerKwh{1};									// Rotation number per kWh.
			double initialKwh{-1.0};											// Initial kWh count (read from MQTT).

			uint32_t lastPulseTime{0};											// Last pulse time (millis).
			uint32_t totalPulseCount{0};										// Total black line detection counter.

			std::weak_ptr<ksf::comps::ksMqttConnector> mqttWp;					// Weak pointer to MQTT connector.
			std::weak_ptr<ksf::comps::ksLed> eventLedWp;						// Weak pointer to event LED.

			std::shared_ptr<ksf::evt::ksEventHandle> connEventHandleSp;			// MQTT onConnection event handler.
			std::shared_ptr<ksf::evt::ksEventHandle> msgEventHandleSp;			// MQTT onMessage event handler.

			ksf::ksSimpleTimer sensorTimer{MS_ADC_READ_INTERVAL};				// Timer for sensor ticking.
			ksf::ksSimpleTimer totalUpdateTimer{MS_SENSOR_TOTALUPDATE};			// Timer for updaing total kWh.
			ksf::ksSimpleTimer zeroWattsTimer{MS_ZERO_WATTS_TIMEOUT};			// Timer to send '0' value when no rotation detected.

			/*
				Called when black line is detected.
			*/
			void onBlackLineDetected();

			/*
				Called when total usage timer expires (typically every few minutes).
			*/
			void onUpdateTotalUsage();

			/*
				Updates (and sends) current Watts 'usage'.

				@param currentWatts Number of watts.
			*/
			void updateCurrentUsage(uint16_t currentWatts);

			/*
				Called on MQTT connection established.
			*/
			void onMqttConnected();

			/*
				Called on MQTT message arrival.
			*/
			void onMqttMessage(const std::string_view& topic, const std::string_view& payload);

		public:
			/*
				Constructs EnergySensor.

				@param pin Analog pin number for the sensor.
			*/
			EnergySensor(uint8_t pin);

			/*
				Passes weak pointer to the event signalling LED.

				@param eventLedWp Weak pointer to ksLed component.
			*/
			void setEventLED(std::weak_ptr<ksf::comps::ksLed>& eventLedWp);
			
			/*
				Initializes EnergySensor component.

				@param owner Pointer to ownning ksComposable object (application).
				@return True on success, false on fail.
			*/
			bool init(ksf::ksComposable* owner) override;

			/*
				Called from application loop. Handles EnergySensor logic.
				@return True on success, false on fail.
			*/
			bool loop() override;
	};
}
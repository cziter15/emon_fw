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
#include <optional>
#include "utils/PlateSpinSensor.h"

namespace apps::emon::components
{
	class EnergySensor : public ksf::ksComponent
	{
		KSF_RTTI_DECLARATIONS(EnergySensor, ksComponent)

		protected:
			static constexpr uint32_t MS_KWH_UPDATE_INTERVAL{300000UL};			// Interval of publishing total kWh usage.
			static constexpr uint32_t MS_ZERO_WATTS_TIMEOUT{300000UL};			// Timeout value after which '0' watts will be published.
			static constexpr double MS_PER_HOUR{3600000.0};						// Milliseconds in hour.

			utils::PlateSpinSensor plateSpinSensor;								// Sensor utility, handles analog part.

			unsigned short rotationsPerKwh{1};									// Rotation number per kWh.
			std::optional<double> initialKwh;									// Initial kWh count (read from MQTT).

			std::optional<uint32_t> lastPulseTime;								// Last pulse time (millis).
			uint32_t totalPulseCount{0};										// Total black line detection counter.

			std::weak_ptr<ksf::comps::ksMqttConnector> mqttWp;					// Weak pointer to MQTT connector.
			std::weak_ptr<ksf::comps::ksLed> eventLedWp;						// Weak pointer to event LED.

			std::shared_ptr<ksf::evt::ksEventHandle> connEventHandleSp;			// MQTT onConnection event handler.
			std::shared_ptr<ksf::evt::ksEventHandle> msgEventHandleSp;			// MQTT onMessage event handler.

			ksf::ksSimpleTimer totalUpdateTimer{MS_KWH_UPDATE_INTERVAL};		// Timer for updaing total kWh.
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
				Event handler method called when MQTT service receives a message.

				@param topic Reference of topic string_view.
				@param message Reference of message string_view.
			*/
			void onMqttMessage(const std::string_view& topic, const std::string_view& payload);

			/*
				Calculates kwh usage by dividing pulse count per rotationsPerKwh.

				@return Calculated kwh usage.
			*/
			double getKwhFromTotal() const;

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
			void setEventLed(std::weak_ptr<ksf::comps::ksLed>& eventLedWp);
			
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
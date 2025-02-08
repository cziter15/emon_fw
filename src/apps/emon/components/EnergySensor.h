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

namespace apps::emon::components
{
	class EnergySensor : public ksf::ksComponent
	{
		KSF_RTTI_DECLARATIONS(EnergySensor, ksComponent)

		protected:
			static constexpr uint32_t MS_KWH_UPDATE_INTERVAL{300000UL};			// Interval of publishing total kWh usage.
			static constexpr uint32_t MS_ZERO_WATTS_TIMEOUT{300000UL};			// Timeout value after which '0' watts will be published.
			static constexpr uint32_t ADC_MAX_UPDATE_TIMEWINDOW{120000UL};		// Time window for ADC reading update.

			static constexpr double MS_PER_HOUR{3600000.0};						// Milliseconds in hour.

			uint8_t pin{0};														// Pin number.
			unsigned short rotationsPerKwh{1};									// Rotation number per kWh.
			std::optional<double> initialKwh;									// Initial kWh count (read from MQTT).

			std::optional<uint32_t> lastPulseTime;								// Last pulse time (millis).
			uint16_t maxAdcValueTemp{0}; 										// Maximum ADC value.
			int16_t numTrendReadings{0};										// Number of readings for trend calculation.
			float highAdcTreshold{0.0f}, lowAdcTreshold{0.0f};					// ADC thresholds.

			uint32_t totalPulseCount{0};										// Total black line detection counter.

			std::weak_ptr<ksf::comps::ksMqttConnector> mqttWp;					// Weak pointer to MQTT connector.
			std::weak_ptr<ksf::comps::ksLed> eventLedWp;						// Weak pointer to event LED.

			std::unique_ptr<ksf::evt::ksEventHandle> connEventHandleSp;			// MQTT onConnection event handler.
			std::unique_ptr<ksf::evt::ksEventHandle> msgEventHandleSp;			// MQTT onMessage event handler.

			ksf::misc::ksSimpleTimer fast50msTimer{50};								// Timer for fast 50ms events.
			ksf::misc::ksSimpleTimer totalUpdateTimer{MS_KWH_UPDATE_INTERVAL};		// Timer for updaing total kWh.
			ksf::misc::ksSimpleTimer adcMaxUpdateTimer{ADC_MAX_UPDATE_TIMEWINDOW};	// Timer for updating ADC max value.
			ksf::misc::ksSimpleTimer zeroWattsTimer{MS_ZERO_WATTS_TIMEOUT};			// Timer to send '0' value when no rotation detected.

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
			void onMqttDevMessage(const std::string_view& topic, const std::string_view& payload);

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

				@param app Pointer to ownning application
				@return True on success, false on fail.
			*/
			bool init(ksf::ksApplication* app) override;

			/*
				Post-initializes EnergySensor component.

				@param app Pointer to ownning application
				@return True on success, false on fail
			*/
			bool postInit(ksf::ksApplication* app) override; 

			/*
				Called from application loop. Handles EnergySensor logic.
				@param app Pointer to ownning application
				@return True on success, false on fail.
			*/
			bool loop(ksf::ksApplication* app) override;
	};
}
/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/raesp_gateway/blob/main/firmware/LICENSE
 */

#pragma once

#include <ksIotFrameworkLib.h>
#include "utils/LineSensor.h"

#define EMON_SENSOR_PROBES 400
#define EMON_TIMER_INTERVAL 50
#define EMON_MAX_ANA_VALUE 1024
#define EMON_SENSOR_TOTALUPDATE 300000UL
#define EMON_MS_PER_HOUR 3600000.0

namespace apps::emon::components
{
	class EnergySensor : public ksf::ksComponent
	{
		KSF_RTTI_DECLARATIONS(EnergySensor, ksComponent)

		protected:
			utils::LineSensor lineSensor;

			unsigned short rotationsPerKwh{1};
			double initialKwh{-1.0};

			uint32_t lastPulseTime{0};
			uint32_t totalPulseCount{0};

			std::weak_ptr<ksf::comps::ksMqttConnector> mqttWp;
			std::weak_ptr<ksf::comps::ksLed> eventLedWp;

			std::shared_ptr<ksf::evt::ksEventHandle> connEventHandleSp, msgEventHandleSp;

			ksf::ksSimpleTimer sensorTimer{EMON_TIMER_INTERVAL};
			ksf::ksSimpleTimer totalUpdateTimer{EMON_SENSOR_TOTALUPDATE};

			void onBlackLineDetected();
			void onUpdateTotalUsage();

			void updateCurrentUsage(uint16_t currentWatts);

			void onMqttConnected();
			void onMqttMessage(const std::string_view& topic, const std::string_view& payload);

		public:
			EnergySensor(uint8_t pin);
			void setEventLed(std::weak_ptr<ksf::comps::ksLed>& eventLedWp);
			bool init(ksf::ksComposable* owner) override;
			bool loop() override;
	};
}